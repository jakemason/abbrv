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


// these are included from the game in Platform.cpp
struct Memory;
class Input;

class Platform
{
public:
  Platform();
  ~Platform();

  void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
  void handleOSEvents();
  void initRenderer();
  void clean();

  std::string version = "1.0";

  bool isRunning;
};

#endif
