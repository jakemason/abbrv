cmake_minimum_required(VERSION 2.8.12)
project(abbrv)

macro(RemoveCXXFlag flag)
  string(REPLACE "${flag}" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  string(REPLACE "${flag}" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endmacro()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./bin)

include_directories(
  ./include 
  ./include/GL
  ./src/headers
  ./include/SDL2
  ./src/headers/External
)
link_directories(./lib)

# Disable certain compiler warnings, take care here!
# Also, enable parallel builds for faster compile times
add_compile_options(/wd4244 /wd4996 /MP)

#options
option(OPENGL_RENDERER "Enable the OpenGL renderer" ON)
option(ENABLE_DEBUG "Enables various debugging outputs and logging" ON)

if(OPENGL_RENDERER)
  add_definitions(-DOPENGL_RENDERER)
endif()

if(ENABLE_DEBUG)
  add_definitions(-DDEBUG_MODE)
else()
  # /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF 
  # (generate PDB, disable incremental linking, remove unreferenced sections, merge identical sections).
  #
  # /LTCG
  # Suggested when using "/GL"
  add_link_options(/LTCG /DEBUG /OPT:REF /OPT:ICF)


  add_compile_options(/O2 /GL) #optimization
  # Not sure where these flags are getting added, but they conflict
  # with /O2 so they get stripped here.
  RemoveCXXFlag("/RTC1")
  RemoveCXXFlag("/Od")
  RemoveCXXFlag("/MD")

  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /Zi")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./abbrv)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./abbrv)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./abbrv)
endif()


file(GLOB_RECURSE SOURCES "./src/*.cpp")

# NOTE: On Windows Icons -- 
# The "abbrv.rc" file is a convention needed to add an icon and other
# built-into-the-binary data on Windows. Setting the icon through
# SDL_SetWindowIcon is NOT good enough! That only sets the taskbar icon, but
# not _most_ of the other sizes. Thus, we use them both. The .png used by SDL
# is crisper, but the .ico file meets the other sizing / placement
# requirements.
#
# TODO: 
# WIN32 flag here disables the console which we want, but not before we've
# added an in-editor console via IMGUI

if(ENABLE_DEBUG)
  add_executable(abbrv ${SOURCES} ./src/abbrv.rc)
else()
  add_executable(abbrv WIN32 ${SOURCES} ./src/abbrv.rc)
endif()


target_link_libraries(abbrv setupapi.lib winmm.lib imm32.lib version.lib glew32s.lib opengl32.lib SDL2main.lib SDL2-static.lib)
