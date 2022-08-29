/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 1.2
 * @author Jake Mason
 * @date 08-25-2022
 *
 * abbrv is licensed under the Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License
 *
 * See LICENSE.txt for more information
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
  int screenHeight   = 400;
  platform->init("abbrv", screenWidth, screenHeight);
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
  }

  platform->cleanUp();
  return 0;
}
