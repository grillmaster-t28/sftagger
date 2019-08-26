/**
 * main.cpp
 *
 * sftagger rewrite
 */

#include <array>
#include <filesystem>
#include <iostream>

#include "tools/tools.h"

#define TAGS_LOCATION ".sftagger_storage"

int
main(int argc, char **argv)
{
  std::vector<std::string> args = tool::argumentsToVector(argc, argv);
  std::filesystem::path path(tool::getHomeDirectory());

  std::cout << path << std::endl;

  return 0;
}

