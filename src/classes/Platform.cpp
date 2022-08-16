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
#include <vector>
#if WINDOWS_BUILD
#include <windows.h>
#endif
#include "Debug.hpp"
#include "Icons.hpp"
#include "Input.hpp"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

Platform::Platform() {}
Platform::~Platform() {}

void Platform::init()
{
  int screenWidth, screenHeight;
  SDL_GetWindowSize(window, &screenWidth, &screenHeight);
  SDL_CaptureMouse(SDL_TRUE);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsLight();
  const float systemDefaultDPI =
#ifdef __APPLE__
      72.0f;
#elif defined(_WIN32)
      96.0f;
#else
      ERR("No system default DPI set for this platform.");
#endif

  float ddpi, hdpi, vdpi;
  SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi);

  float dpiScalar = ddpi / systemDefaultDPI;

  // NOTE: Fonts must be loaded _here_ as shown below, before ImGuiSDL::Initialize is called.
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.DisplaySize                       = ImVec2(screenWidth, screenHeight);
  io.ConfigWindowsMoveFromTitleBarOnly = true;
  ImGuiStyle* style                    = &ImGui::GetStyle();
  // we want to scale our font size by the dpi of our current monitor,
  // otherwise we end up with a tiny font
  //
  // Additionally, the font we want loaded by "default" needs to be
  // loaded first.
  io.Fonts->AddFontFromFileTTF("../assets/fonts/roboto.ttf", (int)(19 * dpiScalar));

  // merge in icons from Font Awesome
  static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
  ImFontConfig icons_config;
  icons_config.MergeMode  = true;
  icons_config.PixelSnapH = true;
  io.Fonts->AddFontFromFileTTF("../assets/fonts/fontawesome.ttf", (int)(17.0f * dpiScalar), &icons_config,
                               icons_ranges);

  /*
  io.Fonts->AddFontDefault();
  */
  io.Fonts->Build();

  /* enable docking in IMGUI. This is _not_ available in the main master */
  /* branch, but rather in the "docking" branch which is, at the time of */
  /* writing, 930 commits ahead of master.*/
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
  //  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

  // set theme styles
  ImGui::StyleColorsDark();

#if OPENGL_RENDERER
  bool err = glewInit() != GLEW_OK;
  if (err) { ERR("Failed to init OpenGL loader!"); }

  // NOTE This changed when we updated to IMGUI 1.85. Previously,
  // we _had_ to call ImGui_ImplOpenGL3_Shutdown() first to keep hot
  // reloading working, but now that _seems_ unnecessary? We'll see.
  // If we try to call these more than once we now throw an assert that
  // didn't exist in 1.84!
  // ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_InitForOpenGL(window, context);
  ImGui_ImplOpenGL3_Init("#version 150");
