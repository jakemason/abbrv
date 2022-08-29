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

#include <windows.h>

#include "Debug.hpp"
#include "Editor.hpp"
#include "Platform.hpp"
#include "SDL_syswm.h"

int Platform::isShiftActive() { return GetKeyState(VK_LSHIFT) < 0 || GetKeyState(VK_RSHIFT) < 0; }

int Platform::isCapsLockActive() { return (GetKeyState(VK_CAPITAL) & 1) == 1; }

void Platform::onKeyPress(char pressed)
{
  DEBUG("Input received. char: %c, value of %d", pressed, (int)pressed);

  const int MODIFIER_PRESSED = -52; // both alt and ctrl return -52 and we don't want to consider
                                    // these "breaks" in the matching chain so don't report them
  const int SHIFT_RELEASED = 0;     // we also need to ignore when shift is released for the same
                                    // reasons


  // finally, if our Editor inputs are active we also want to bail because we don't want the
  // autocomplete triggering while the user is editing their settings.
  bool windowHasInputFocus       = (SDL_GetWindowFlags(Platform::window) & SDL_WINDOW_INPUT_FOCUS);
  bool inputsAndWindowAreaActive = Editor::anInputIsActive && windowHasInputFocus;
  if (pressed == MODIFIER_PRESSED || pressed == SHIFT_RELEASED || inputsAndWindowAreaActive) return;

  data->advanceSearches(pressed);
  Abbreviation* toSend = data->checkForCompletions();
  if (toSend != nullptr)
  {
    std::string s(toSend->expandsTo);
    simulateKeyboardInput(strlen(toSend->abbreviation), s);
  }
}

void Platform::addTrayIcon(SDL_Window* window)
{
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);

  NOTIFYICONDATA iconData;
  if (SDL_GetWindowWMInfo(window, &info))
  {
    iconData.uCallbackMessage = WM_USER + 1;
    iconData.uFlags           = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    int iconConst             = 102; // NOTE: defined in "abbrv.rc"
    iconData.hIcon            = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(iconConst));
    iconData.cbSize           = sizeof(iconData);
    iconData.hWnd             = info.info.win.window;
    strcpy_s(iconData.szTip, "abbrv");

    Shell_NotifyIcon(NIM_ADD, &iconData);
  }
}

void Platform::removeTrayIcon(SDL_Window* window)
{
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);

  NOTIFYICONDATA iconData;
  if (SDL_GetWindowWMInfo(window, &info))
  {
    iconData.cbSize = sizeof(iconData);
    iconData.hWnd   = info.info.win.window;
    Shell_NotifyIcon(NIM_DELETE, &iconData);
  }
}

// The function that implements the key logging functionality
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  // We do this on KEYUP so we can insert our expansion _after_ the abbreviation
  // is completed. This also makes it much easier to issue the required count
  // of backspaces. Doing it on KEYDOWN doesn't work very well.
  if (wParam == WM_KEYUP)
  {
    KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
    DWORD wVirtKey             = kbdStruct->vkCode;
    DWORD wScanCode            = kbdStruct->scanCode;

    BYTE lpKeyState[256];
    GetKeyboardState(lpKeyState);
    lpKeyState[VK_SHIFT]   = 0;
    lpKeyState[VK_CAPITAL] = 0;
    if (Platform::isShiftActive()) { lpKeyState[VK_SHIFT] = 0x80; }
    if (Platform::isCapsLockActive()) { lpKeyState[VK_CAPITAL] = 0x01; }

    char result;
    ToAscii(wVirtKey, wScanCode, lpKeyState, (LPWORD)&result, 0);
    Platform::onKeyPress(result);
  }

  return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void Platform::simulateKeyboardInput(int abbreviationLength, std::string toSend)
{
  // disable our hook here so we can't have an abbreviation that creates an expansion which
  // creates an abbreviation which creates an expansion which creates an expansion...
  UnhookWindowsHookEx(keyboardHook);
  HKL kbl = GetKeyboardLayout(0);


  // TODO: Max limit here?
  INPUT inputs[4096] = {};

  int totalPresses = 0;

  // send a backspace for each character in our abbreviation
  for (int i = 0; i < abbreviationLength * 2; i += 2)
  {
    inputs[i]            = {};
    inputs[i].type       = INPUT_KEYBOARD;
    inputs[i].ki.wVk     = VK_BACK;
    inputs[i].ki.dwFlags = 0;

    inputs[i + 1]            = {};
    inputs[i + 1].type       = INPUT_KEYBOARD;
    inputs[i + 1].ki.wVk     = VK_BACK;
    inputs[i + 1].ki.dwFlags = KEYEVENTF_KEYUP;
    totalPresses += 2;
  }

  int offset = abbreviationLength * 2;

  int characterIndex   = 0;
  int additionalStride = 0;
  for (int i = 0; i < toSend.size() * 2; i += 2)
  {
    SHORT vk           = VkKeyScanEx(toSend[characterIndex], kbl);
    bool shiftModifier = vk & 0x100;
    if (shiftModifier) // if shift was held
    {
      inputs[i + offset + additionalStride]            = {};
      inputs[i + offset + additionalStride].type       = INPUT_KEYBOARD;
      inputs[i + offset + additionalStride].ki.wVk     = VK_LSHIFT;
      inputs[i + offset + additionalStride].ki.dwFlags = 0;
      totalPresses++;
      additionalStride++; // offset our character insert by 1 as a shift must precede it
    }

    // KEY DOWN for the character
    inputs[i + offset + additionalStride]            = {};
    inputs[i + offset + additionalStride].ki.dwFlags = 0;
    inputs[i + offset + additionalStride].type       = INPUT_KEYBOARD;
    inputs[i + offset + additionalStride].ki.wVk     = vk & 0xFF;
    totalPresses++;

    // KEY UP for the character
    inputs[i + offset + additionalStride + 1]            = {};
    inputs[i + offset + additionalStride + 1].type       = INPUT_KEYBOARD;
    inputs[i + offset + additionalStride + 1].ki.dwFlags = KEYEVENTF_KEYUP;
    totalPresses++;


    if (shiftModifier) // release shift if we sent it earlier
    {
      additionalStride++; // now we bump the stride by another one as we have a shift release after
                          // the character
      inputs[i + offset + additionalStride + 1]            = {};
      inputs[i + offset + additionalStride + 1].type       = INPUT_KEYBOARD;
      inputs[i + offset + additionalStride + 1].ki.wVk     = VK_LSHIFT;
      inputs[i + offset + additionalStride + 1].ki.dwFlags = KEYEVENTF_KEYUP;
      totalPresses++;
    }

    characterIndex += 1;
  }

  SendInput(totalPresses, inputs, sizeof(INPUT));

  registerKeyboardHook();
}

void Platform::registerKeyboardHook()
{
  // Retrieve the applications instance
  HINSTANCE instance = GetModuleHandle(NULL);
  // Set a global Windows Hook to capture keystrokes using the function declared above
  keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, instance, 0);
}
