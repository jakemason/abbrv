#include <windows.h>

#include "Debug.hpp"
#include "Platform.hpp"
#include "SDL_syswm.h"

int Platform::isShiftActive() { return GetKeyState(VK_LSHIFT) < 0 || GetKeyState(VK_RSHIFT) < 0; }

int Platform::isCapsLockActive() { return (GetKeyState(VK_CAPITAL) & 1) == 1; }

void Platform::onKeyPress(char pressed)
{
  const int BACKSPACE_PRESSED = 8;   // TODO: Use this to "back up" the Tries?
  const int MODIFIER_PRESSED  = -52; // both alt and ctrl return -52 and we don't want to consider
                                     // these "breaks" in the matching chain so don't report them
  if ((int)pressed == MODIFIER_PRESSED) return;

  DEBUG("Input received. char: %c, value of %d", pressed, (int)pressed);
  // TODO: Okay, so now we just do the Trie thing!
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
    // iconData.hIcon            = LoadIcon(NULL, IDI_INFORMATION);
    int iconConst   = 102; // NOTE: defined in "abbrv.rc"
    iconData.hIcon  = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(iconConst));
    iconData.cbSize = sizeof(iconData);
    iconData.hWnd   = info.info.win.window;
    strcpy_s(iconData.szTip, "abbrv");

    bool success = Shell_NotifyIcon(NIM_ADD, &iconData);
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
    bool success    = Shell_NotifyIcon(NIM_DELETE, &iconData);
  }
}


// The function that implements the key logging functionality
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  // We o this on KEYUP so we can insert our expansion _after_ the abbreviation
  // is completed. This also makes it much easier to issue the required count
  // of backspaces.
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
#if WIN32
  HKL kbl = GetKeyboardLayout(0);

  // the final backspace happens at the end!
  for (int i = 0; i < abbreviationLength; i++)
  {
    INPUT input  = {};
    input.type   = INPUT_KEYBOARD;
    input.ki.wVk = VK_BACK;
    SendInput(1, &input, sizeof(INPUT));

    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
  }

  for (int i = 0; i < toSend.size(); i++)
  {
    INPUT input        = {};
    input.type         = INPUT_KEYBOARD;
    SHORT vk           = VkKeyScanEx(toSend[i], kbl);
    bool shiftModifier = vk & 0x100;
    if (shiftModifier) // if shift was held
    {
      INPUT shift  = {};
      shift.type   = INPUT_KEYBOARD;
      shift.ki.wVk = VK_LSHIFT;
      SendInput(1, &shift, sizeof(INPUT));
    }


    input.ki.wVk = vk & 0xFF;
    SendInput(1, &input, sizeof(INPUT));
    // NOTE: You NEED to send a key up after the keydown otherwise Windows eats characters oddly
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));

    if (shiftModifier) // release shift if we sent it earlier
    {
      INPUT shift      = {};
      shift.type       = INPUT_KEYBOARD;
      shift.ki.wVk     = VK_LSHIFT;
      shift.ki.dwFlags = KEYEVENTF_KEYUP;
      SendInput(1, &shift, sizeof(INPUT));
    }
  }
#endif
}

void Platform::registerKeyboardHook()
{
#if WIN32
  // Retrieve the applications instance
  HINSTANCE instance = GetModuleHandle(NULL);
  // Set a global Windows Hook to capture keystrokes using the function declared above
  keylistener = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, instance, 0);
#endif
}