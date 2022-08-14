/**
 * Displace Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-13-2022
 *
 **/
#include <windows.h>

#include <ctime>
#include <string>

#include "Core/Platform.hpp"
#include "Debug.hpp"

int main(int argc, char* args[])
{
  Platform* platform = new Platform();
  int screenWidth    = 500;
  int screenHeight   = 500;
  int fullscreen     = 0;
  platform->init("Displace", 50, 50, screenWidth, screenHeight, fullscreen);
  DEBUG("Platform initialization complete.");
  Debug::init();

  while (platform->isRunning)
  {
    // BEGIN GAME LOOP
    platform->handleOSEvents();
  }

  return 0;
}
