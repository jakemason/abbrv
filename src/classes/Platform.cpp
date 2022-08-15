/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-13-2022
 *
 **/

#define GL3_PROTOTYPES 1
#include "Platform.hpp"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <fstream>

#include "Debug.hpp"
#include "Input.hpp"

Platform::Platform() {}
Platform::~Platform() {}

int Platform::isShiftActive() { return GetKeyState(VK_LSHIFT) < 0 || GetKeyState(VK_RSHIFT) < 0; }

int Platform::isCapsLockActive() { return (GetKeyState(VK_CAPITAL) & 1) == 1; }

void Platform::onKeyPress(char pressed)
{
  const int MODIFIER_PRESSED = -52; // both alt and ctrl return -52 and we don't want to consider
                                    // these "breaks" in the matching chain so don't report them
  if ((int)pressed == MODIFIER_PRESSED) return;
  DEBUG("input received. char: %c, value of %d", pressed, (int)pressed);
}

#if WINDOWS_BUILD
// The function that implements the key logging functionality
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{

  if (wParam == WM_KEYDOWN)
  {
    KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
    DWORD wVirtKey             = kbdStruct->vkCode;
    DWORD wScanCode            = kbdStruct->scanCode;

    BYTE lpKeyState[256];
    GetKeyboardState(lpKeyState);
    lpKeyState[VK_SHIFT]   = 0;
    lpKeyState[VK_CAPITAL] = 0;
    if (Platform::isShiftActive()) { lpKeyState[VK_SHIFT] = 0x80; }
    if (Platform::isCapsLockActive()) { lpKeyState[VK_CAPITAL] = 0x01; }

    char result;
    ToAscii(wVirtKey, wScanCode, lpKeyState, (LPWORD)&result, 0);
    Platform::onKeyPress(result);
  }

  return CallNextHookEx(NULL, nCode, wParam, lParam);
}
#endif

void Platform::registerKeyboardHook()
{
#if WINDOWS_BUILD
  // Retrieve the applications instance
  HINSTANCE instance = GetModuleHandle(NULL);
  // Set a global Windows Hook to capture keystrokes using the function declared above
  HHOOK keylistener = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, instance, 0);
#endif
}

void Platform::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
{
  std::string fullTitle = std::string(title);
  fullTitle             = fullTitle + " " + version;
  // NOTE: This sets DPI awareness for modern laptops and such. Windows
  // advises _against_ doing this in code and, instead, suggests using
  // a manifest.xml file to specify it. The manifest.xml setup destroys my
  // entire build so... code it is.
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    ERR("SDL failed to initialize! SDL_Error: %s", SDL_GetError());
    isRunning = false;
    return;
  }

  if (TTF_Init() == -1)
  {
    ERR("SDL TTF failed to initialize. Error: %s", TTF_GetError());
    isRunning = false;
    return;
  }

#if EDITOR_MODE
  window = SDL_CreateWindow(title, xpos, ypos, width, height,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
#else
  window = SDL_CreateWindow(title, xpos, ypos, width, height,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALLOW_HIGHDPI);
#endif
  // NOTE
  // This is NOT good enough by itself. An ".ico" file is required as well
  // and baked directly into the program itself through CMake. See
  // "platform/CMakeLists.txt" for more information, as well as the
  // associated "athena.rc" file.
  SDL_Surface* icon = IMG_Load("../assets/app_icon.png");
  SDL_SetWindowIcon(window, icon);

  if (fullscreen) SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

  // this line is needed to ensure that textures are recreated when the
  // window is resized. If we remove this line and resize the window we
  // are presented with black boxes where our textures used to be.
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

#if OPENGL_RENDERER
  this->initRenderer();
  fullTitle = fullTitle + " | OpenGL ";
  fullTitle += (const char*)glGetString(GL_VERSION);
#endif

#if WINDOWS_BUILD
  fullTitle += " | Windows OS";
#endif

#if DEBUG_MODE
  fullTitle += " | Debugging Enabled";
#endif

#if EDITOR_MODE
  fullTitle += " | Editor Enabled";
#endif

  if (window) { DEBUG("Window created successfully."); }

  isRunning = true;

  SDL_SetWindowTitle(window, fullTitle.c_str());
}

