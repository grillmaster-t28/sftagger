#ifndef CORE__CREATE_HPP
#define CORE__CREATE_HPP

#include <deque>
#include <string>

namespace core
{
  namespace create
  {
    void createFile(std::deque<std::string> &args, const std::string &targetFile);
    void createTag(std::deque<std::string> &args, const std::string &targetFile);
  }
}

#endif
