#include "Serialization.hpp"

template <>
int templated_parse<int>(std::string value)
{
  return atoi(value.c_str());
}

template <>
float templated_parse<float>(std::string value)
{
  return atof(value.c_str());
}

template <>
bool templated_parse<bool>(std::string value)
{
  return (bool)atoi(value.c_str());
}

template <>
std::string templated_parse<std::string>(std::string value)
{
  return value;
}

template <>
char templated_parse<char>(std::string value)
{
  return value[0];
}
