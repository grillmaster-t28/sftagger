#include "core/create.h"

#include <iostream>
#include <filesystem>
#include <sqlite_modern_cpp.h>

#include "tools/homeDirectory.h"

void core::create::createFile(std::deque<std::string> &args, const std::string &targetFile)
{
  // If the user wants to override the tags file
  if ((args.size() > 2 && args[2] == "override"))
  {
    std::string userChoice;

    std::cout << "\033[1;31mWARNING\033[0m: This will remove all of your tags database, do you wish to continue? Type in \"yes\" if you want to: ";
    std::cin >> userChoice;

    if (userChoice == "yes")
    {
      std::filesystem::remove(targetFile);
      std::cout << "Database will be overridden\n";
    }
    else
    {
      std::cout << "Database not overridden\n";
      return;
    }
  }

  // Prevent file override if it exists
  if (std::filesystem::exists(targetFile))
  {
    std::cerr << "\033[1;35mERROR\033[0m: File already exists, don't need to create\n";
    return;
  }

  try
  {
    sqlite::database db(targetFile);

    db <<
      "create table if not exists files ("
      "  _id integer primary key autoincrement not null"
      ", path text"
      ");";

    db <<
      "create table if not exists types ("
      "  name text primary key not null"
      ");";

    db <<
      "create table if not exists tags ("
      "  name text primary key not null"
      ");";

    db << "insert into types(name) values('general')";

    std::cout << "Database created\n";
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }
}

void core::create::createTag(std::deque<std::string> &args, const std::string &targetFile)
{
  if (args.size() <= 2)
  {
    std::cerr << "ERROR: No tags given.\n";
    return;
  }

  try
  {
    sqlite::database db(targetFile);

    // Remove first 2 unwanted arguments
    args.pop_front();
    args.pop_front();

    for (const auto &arg : args)
    {
      db <<
        "insert into tags(name) values(?);"
        << arg;
    }

    std::cout << args.size() << " tags inserted to the database.\n";
  }
  // SQL Primary key constraint exception
  catch (sqlite::errors::constraint_primarykey &e)
  {
    std::cerr << "\033[1;31mWARNING (SQL)\033[0m: Tag already exists\n";
  }
  // SQL Generic exception
  catch (sqlite::sqlite_exception &e)
  {
    std::cerr << "\033[1;31mWARNING (SQL)\033[0m: " << e.get_code() << ": " << e.what() << '\n';
  }
  // Generic exception
  catch (std::exception &e)
  {
    std::cerr << "\033[1;31mWARNING (Generic)\033[0m: " << e.what() << '\n';
  }
}
