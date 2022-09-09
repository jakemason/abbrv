/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 1.4.1
 * @author Jake Mason
 * @date 09-05-2022
 *
 * abbrv is licensed under the Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License
 *
 * See LICENSE.txt for more information
 **/

#pragma once
#ifndef INPUT_HPP
#define INPUT_HPP

// This size is dictated by:
// https://wiki.libsdl.org/SDLScancodeLookup
#define SDL_SCAN_CODES_COUNT 284

#include <string>

class Input
{
public:
  int rawMouseX;
  int rawMouseY;
  int mouseX;
  int mouseY;
  bool mousePosUpdated;

  int mouseScroll;
  bool leftClicked;
  bool rightClicked;
  bool leftClickHeld;
  bool rightClickHeld;
  bool middleClicked;
  bool middleClickHeld;

  int windowID;
  bool windowResized;
  bool windowClosed;
  bool windowMoved;


  /*
   * TODO:
   * Currently these are case sensitive which isn't great.
   */
  bool isKeyPressed[SDL_SCAN_CODES_COUNT];
  bool isKeyHeld[SDL_SCAN_CODES_COUNT];
  bool textInputTrigger;
  std::string textInput;

  bool shiftHeld = false;
  bool ctrlHeld  = false;
};

#endif
