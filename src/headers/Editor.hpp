#pragma once
#ifndef EDITOR_HPP
#define EDITOR_HPP

#define GLEW_STATIC

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Data.hpp"
#include "Icons.hpp"
#include "Input.hpp"
#include "Platform.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "imgui_internal.h"

#define UTIL_COLUMN_SIZE 35.0f

#if WIN32
#include <tchar.h>
#endif

class Editor
{
public:
  static void setEditorStyles()
  {
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.0f, 0.0f));
  }

  static void render(Platform* platform, Input* input, AppData* data)
  {
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
      if (ImGui::BeginMenu("Config"))
      {
        if (ImGui::MenuItem("Open in Explorer"))
        {
#if WIN32
          PROCESS_INFORMATION ProcessInfo = {0};
          STARTUPINFO StartupInfo         = {0};
          std::string command             = "explorer.exe .";
          char cmd[128];
          strcpy(cmd, command.c_str());
          CreateProcess(NULL, _T(cmd), NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo);
#endif
        }
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Help"))
      {
        std::string version = "abbrv v." + platform->version;
        ImGui::Text(version.c_str());
        ImGui::Separator();
        ImGui::Text(ICON_FA_COPYRIGHT " 2022 Jake Mason. All rights reserved.");
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    int columns = 4;
    bool open   = true;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight));

    ImGui::Begin("Main Window", &open,
                 ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_HiddenTabBar | ImGuiWindowFlags_NoResize);
    if (ImGui::BeginTable("dataTable", columns, ImGuiTableFlags_Borders))
    {

      ImGui::TableSetupColumn("Abbreviation");
      ImGui::TableSetupColumn("Expands To");
      ImGui::TableSetupColumn("##lines", ImGuiTableColumnFlags_WidthFixed, UTIL_COLUMN_SIZE);
      ImGui::TableSetupColumn("##delete", ImGuiTableColumnFlags_WidthFixed, UTIL_COLUMN_SIZE);
      ImGui::TableHeadersRow();

      for (int row = 0; row < data->entries.size(); row++)
      {
        if (row == 0)
        {
          ImGui::TableSetColumnIndex(0);
          ImGui::PushItemWidth(-FLT_MIN);
          ImGui::TableSetColumnIndex(1);
          ImGui::PushItemWidth(-FLT_MIN);
        }
        ImGui::TableNextRow();
        int column = 0;
        ImGui::TableSetColumnIndex(column);
        // ImGui::Text("Row %d Column %d", row, column);
        ImGui::PushID(row * columns + column); // assign unique id
        if (ImGui::InputText("##v", data->entries[row].abbreviation, IM_ARRAYSIZE(data->entries[row].abbreviation)))
        {
          data->saveToFile();
        }
        ImGui::PopID();

        column = 1;
        ImGui::TableSetColumnIndex(column);
        // ImGui::Text("Row %d Column %d", row, column);
        ImGui::PushID(row * columns + column); // assign unique id
        if (data->entries[row].isMultiline)
        {
          if (ImGui::InputTextMultiline("##v", data->entries[row].expandsTo,
                                        IM_ARRAYSIZE(data->entries[row].expandsTo)))
          {
            data->saveToFile();
          }
        }
        else
        {
          if (ImGui::InputText("##v", data->entries[row].expandsTo, IM_ARRAYSIZE(data->entries[row].expandsTo)))
          {
            data->saveToFile();
          }
        }
        ImGui::PopID();

        column = 2;


        ImVec2 button_size(UTIL_COLUMN_SIZE, ImGui::GetFontSize() * 2.0f);
        ImGui::TableSetColumnIndex(column);
        // ImGui::Text("Row %d Column %d", row, column);
        ImGui::PushID(row * columns + column); // assign unique id


        std::string icon = data->entries[row].isMultiline ? ICON_FA_MINUS : ICON_FA_BARS;
        if (ImGui::Button(icon.c_str(), button_size))
        {
          data->entries[row].isMultiline = !data->entries[row].isMultiline;
        }
        if (ImGui::IsItemHovered())
        {
          std::string text =
              data->entries[row].isMultiline ? "Reduce to a single line entry" : "Expand to a multiline entry";
          ImGui::SetTooltip(text.c_str());
        }
        ImGui::PopID();


        column = 3;
        ImGui::TableSetColumnIndex(column);
        // ImGui::Text("Row %d Column %d", row, column);
        ImGui::PushID(row * columns + column); // assign unique id


        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(7.0f, 0.8f, 0.8f));
        if (ImGui::Button(ICON_FA_TRASH, button_size)) { data->deleteIndex(row); }
        if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Delete this pair. This cannot be undone."); }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
      }
      ImGui::EndTable();

      ImVec2 button_size(ImGui::GetFontSize() * 3.0f, ImGui::GetFontSize() * 2.0f);
      if (ImGui::Button(ICON_FA_PLUS, button_size)) { data->entries.push_back({}); }
      if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Add a new abbreviation & expansion pair."); }
    }

    ImGui::End();
  }
};

#endif
