/*
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 1.4.1
 * @author Jake Mason
 * @date 09-09-2022
 *
 * abbrv is licensed under the Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License
 *
 * See LICENSE.txt for more information
 **/

/*
 * TODO: MacOSX implementation threads / ideas
 * https://stackoverflow.com/questions/10711095/how-do-i-keep-a-key-pressed-using-cgeventpost-in-c-on-osx
 */


#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <stdio.h>

#include <string>

#include "AppData.hpp"

#if WIN32
#include <windows.h>
#endif

struct SDL_Window;
struct SDL_Renderer;
typedef void* SDL_GLContext;
class Input;

class Platform
{
public:
  Platform();
  ~Platform();

  void init(const char* title, int width, int height);
  void init();
  void io(float deltaTime, Input* input);
  void frameStart(Input* input);
  void frameEnd();
  void handleOSEvents(Input* input);
  void initRenderer();
  void cleanUp();

  // varies from platform to platform
  static int isShiftActive();
  static int isCapsLockActive();
  static void registerKeyboardHook();
  static void onKeyPress(char pressed);
  static void simulateKeyboardInput(int abbreviationLength, std::string toSend);

  std::string version = "1.4.1";

  inline static SDL_Window* window;
  SDL_GLContext context;
  SDL_Renderer* renderer;

  static AppData* data;


  bool isRunning;
#if WIN32
  static HHOOK keyboardHook;
  static UINT WM_TASKBARCREATED;
  static void addTrayIcon(SDL_Window* window);
  static void removeTrayIcon(SDL_Window* window);
#endif
};

#endif

