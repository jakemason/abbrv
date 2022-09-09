/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 1.4
 * @author Jake Mason
 * @date 09-01-2022
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
    simulateKeyboardInput((int)strlen(toSend->abbreviation), s);
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

  INPUT inputs[2 * ABBREVIATION_MAX_SIZE + 4 * EXPAND_MAX_SIZE] = {};
  int inputCount = 0;

  // send a backspace for each character in our abbreviation
  for (int i = 0; i < abbreviationLength; i++)
  {
    inputs[inputCount]            = {};
    inputs[inputCount].type       = INPUT_KEYBOARD;
    inputs[inputCount].ki.wVk     = VK_BACK;
    inputs[inputCount].ki.dwFlags = 0;
    inputCount += 1;

    inputs[inputCount]            = {};
    inputs[inputCount].type       = INPUT_KEYBOARD;
    inputs[inputCount].ki.wVk     = VK_BACK;
    inputs[inputCount].ki.dwFlags = KEYEVENTF_KEYUP;
    inputCount += 1;
  }

  for (int characterIndex = 0; characterIndex < toSend.size(); characterIndex++)
  {
    SHORT vk           = VkKeyScanEx(toSend[characterIndex], kbl);
    bool shiftModifier = vk & 0x100;
    if (shiftModifier) // if shift was held
    {
      inputs[inputCount]            = {};
      inputs[inputCount].type       = INPUT_KEYBOARD;
      inputs[inputCount].ki.wVk     = VK_LSHIFT;
      inputs[inputCount].ki.dwFlags = 0;
      inputCount++;
    }

    // KEY DOWN for the character
    inputs[inputCount]            = {};
    inputs[inputCount].ki.dwFlags = 0;
    inputs[inputCount].type       = INPUT_KEYBOARD;
    inputs[inputCount].ki.wVk     = vk & 0xFF;
    inputCount++;

    // KEY UP for the character
    inputs[inputCount]            = {};
    inputs[inputCount].type       = INPUT_KEYBOARD;
    inputs[inputCount].ki.dwFlags = KEYEVENTF_KEYUP;
    inputCount++;

    if (shiftModifier) // release shift if we sent it earlier
    {
      inputs[inputCount]            = {};
      inputs[inputCount].type       = INPUT_KEYBOARD;
      inputs[inputCount].ki.wVk     = VK_LSHIFT;
      inputs[inputCount].ki.dwFlags = KEYEVENTF_KEYUP;
      inputCount++;
    }
  }

  SendInput(inputCount, inputs, sizeof(INPUT));

  registerKeyboardHook();
}

void Platform::registerKeyboardHook()
{
  // Retrieve the applications instance
  HINSTANCE instance = GetModuleHandle(NULL);
  // Set a global Windows Hook to capture keystrokes using the function declared above
  keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, instance, 0);
}
