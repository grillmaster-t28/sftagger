/**
 * main.cpp
 *
 * sftagger C++ edition:
 *    https://github.com/marttcw/sftagger
 * sqlite_modern_cpp documentation/README:
 *    https://github.com/SqliteModernCpp/sqlite_modern_cpp/blob/master/README.md
 */

#include <string>
#include <array>
#include <iostream>
#include <map>
#include <functional>
#include <deque>

#include "tools/argumentsToVector.h"
#include "tools/homeDirectory.h"
#include "core/create.h"

int main(int argc, char **argv)
{
  if (argc == 1)
  {
    std::cerr << "ERROR: Too few parameters.\n";
    return 0;
  }

  std::deque<std::string> args = tool::argumentsToDeque(argc, argv);
  std::string targetFile = tool::getHomeDirectory()+"/.sftagger_tags.db";

  /// String mapped to function (args, targetFile)
  std::map<std::string,
    std::function<void(std::deque<std::string> &, const std::string &)>
      > funcStrMap;

  /// Lists of functions that'll be mapped to their relative parameter
  funcStrMap["create"] = core::create::createFile;
  funcStrMap["createTag"] = core::create::createTag;

  /// C++20: replace try-catch exception with contains statement
  try
  {
    funcStrMap[args[1]](args, targetFile);
  }
  catch (std::bad_function_call &e)
  {
    std::cerr << "ERROR: Parameter not found\n"; 
  }

  return 0;
}

