/**
 * Displace Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-13-2022
 *
 **/
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <tchar.h>
#include <windows.h>

#include <ctime>
#include <string>

#include "Debug.hpp"
#include "Input.hpp"
#include "Platform.hpp"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

int main(int argc, char* args[])
{
  Input* input       = new Input();
  Platform* platform = new Platform();
  int screenWidth    = 800;
  int screenHeight   = 500;
  int fullscreen     = 0;
  platform->init("Displace", 50, 50, screenWidth, screenHeight, fullscreen);
  platform->registerKeyboardHook();
  DEBUG("Platform initialization complete.");
  int countFrequency = SDL_GetPerformanceFrequency();
  float deltaTime    = 0.0f;
  float fps          = 0.0f;
  Debug::init();

  while (platform->isRunning)
  {

    int prevCounter = SDL_GetPerformanceCounter();

    // BEGIN GAME LOOP
    platform->handleOSEvents(input);
    static bool firstInit = true;
    { // init
      if (firstInit)
      {
        DEBUG("Initializing Editor.");
        int screenWidth, screenHeight;
        SDL_GetWindowSize(platform->window, &screenWidth, &screenHeight);
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
        // static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        // ImFontConfig icons_config;
        // icons_config.MergeMode  = true;
        // icons_config.PixelSnapH = true;
        // io.Fonts->AddFontFromFileTTF("../assets/fonts/fontawesome.ttf", (int)(17.0f * dpiScalar), &icons_config,
        //                            icons_ranges);

        /*
        io.Fonts->AddFontDefault();
        */
        io.Fonts->Build();

        /* enable docking in IMGUI. This is _not_ available in the main master */
        /* branch, but rather in the "docking" branch which is, at the time of */
        /* writing, 930 commits ahead of master.*/
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

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
        if (firstInit)
        {
          // ImGui_ImplOpenGL3_Shutdown();
          ImGui_ImplSDL2_InitForOpenGL(platform->window, platform->context);
          ImGui_ImplOpenGL3_Init("#version 150");
        }
#endif
        //
        firstInit = false;
      }
    }


    { // io
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
      io.DeltaTime = 0.016f;
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


    { // frame start
      int screenWidth, screenHeight;
      SDL_GetWindowSize(platform->window, &screenWidth, &screenHeight);


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
        ImGui_ImplSDL2_NewFrame(platform->window);
#endif
      }

      ImGui::NewFrame();

      if (ImGui::BeginMainMenuBar())
      {
        if (ImGui::BeginMenu("Displace")) { ImGui::Text("Test"); }
      }

      ImGui::EndMainMenuBar();

      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight));

      ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
      ImGui::ShowDemoWindow();
      ImGui::ShowMetricsWindow();

      ImGui::Begin("Testing Window");
      ImGui::End();
    }


    { // app logic
      int screenWidth, screenHeight;
      SDL_GetWindowSize(platform->window, &screenWidth, &screenHeight);
      if (input->windowResized)
      {
        DEBUG("New screen size detected: %d X %d", screenWidth, screenHeight);
#if OPENGL_RENDERER
        // glEnable(GL_SCISSOR_TEST);
        //  the scissor prevents pixels from being rendered outside of the frame.
        // glScissor(0, 0, screenWidth, screenHeight);
#endif
      }

      glViewport(0, 0, screenWidth, screenHeight);
    }


    { // frame end

      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Round borders
      ImGui::PushStyleColor(ImGuiCol_WindowBg,
                            ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f)); // Background color
      ImGui::PopStyleVar(1);
      ImGui::PopStyleColor(1);


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
        SDL_GL_MakeCurrent(platform->window, platform->context);
      }

      // glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    }


#if OPENGL_RENDERER
    SDL_GL_SwapWindow(platform->window);
#endif
    int nowCounter     = SDL_GetPerformanceCounter();
    int counterElapsed = nowCounter - prevCounter;
    deltaTime          = (((float)counterElapsed * 1000.0f) / (float)countFrequency) / 1000.0f;
    fps                = (float)countFrequency / (float)counterElapsed;
    prevCounter        = nowCounter;
  }

  return 0;
}
