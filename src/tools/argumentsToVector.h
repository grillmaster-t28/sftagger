#ifndef TOOLS__ARGUMENTSTOVECTOR_H
#define TOOLS__ARGUMENTSTOVECTOR_H

#include <string>
#include <vector>
#include <deque>

namespace tool
{
  std::vector<std::string> argumentsToVector(int argc, char **argv);
  std::deque<std::string> argumentsToDeque(int argc, char **argv);
}

#endif
