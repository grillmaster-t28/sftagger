#include "tools/argumentsToVector.h"

std::vector<std::string> tool::argumentsToVector(int argc, char **argv)
{
  std::vector<std::string> retArrayStrings;

  for (int i=0; i < argc; ++i)
  {
    retArrayStrings.emplace_back(argv[i]);
  }

  return retArrayStrings;
}

std::deque<std::string> tool::argumentsToDeque(int argc, char **argv)
{
  std::deque<std::string> retDequeStrings;

  for (int i=0; i < argc; ++i)
  {
    retDequeStrings.emplace_back(argv[i]);
  }

  return retDequeStrings;
}

