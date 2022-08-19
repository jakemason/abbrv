/**
 * abbrv Source Code
 * Copyright (C) 2022 Jake Mason
 *
 * @version 0.01
 * @author Jake Mason
 * @date 08-17-2022
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
#define ALPHABET_SIZE 128

#define SAVE_FILE_NAME "config.abbrv"

// we need to use an unusual delimiter in the save format so that we can safely
// accept _almost_ any input from the user in their abbreviations or expansions
#define DELIMITER '\x1f'

#include <fstream>
#include <string>
#include <vector>

#include "Debug.hpp"
#include "imgui.h"

struct Abbreviation
{
  char abbreviation[ABBREVIATION_MAX_SIZE];
  char expandsTo[EXPAND_MAX_SIZE];
  bool isMultiline;
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
    std::ifstream file;
    file.open("./" SAVE_FILE_NAME);
    if (!file)
    {
      ERR("Failed to open the config file %s", SAVE_FILE_NAME);
      resetEntries();
      return;
    }
    std::string abbreviation;
    std::string expandsTo;
    bool isMultiline;
    while (!file.eof())
    {
      file >> isMultiline;
      // file >> std::ws -- removes any whitespace from the line before processing
      std::getline(file >> std::ws, abbreviation, DELIMITER);
      DEBUG("Abbreviation: [%s]", abbreviation.c_str());
      std::getline(file, expandsTo, DELIMITER);
      if (abbreviation != "")
      {
        Abbreviation toAdd;
        toAdd.isMultiline = isMultiline;
        strcpy(toAdd.abbreviation, abbreviation.c_str());
        strcpy(toAdd.expandsTo, expandsTo.c_str());
        entries.push_back(toAdd);
      }
    }

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
    std::ofstream file;
    file.open("./" SAVE_FILE_NAME);
    if (!file)
    {
      ERR("Failed to open the config file %s", SAVE_FILE_NAME);
      return;
    }
    for (int i = 0; i < entries.size(); i++)
    {
      file << entries[i].isMultiline << " " << entries[i].abbreviation << DELIMITER << entries[i].expandsTo;
      if (i != entries.size() - 1) { file << DELIMITER; }
    }

    DEBUG("Saved our entries.");
    resetEntries();
  }

  TrieNode *root;
  std::vector<Abbreviation> entries;
  std::vector<TrieNode *> livingNodes;
};

#endif
