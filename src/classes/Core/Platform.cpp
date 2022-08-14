/**
 * Displace Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-13-2022
 *
 **/
#define GL3_PROTOTYPES 1
#include "Core/Platform.hpp"

#include <GL/glew.h>

#include <fstream>

#include "Debug.hpp"

extern "C"
{
  // Forces some laptops to use the dedicated graphics card
  __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

Platform::Platform() {}
Platform::~Platform() {}

void Platform::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
{
  std::string fullTitle = std::string(title);
  fullTitle             = fullTitle + " " + version;
  // NOTE: This sets DPI awareness for modern laptops and such. Windows
  // advises _against_ doing this in code and, instead, suggests using
  // a manifest.xml file to specify it. The manifest.xml setup destroys my
  // entire build so... code it is.
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

  // TODO: CREATE WINDOW HERE
  if (true) { DEBUG("Window created successfully."); }

  // NOTE
  // This is NOT good enough by itself. An ".ico" file is required as well
  // and baked directly into the program itself through CMake. See
  // "platform/CMakeLists.txt" for more information, as well as the
  // associated "displace.rc" file.

  // this line is needed to ensure that textures are recreated when the
  // window is resized. If we remove this line and resize the window we
  // are presented with black boxes where our textures used to be.

#if WINDOWS_BUILD
  fullTitle += " | Windows OS";
#endif

#if DEBUG_MODE
  fullTitle += " | Debugging Enabled";
#endif

  isRunning = true;
}

void Platform::initRenderer()
{
  // TODO:
}

void Platform::handleOSEvents() {}

void Platform::clean() { DEBUG("Platform exited successfully."); }
