/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-13-2022
 *
 **/
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <tchar.h>
#include <windows.h>

#include <ctime>
#include <string>
#include <vector>

#include "Data.hpp"
#include "Debug.hpp"
#include "Icons.hpp"
#include "Input.hpp"
#include "Platform.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "imgui_internal.h"

int main(int argc, char* args[])
{
  Input* input       = new Input();
  Platform* platform = new Platform();
  AppData* data      = new AppData();
  data->init();
  int screenWidth  = 800;
  int screenHeight = 500;
  int fullscreen   = 0;
  platform->init("abbrv", 50, 50, screenWidth, screenHeight, fullscreen);
  platform->registerKeyboardHook();
  int countFrequency = SDL_GetPerformanceFrequency();
  float deltaTime    = 0.0f;
  Debug::init();
  platform->init();


  while (platform->isRunning)
  {
    int prevCounter = SDL_GetPerformanceCounter();

    platform->handleOSEvents(input);
    platform->io(deltaTime, input);
    platform->frameStart(input);


    { // app logic
      int screenWidth, screenHeight;
      SDL_GetWindowSize(platform->window, &screenWidth, &screenHeight);
      if (input->windowResized)
      {
#if OPENGL_RENDERER
        glEnable(GL_SCISSOR_TEST);
        //  the scissor prevents pixels from being rendered outside of the frame.
        glScissor(0, 0, screenWidth, screenHeight);
#endif
      }
      glViewport(0, 0, screenWidth, screenHeight);

      if (ImGui::BeginMainMenuBar())
      {
        if (ImGui::BeginMenu("File")) { ImGui::EndMenu(); }
        ImGui::EndMainMenuBar();
      }

      int columns = 3;
      bool open   = true;
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight));

      ImGui::Begin("Main Window", &open,
                   ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_HiddenTabBar | ImGuiWindowFlags_NoResize);
      if (ImGui::BeginTable("table1", columns, ImGuiTableFlags_Borders))
      {

        ImGui::TableSetupColumn("Abbreviation");
        ImGui::TableSetupColumn("Expands To");
        ImGui::TableSetupColumn(" ", 50.0f);
        ImGui::TableHeadersRow();

        for (int row = 0; row < data->entries.size(); row++)
        {
          if (row == 0)
          {
            ImGui::TableSetColumnIndex(0);
            ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
            ImGui::TableSetColumnIndex(1);
            ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
            // ImGui::TableSetColumnWidth(1, 100);
          }
          ImGui::TableNextRow();
          int column = 0;
          ImGui::TableSetColumnIndex(column);
          // ImGui::Text("Row %d Column %d", row, column);
          ImGui::PushID(row * 3 + column); // assign unique id
          if (ImGui::InputText("##v", data->entries[row].abbreviation, IM_ARRAYSIZE(data->entries[row].abbreviation)))
          {
            data->saveToFile();
          }
          ImGui::PopID();

          column = 1;
          ImGui::TableSetColumnIndex(column);
          // ImGui::Text("Row %d Column %d", row, column);
          ImGui::PushID(row * 3 + column); // assign unique id
          if (ImGui::InputText("##v", data->entries[row].expandsTo, IM_ARRAYSIZE(data->entries[row].expandsTo)))
          {
            data->saveToFile();
          }
          ImGui::PopID();


          column = 2;
          ImGui::TableSetColumnIndex(column);
          // ImGui::Text("Row %d Column %d", row, column);
          ImGui::PushID(row * 3 + column); // assign unique id


          ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(7.0f, 0.7f, 0.7f));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(7.0f, 0.8f, 0.8f));
          if (ImGui::Button(ICON_FA_TRASH)) { data->deleteIndex(row); }
          ImGui::PopStyleColor(3);
          ImGui::PopID();
        }
        ImGui::EndTable();

        if (ImGui::Button(ICON_FA_PLUS " New Entry")) { data->entries.push_back({}); }
      }


      ImGui::End();
    }


    platform->frameEnd();


#if OPENGL_RENDERER
    SDL_GL_SwapWindow(platform->window);
#endif
    int nowCounter     = SDL_GetPerformanceCounter();
    int counterElapsed = nowCounter - prevCounter;
    deltaTime          = (((float)counterElapsed * 1000.0f) / (float)countFrequency) / 1000.0f;
    prevCounter        = nowCounter;
  }

  return 0;
}
