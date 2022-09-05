#pragma once
#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP

#include <string>

class Serialization
{
public:
  inline static int indent = 0;
};

template <typename T>
T templated_parse(std::string value);

// we need to use an unusual delimiter in the save format so that we can safely
// accept _almost_ any input from the user in their abbreviations or expansions
#define DELIMITER '\x1f'

#define START_WRITE(brace)                                                                                             \
  out << std::string(Serialization::indent, '\t') << brace << DELIMITER;                                               \
  Serialization::indent++;

#define SERIALIZATION_INDENT std::string(Serialization::indent, '\t')

#define WRITE(x) out << std::string(Serialization::indent, '\t') << #x << ":" << x << DELIMITER;

#define WRITE_ARRAY(x)                                                                                                 \
  out << std::string(Serialization::indent, '\t') << #x << ":ARRAY" << std::endl;                                      \
  out << std::string(Serialization::indent, '\t') << x.size() << std::endl;                                            \
  out << std::string(Serialization::indent, '\t') << "[" << std::endl;                                                 \
  Serialization::indent++;                                                                                             \
  for (int i = 0; i < x.size(); i++)                                                                                   \
  {                                                                                                                    \
    if (i == 0) out << std::string(Serialization::indent, '\t');                                                       \
    out << x[i] << ' ';                                                                                                \
  }                                                                                                                    \
  out << std::endl;                                                                                                    \
  Serialization::indent--;                                                                                             \
  out << std::string(Serialization::indent, '\t') << "]" << std::endl;


#define END_WRITE(brace)                                                                                               \
  Serialization::indent--;                                                                                             \
  out << std::string(Serialization::indent, '\t') << brace << DELIMITER;

#define READ_CHAR_ARRAY(x)                                                                                             \
  else if (label == #x && value != "ARRAY") { strcpy(x, value.c_str()); }

#define READ(x)                                                                                                        \
  else if (label == #x && value != "ARRAY") { x = templated_parse<decltype(x)>(value.c_str()); }

#define READ_ARRAY(x)                                                                                                  \
  else if (label == #x && value == "ARRAY")                                                                            \
  {                                                                                                                    \
    std::string read;                                                                                                  \
    int arraySize = 0;                                                                                                 \
    getline(in >> std::ws, read); /* the first line after the array declare is the size */                             \
    arraySize = atoi(read.c_str());                                                                                    \
    x.resize(arraySize);                                                                                               \
    getline(in >> std::ws, read); /* throw away "[" */                                                                 \
    for (int i = 0; i < arraySize; i++)                                                                                \
    {                                                                                                                  \
      in >> x[i];                                                                                                      \
    }                                                                                                                  \
    getline(in >> std::ws, read); /* throw away "]" */                                                                 \
    label = "COMPLETED";          /* force our outer loop to continue */                                               \
  }

#define GET_LABEL_AND_VALUE                                                                                            \
  for (int i = 0; i < line.size(); i++)                                                                                \
  {                                                                                                                    \
    if (line[i] == ':')                                                                                                \
    {                                                                                                                  \
      label = line.substr(0, i);                                                                                       \
      value = line.substr(i + 1);                                                                                      \
      break;                                                                                                           \
    }                                                                                                                  \
  }


#endif
