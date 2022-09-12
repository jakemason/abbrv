/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 1.5
 * @author Jake Mason
 * @date 09-09-2022
 *
 * abbrv is licensed under the Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License
 *
 * See LICENSE.txt for more information
 **/

#pragma once
#ifndef DATA_HPP
#define DATA_HPP

#define ABBREVIATION_MAX_SIZE 1024
#define EXPAND_MAX_SIZE       4096

// we allow all ASCII entries
#define ALPHABET_SIZE           128
#define ABBRV_SAVE_FILE_VERSION "ABBRV_SAVE_1_0"
#define SAVE_FILE_NAME          "config.abbrv"

#include <fstream>
#include <string>
#include <vector>

#include "Debug.hpp"
#include "Serialization.hpp"
#include "imgui.h"

struct Abbreviation
{
  char abbreviation[ABBREVIATION_MAX_SIZE];
  char expandsTo[EXPAND_MAX_SIZE];
  bool isMultiline;
  bool isHiddenField = false;
};

struct TrieNode
{
  bool terminal;
  Abbreviation *abbreviation;
  TrieNode *children[ALPHABET_SIZE];

  static void insert(TrieNode *root, std::string key, Abbreviation *abbreviation)
  {
    DEBUG("Inserting %s into Trie", key.c_str());
    TrieNode *current = root;

    for (int i = 0; i < key.length(); i++)
    {
      int index = key[i];
      DEBUG("Looking for index %d", index);
      if (!current->children[index]) { current->children[index] = getNode(); }

      current = current->children[index];
    }

    current->terminal     = true;
    current->abbreviation = abbreviation;
  }

  // Because  we're doing partial matching as the user types on the keyboard,
  // we need to check if possible matches are still "alive" even if they're not
  // at a terminal leaf yet.
  static bool containsPartial(TrieNode *root, char key)
  {
    TrieNode *current = root;

    if (!current->children[(int)key])
    {
      DEBUG("Value of %c not found.", key);
      return false;
    }

    DEBUG("Value of %c found.", key);
    return true;
  }

  static bool contains(TrieNode *root, std::string key)
  {

    TrieNode *current = root;

    for (int i = 0; i < key.length(); i++)
    {
      int index = key[i];
      if (!current->children[index]) { return false; }

      current = current->children[index];
    }

    return current->terminal;
  }

  static TrieNode *getNode()
  {
    TrieNode *node     = new TrieNode();
    node->terminal     = false;
    node->abbreviation = nullptr;

    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
      node->children[i] = nullptr;
    }

    return node;
  }
};


class AppData
{
public:
  void init() { readSaveFile(); }

  Abbreviation *checkForCompletions()
  {
    DEBUG("Living Nodes Count: %d", livingNodes.size());
    for (int i = (int)livingNodes.size() - 1; i >= 0; i--)
    {
      if (livingNodes[i]->terminal)
      {
        Abbreviation *result = livingNodes[i]->abbreviation;
        std::swap(livingNodes[i], livingNodes[livingNodes.size() - 1]);
        livingNodes.pop_back();
        return result;
      }
    }
    return nullptr;
  }

  void advanceSearches(char c)
  {
    for (int i = (int)livingNodes.size() - 1; i >= 0; i--)
    {
      if (TrieNode::containsPartial(livingNodes[i], c))
      {
        // this one matches, continue advancing down the children
        livingNodes[i] = livingNodes[i]->children[(int)c];
      }
      else
      {
        std::swap(livingNodes[i], livingNodes[livingNodes.size() - 1]);
        livingNodes.pop_back();
      }
    }

    if (TrieNode::containsPartial(root, c)) { livingNodes.push_back(root->children[(int)c]); }
  }

  void deleteIndex(int index)
  {
    entries.erase(entries.begin() + index);
    saveToFile();
  }

