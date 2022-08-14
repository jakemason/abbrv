cmake_minimum_required(VERSION 2.8.12)
project(displace)

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

include_directories(./include ./include/GL ./src/headers)
link_directories(./lib ./bin)

# Disable certain compiler warnings, take care here!
# Also, enable parallel builds for faster compile times
add_compile_options(/wd4244 /wd4996 /MP)

add_definitions(-DDebugMode)

#options
option(EDITOR_MODE "Enables the game engine editor for development" ON)
option(SDL_RENDERER "Enable the built-in SDL_Renderer (software rendering)" OFF)
option(OPENGL_RENDERER "Enable the OpenGL renderer" ON)
option(WINDOWS_TARGET "Build for Microsoft's Windows Operating System" ON)
option(ENABLE_DEBUG "Enables various debugging outputs and logging" ON)

if(EDITOR_MODE)
    add_definitions(-DEDITOR_MODE)
endif()

if(SDL_RENDERER)
    add_definitions(-DSDL_RENDERER)
endif()

if(OPENGL_RENDERER)
    add_definitions(-DOPENGL_RENDERER)
endif()

if(WINDOWS_TARGET)
    add_definitions(-DWINDOWS_BUILD)
endif()

if(ENABLE_DEBUG)
    add_definitions(-DDEBUG_MODE)
else()
    add_link_options(/LTCG) # Suggested when using "/GL"
    add_compile_options(/O2 /GL) #optimization
    # Not sure where these flags are getting added, but they conflict
    # with /O2 so they get stripped here.
    RemoveCXXFlag("/RTC1")
    RemoveCXXFlag("/Od")
endif()


file(GLOB_RECURSE SOURCES "./src/*.cpp")

# NOTE: On Windows Icons -- 
# The "displace.rc" file is a convention needed to add an icon and other
# built-into-the-binary data on Windows. Setting the icon through
# SDL_SetWindowIcon is NOT good enough! That only sets the taskbar icon, but
# not _most_ of the other sizes. Thus, we use them both. The .png used by SDL
# is crisper, but the .ico file meets the other sizing / placement
# requirements.
add_executable(displace ${SOURCES} ./src/displace.rc)
# TODO: 
# WIN32 flag here disables the console which we want, but not before we've
# added an in-editor console via IMGUI
target_link_libraries(displace glew32 opengl32)
