/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-17-2022
 *
 **/

#include "Debug.hpp"

#include <ctime>
#include <string>

#if WIN32
#include <windows.h>
#endif

#ifndef DEBUG_COLORS

#if WIN32
#define RED    4
#define YELLOW 14
#define RESET  15
#else
#define RED    "\033[0;31m"
#define YELLOW "\033[33;1m"
#define RESET  "\033[0m"
#endif

#define DEBUG_COLORS
#endif

// Easier way to handle the vargs shenanigans than writing a helper function
#define PRINT_OUTPUT                                                                                                   \
  std::string prefix = file;                                                                                           \
  prefix += ": ";                                                                                                      \
  prefix += std::to_string(line);                                                                                      \
  prefix += " -- ";                                                                                                    \
  std::string with_newline = prefix + format;                                                                          \
                                                                                                                       \
  va_list args;                                                                                                        \
  va_start(args, format);                                                                                              \
                                                                                                                       \
  char buffer[256]{};                                                                                                  \
  vsprintf(buffer, with_newline.c_str(), args);                                                                        \
  std::string currentLine = buffer;                                                                                    \
                                                                                                                       \
  if (currentLine == Debug::lastLine)                                                                                  \
  {                                                                                                                    \
    Debug::timesRepeated += 1;                                                                                         \
    printf("\r");                                                                                                      \
    std::string timesRepeatedString = "  [" + std::to_string(Debug::timesRepeated) + "]";                              \
    vprintf((prefix + format + timesRepeatedString).c_str(), args);                                                    \
  }                                                                                                                    \
  else                                                                                                                 \
  {                                                                                                                    \
    printf("\n");                                                                                                      \
    Debug::timesRepeated = 0;                                                                                          \
    vprintf(with_newline.c_str(), args);                                                                               \
  }                                                                                                                    \
  Debug::lastLine = currentLine;                                                                                       \
  va_end(args);

FILE* Debug::file;
std::string Debug::lastLine = "";
int Debug::timesRepeated    = 0;

void Debug::init()
{
  std::time_t now = std::time(NULL);
  char formatted_time[80];
  strftime(formatted_time, 80, "%Y-%m-%d_%Hh%Mm%Ss", localtime(&now));
  std::string filename = "logs/log_";
  filename += std::string(formatted_time);
  filename += ".log";
  Debug::file = fopen(filename.c_str(), "w+");
}

void Debug::log(const char* file, int line, std::string format, ...) { PRINT_OUTPUT; }

void Debug::warn(const char* file, int line, std::string format, ...)
{
#if WIN32
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(console, YELLOW);
  printf("[WARN]: ");
  SetConsoleTextAttribute(console, RESET);
#else
  printf(YELLOW);
  printf("[WARN]: ");
  printf(RESET);
#endif
  PRINT_OUTPUT;
}

void Debug::error(const char* file, int line, std::string format, ...)
{
#if WIN32
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(console, RED);
  printf("[ERROR]: ");
  SetConsoleTextAttribute(console, RESET);
#else
  printf(RED);
  printf("[ERROR]: ");
  printf(RESET);
#endif
  PRINT_OUTPUT;
}
