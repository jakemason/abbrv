/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 1.2
 * @author Jake Mason
 * @date 08-19-2022
 *
 * abbrv is licensed under the Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License
 *
 * See LICENSE.txt for more information
 **/

#pragma once
#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdarg.h>
#include <stdio.h>

#include <string>

#if WIN32
#include <windows.h>
#endif

#if DEBUG_MODE

#define CRY() Debug::warn("CRY");

// '##' removes comma if __VA_ARGS__ is empty. Allows calling:
// DEBUG("No variables needed");
#define ERR(format, ...)   Debug::error(__FILE__, __LINE__, format, ##__VA_ARGS__);
#define WARN(format, ...)  Debug::warn(__FILE__, __LINE__, format, ##__VA_ARGS__);
#define DEBUG(format, ...) Debug::log(__FILE__, __LINE__, format, ##__VA_ARGS__);
// Conditional DEBUG
#define CDEBUG(c, format, ...)                                                                                         \
  if (c) { Debug::log(format, ##__VA_ARGS__); }

#else
// TODO: Do we really only want this while in debug mode? We probably want to be
// able to retrieve player logs even when they're playing in production. Being
// able to debug with greater ease is likely better than trying to save some CPU
// cycles here. Requires more research.
#define ERR(format, ...)
#define WARN(format, ...)
#define DEBUG(format, ...)
#define CDEBUG(c, format, ...)

#endif

class Debug
{
public:
  static void init();
  static void error(const char* file, int line, std::string format, ...);
  static void log(const char* file, int line, std::string format, ...);
  static void warn(const char* file, int line, std::string format, ...);

  static FILE* file;
  static std::string lastLine;
  static int timesRepeated;
};

#endif