#endif
}
void Platform::io(float deltaTime, Input* input)
{
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  if (input->textInputTrigger) { io.AddInputCharactersUTF8(input->textInput.c_str()); }

  // have to setup this to read these keys. Not sure why this has to be
  // done manually
  io.KeyMap[ImGuiKey_Tab]         = SDL_SCANCODE_TAB;
  io.KeyMap[ImGuiKey_LeftArrow]   = SDL_SCANCODE_LEFT;
  io.KeyMap[ImGuiKey_RightArrow]  = SDL_SCANCODE_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow]     = SDL_SCANCODE_UP;
  io.KeyMap[ImGuiKey_DownArrow]   = SDL_SCANCODE_DOWN;
  io.KeyMap[ImGuiKey_PageUp]      = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ImGuiKey_PageDown]    = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ImGuiKey_Home]        = SDL_SCANCODE_HOME;
  io.KeyMap[ImGuiKey_End]         = SDL_SCANCODE_END;
  io.KeyMap[ImGuiKey_Insert]      = SDL_SCANCODE_INSERT;
  io.KeyMap[ImGuiKey_Delete]      = SDL_SCANCODE_DELETE;
  io.KeyMap[ImGuiKey_Backspace]   = SDL_SCANCODE_BACKSPACE;
  io.KeyMap[ImGuiKey_Space]       = SDL_SCANCODE_SPACE;
  io.KeyMap[ImGuiKey_Enter]       = SDL_SCANCODE_RETURN;
  io.KeyMap[ImGuiKey_Escape]      = SDL_SCANCODE_ESCAPE;
  io.KeyMap[ImGuiKey_KeyPadEnter] = SDL_SCANCODE_KP_ENTER;
  io.KeyMap[ImGuiKey_A]           = SDL_SCANCODE_A;
  io.KeyMap[ImGuiKey_C]           = SDL_SCANCODE_C;
  io.KeyMap[ImGuiKey_V]           = SDL_SCANCODE_V;
  io.KeyMap[ImGuiKey_X]           = SDL_SCANCODE_X;
  io.KeyMap[ImGuiKey_Y]           = SDL_SCANCODE_Y;
  io.KeyMap[ImGuiKey_Z]           = SDL_SCANCODE_Z;

  for (int i = 0; i < 284; i++)
  {
    io.KeysDown[i] = input->isKeyHeld[i];
  }
  io.DeltaTime = deltaTime;
  io.KeyShift  = input->shiftHeld;
  io.KeyCtrl   = input->ctrlHeld;

  int mouseX, mouseY;
  const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

  /*
   * If viewports are enabled we need to set our raw mouse coordinates
   * relative to the entire Desktop, not the window.
   */
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    SDL_GetGlobalMouseState(&input->rawMouseX, &input->rawMouseY);
  }
  io.MousePos     = ImVec2(static_cast<float>(input->rawMouseX), static_cast<float>(input->rawMouseY));
  io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
  io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
  io.MouseWheel   = static_cast<float>(-input->mouseScroll);
}
void Platform::frameStart(Input* input)
{
  { // frame start
    int screenWidth, screenHeight;
    SDL_GetWindowSize(window, &screenWidth, &screenHeight);


    ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)SDL_GetWindowFromID(input->windowID));
    // NOTE: Viewport Validity
    // This can fail when the engine undergoes a hot-reload. We need to
    // check this to make sure we don't attempt to access the viewport in
    // this case. ImGui appears to eventually correct this and it continues
    // to work, but it seems there's a delay of at least a single frame
    // where this does not return a valid pointer.
    if (viewport != nullptr)
    {
      if (input->windowClosed) { viewport->PlatformRequestClose = true; }

      if (input->windowMoved) { viewport->PlatformRequestMove = true; }

      if (input->windowResized) { viewport->PlatformRequestResize = true; }
    }

    if (input->windowResized)
    {
#if OPENGL_RENDERER
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame(window);
#endif
    }

    ImGui::NewFrame();


    // ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    //  ImGui::ShowDemoWindow();
    //  ImGui::ShowMetricsWindow();
  }
}
void Platform::frameEnd()
{
#if OPENGL_RENDERER
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  // Update and Render additional Platform Windows
  // (Platform functions may change the current OpenGL context, so we save/restore it to make it
  // easier to paste this code elsewhere.
  //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context)
  //  directly)
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    SDL_GL_MakeCurrent(window, context);
  }
#endif
}

int Platform::isShiftActive() { return GetKeyState(VK_LSHIFT) < 0 || GetKeyState(VK_RSHIFT) < 0; }

int Platform::isCapsLockActive() { return (GetKeyState(VK_CAPITAL) & 1) == 1; }

void Platform::onKeyPress(char pressed)
{
  const int BACKSPACE_PRESSED = 8;   // TODO: Use this to "back up" the Tries?
  const int MODIFIER_PRESSED  = -52; // both alt and ctrl return -52 and we don't want to consider
                                     // these "breaks" in the matching chain so don't report them
  if ((int)pressed == MODIFIER_PRESSED) return;

  // TODO: Need to unhook then rehook when we are successful
  // if (pressed == 't') { simulateKeyboardInput(1, "Yes, this works!"); }

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


void Platform::simulateKeyboardInput(int backspaceCount, std::string toSend)
{
#if WINDOWS_BUILD
  HKL kbl = GetKeyboardLayout(0);

  for (int i = 0; i < backspaceCount; i++)
  {
    INPUT input  = {};
    input.type   = INPUT_KEYBOARD;
    input.ki.wVk = VK_BACK;
    SendInput(1, &input, sizeof(INPUT));
  }

  for (int i = 0; i < toSend.size(); i++)
  {
    INPUT input  = {};
    input.type   = INPUT_KEYBOARD;
    input.ki.wVk = VkKeyScanEx(toSend[i], kbl);
    SendInput(1, &input, sizeof(INPUT));
  }

#endif
}


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

  window = SDL_CreateWindow(title, xpos, ypos, width, height,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
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

  SDL_SetWindowMinimumSize(window, 500, 300);
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
