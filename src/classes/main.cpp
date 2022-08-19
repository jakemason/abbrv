/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-18-2022
 *
 **/
#include <SDL2/SDL.h>

#include "AppData.hpp"
#include "Debug.hpp"
#include "Editor.hpp"
#include "Input.hpp"
#include "Platform.hpp"

int main(int argc, char* args[])
{
  Input* input       = new Input();
  Platform* platform = new Platform();
  int screenWidth    = 800;
  int screenHeight   = 500;
  int fullscreen     = 0;
  platform->init("abbrv", 50, 50, screenWidth, screenHeight, fullscreen);
  platform->registerKeyboardHook();
  int countFrequency = SDL_GetPerformanceFrequency();
  float deltaTime    = 0.0f;
  Debug::init();
  platform->init();


  while (platform->isRunning)
  {
    int prevCounter = SDL_GetPerformanceCounter();

    platform->handleOSEvents(input);
    platform->io(deltaTime, input);
    platform->frameStart(input);

    Editor::render(platform, input, platform->data);

    platform->frameEnd();


#if OPENGL_RENDERER
    SDL_GL_SwapWindow(platform->window);
#endif
    int nowCounter     = SDL_GetPerformanceCounter();
    int counterElapsed = nowCounter - prevCounter;
    deltaTime          = (((float)counterElapsed * 1000.0f) / (float)countFrequency) / 1000.0f;
    prevCounter        = nowCounter;
  }

  platform->cleanUp();
  return 0;
}
