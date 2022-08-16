/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-13-2022
 *
 **/

#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <stdio.h>

#include <string>

#include "Data.hpp"

#if WINDOWS_BUILD
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

  void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
  void init();
  void io(float deltaTime, Input* input);
  void frameStart(Input* input);
  void frameEnd();
  void handleOSEvents(Input* input);
  void initRenderer();
  void clean();

  // varies from platform to platform
  static int isShiftActive();
  static int isCapsLockActive();
  static void registerKeyboardHook();
  static void onKeyPress(char pressed);
  static void simulateKeyboardInput(int abbreviationLength, std::string toSend);

  std::string version = "1.0";

  SDL_Window* window;
  SDL_GLContext context;
  SDL_Renderer* renderer;

  static AppData* data;


  bool isRunning;
#if WINDOWS_BUILD
  static HHOOK keylistener;
#endif
};

#endif

