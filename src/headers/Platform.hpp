/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-17-2022
 *
 **/

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

  void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
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

  std::string version = "1.0";

  SDL_Window* window;
  SDL_GLContext context;
  SDL_Renderer* renderer;

  static AppData* data;


  bool isRunning;
#if WIN32
  static HHOOK keylistener;
  static UINT WM_TASKBARCREATED;
  static void addTrayIcon(SDL_Window* window);
  static void removeTrayIcon(SDL_Window* window);
#endif
};

#endif

