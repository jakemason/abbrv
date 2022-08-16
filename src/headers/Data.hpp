#pragma once
#ifndef DATA_HPP
#define DATA_HPP

#define ABBREVIATION_MAX_SIZE 1024 - 1
#define EXPAND_MAX_SIZE       4096 - 1

#define SAVE_FILE_NAME "config.abbrv"

#include <fstream>
#include <string>
#include <vector>

#include "Debug.hpp"
#include "imgui.h"

struct Abbreviations
{
  char abbreviation[ABBREVIATION_MAX_SIZE];
  char expandsTo[EXPAND_MAX_SIZE];
};

class AppData
{
public:
  void init() { readSaveFile(); }

  void deleteIndex(int index)
  {
    entries.erase(entries.begin() + index);
    saveToFile();
  }

  void readSaveFile()
  {
    std::ifstream file;
    file.open("./" SAVE_FILE_NAME);
    if (!file) { ERR("Failed to open the config file %s", SAVE_FILE_NAME); }
    std::string abbreviation;
    std::string expandsTo;
    while (!file.eof())
    {
      std::getline(file, abbreviation, ':');
      DEBUG("Abbreviation: [%s]", abbreviation.c_str());
      std::getline(file, expandsTo, '\n');
      DEBUG("Expands To: [%s]", expandsTo.c_str());

      if (abbreviation != "")
      {
        Abbreviations toAdd;
        strcpy(toAdd.abbreviation, abbreviation.c_str());
        strcpy(toAdd.expandsTo, expandsTo.c_str());
        entries.push_back(toAdd);
      }
    }
  }

  void saveToFile()
  {
    std::ofstream file;
    file.open("./" SAVE_FILE_NAME);
    if (!file)
    {
      ERR("Failed to open the config file %s", SAVE_FILE_NAME);
      return;
    }
    for (int i = 0; i < entries.size(); i++)
    {
      file << entries[i].abbreviation << ":" << entries[i].expandsTo << std::endl;
    }

    DEBUG("SAVED");
  }

  std::vector<Abbreviations> entries;
};

#endif