void Platform::initRenderer()
{
  // Set our OpenGL version.
  // SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated
  // functions are disabled
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // 4.6 is part of the modern versions of OpenGL, but most video cards
  // can handle it so we use that as our standard
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

  // Turn on double buffering with a 24bit Z buffer.
  // You may need to change this to 16 or 32 for your system
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifdef __APPLE__
  glewExperimental = GL_TRUE;
  glewInit();
#endif
  glEnable(GL_SCISSOR_TEST);
  glScissor(0, 0, 1024, 576);
  glViewport(0, 0, 1024, 576);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, context);
  SDL_GL_SetSwapInterval(0); // vsync;
}

void Platform::handleOSEvents(Input* input)
{
  memset(input->isKeyPressed, 0, sizeof(input->isKeyPressed));
  input->mouseScroll      = 0;
  input->leftClicked      = false;
  input->rightClicked     = false;
  input->middleClicked    = false;
  input->textInputTrigger = false;
  input->windowResized    = false;
  input->windowMoved      = false;
  input->windowClosed     = false;
  input->windowID         = -1;
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_MOUSEMOTION:
      {
        input->mouseX          = event.motion.x;
        input->mouseY          = event.motion.y;
        input->rawMouseX       = event.motion.x;
        input->rawMouseY       = event.motion.y;
        input->mousePosUpdated = true;
        break;
      }
      case SDL_MOUSEWHEEL:
      {
        if (event.wheel.y > 0) { input->mouseScroll = -1; }
        if (event.wheel.y < 0) { input->mouseScroll = 1; }
        break;
      }
      case SDL_MOUSEBUTTONDOWN:
      {
        if (event.button.button == SDL_BUTTON_LEFT)
        {
          input->leftClicked   = true;
          input->leftClickHeld = true;
        }
        else if (event.button.button == SDL_BUTTON_MIDDLE)
        {
          input->middleClickHeld = true;
          input->middleClicked   = true;
        }
        else
        {
          input->rightClicked   = true;
          input->rightClickHeld = true;
        }
        break;
      }
      case SDL_MOUSEBUTTONUP:
      {
        if (event.button.button == SDL_BUTTON_LEFT) { input->leftClickHeld = false; }
        else if (event.button.button == SDL_BUTTON_MIDDLE)
        {
          input->middleClickHeld = false;
        }
        else
        {
          input->rightClickHeld = false;
        }
        break;
      }
      case SDL_QUIT:
      {
        isRunning = false;
        break;
      }
      case SDL_TEXTINPUT:
      {
        input->textInput        = event.text.text;
        input->textInputTrigger = true;
        break;
      }
      case SDL_KEYDOWN:
      {
        switch (event.key.keysym.sym)
        {
          default:
          {
            int scanCode = event.key.keysym.scancode;

            if (!input->isKeyHeld[scanCode]) { input->isKeyPressed[scanCode] = true; }

            input->isKeyHeld[scanCode] = true;

            input->shiftHeld = event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT);
            input->ctrlHeld  = event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL);

            break;
          }
        }
        break;
      }
      case SDL_KEYUP:
      {
        // switch (event.key.keysym.sym)
        {
          // default:
          {
            int scanCode                  = event.key.keysym.scancode;
            input->isKeyHeld[scanCode]    = false;
            input->isKeyPressed[scanCode] = false;

            input->shiftHeld = event.key.keysym.mod & KMOD_LSHIFT;
            input->ctrlHeld  = event.key.keysym.mod & KMOD_LCTRL;
            break;
          }
        }
      }
      case SDL_WINDOWEVENT:
      {
        input->windowID = event.window.windowID;
        case SDL_WINDOWEVENT_RESIZED:
        {
          input->windowResized = true;
          break;
        }
        case SDL_WINDOWEVENT_MOVED:
        {
          input->windowMoved = true;
          break;
        }
        case SDL_WINDOWEVENT_CLOSE:
        {
          input->windowClosed = true;
          break;
        }
        break;
      }
      default:
        break;
    }
  }
}

void Platform::clean()
{
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  DEBUG("Platform exited successfully.");
}
