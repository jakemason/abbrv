/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-15-2022
 *
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
