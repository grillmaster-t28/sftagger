#include <string>
#include <vector>

namespace tool
{
  std::string               getHomeDirectory();
  std::vector<std::string>  argumentsToVector(int argc, char **argv);
}

