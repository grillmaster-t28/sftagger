#include "tools/tools.h"

std::vector<std::string> tool::argumentsToVector(int argc, char **argv)
{
  std::vector<std::string> retArrayStrings;

  for (int i=0; i < argc; ++i)
  {
    retArrayStrings.emplace_back(argv[i]);
  }

  return retArrayStrings;
}

