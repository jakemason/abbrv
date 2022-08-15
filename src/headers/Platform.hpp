/**
 * Displace Source Code
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
  void registerKeyboardHook();

  static int isShiftActive();
  static int isCapsLockActive();
  static void onKeyPress(char pressed);
  void handleOSEvents(Input* input);
  void initRenderer();
  void clean();

  std::string version = "1.0";


  SDL_Window* window;
  SDL_GLContext context;
  SDL_Renderer* renderer;


  bool isRunning;
};

#endif