  void readSaveFile()
  {
    std::ifstream in;
    in.open("./" SAVE_FILE_NAME);
    if (!in)
    {
      ERR("Failed to open the config in %s", SAVE_FILE_NAME);
      resetEntries();
      return;
    }

    std::string line;
    std::string label;
    std::string value;
    getline(in >> std::ws, line, DELIMITER);
    if (line != "ABBRV_SAVE_FILE_VERSION:" ABBRV_SAVE_FILE_VERSION) { updateSaveFileFormat(); }

    int savedEntriesCount = 0;
    getline(in >> std::ws, line, DELIMITER);
    GET_LABEL_AND_VALUE;
    savedEntriesCount = atoi(value.c_str());

    for (int i = 0; i < savedEntriesCount; i++)
    {
      DEBUG("LOOP");
      entries.push_back({});
      while (line != "}")
      {
        getline(in >> std::ws, line, DELIMITER);
        DEBUG("Reading line [%s]", line.c_str());
        GET_LABEL_AND_VALUE;
        DEBUG("Read label [%s] with value [%s]", label.c_str(), value.c_str());
        if (0) {}
        READ(entries[i].isHiddenField)
        READ(entries[i].isMultiline)
        READ_CHAR_ARRAY(entries[i].abbreviation)
        READ_CHAR_ARRAY(entries[i].expandsTo)
      }
      getline(in >> std::ws, line, DELIMITER);
    }

    resetEntries();
  }

  void backupConfigFile()
  {
    std::ifstream src(SAVE_FILE_NAME, std::ios::binary);
    std::ofstream dst(SAVE_FILE_NAME ".backup", std::ios::binary);

    dst << src.rdbuf();
  }

  void updateSaveFileFormat()
  {
    WARN("Updating to the new Save File Format");
    backupConfigFile();
    std::ifstream in;
    in.open("./" SAVE_FILE_NAME);
    if (!in)
    {
      ERR("Failed to open the config in %s", SAVE_FILE_NAME);
      resetEntries();
      return;
    }

    std::string abbreviation;
    std::string expandsTo;
    bool isMultiline   = false;
    bool isHiddenField = false;

    int abortCounter = 0;
    int abortLimit   = 10000;
    while (!in.eof())
    {
      in >> isMultiline;
      // in >> std::ws -- removes any whitespace from the line before processing
      std::getline(in >> std::ws, abbreviation, DELIMITER);
      DEBUG("Abbreviation: [%s]", abbreviation.c_str());
      std::getline(in, expandsTo, DELIMITER);
      if (abbreviation != "")
      {
        Abbreviation toAdd;
        toAdd.isMultiline   = isMultiline;
        toAdd.isHiddenField = isHiddenField;
        strcpy(toAdd.abbreviation, abbreviation.c_str());
        strcpy(toAdd.expandsTo, expandsTo.c_str());
        entries.push_back(toAdd);
      }

      abortCounter++;
      if (abortCounter > abortLimit)
      {
        ERR("Failure to parse and update Save File. Aborting update and loading clean slate.");
        entries = {};
        return;
      }
    }
    in.close();

    resetEntries();


    std::ofstream out;
    out.open("./" SAVE_FILE_NAME);
    if (!out)
    {
      ERR("Failed to open the config file %s", SAVE_FILE_NAME);
      return;
    }
    WRITE(ABBRV_SAVE_FILE_VERSION);
    WRITE(entries.size());

    for (int i = 0; i < entries.size(); i++)
    {
      START_WRITE("{");
      WRITE(entries[i].isHiddenField);
      WRITE(entries[i].isMultiline);
      WRITE(entries[i].abbreviation);
      WRITE(entries[i].expandsTo);
      END_WRITE("}");
    }

    DEBUG("Saved our entries.");
    resetEntries();
  }

  void resetEntries()
  {
    livingNodes = {};
    root        = nullptr;
    root        = TrieNode::getNode();
    for (int i = 0; i < entries.size(); i++)
    {
      TrieNode::insert(root, entries[i].abbreviation, &entries[i]);
    }
  }

  void saveToFile()
  {
    std::ofstream out;
    out.open("./" SAVE_FILE_NAME);
    if (!out)
    {
      ERR("Failed to open the config file %s", SAVE_FILE_NAME);
      return;
    }

    WRITE(ABBRV_SAVE_FILE_VERSION);
    WRITE(entries.size());

    for (int i = 0; i < entries.size(); i++)
    {
      START_WRITE("{");
      WRITE(entries[i].isHiddenField);
      WRITE(entries[i].isMultiline);
      WRITE(entries[i].abbreviation);
      WRITE(entries[i].expandsTo);
      END_WRITE("}");
    }

    DEBUG("Saved our entries.");
    resetEntries();
  }

  TrieNode *root;
  std::vector<Abbreviation> entries;
  std::vector<TrieNode *> livingNodes;
};

#endif
