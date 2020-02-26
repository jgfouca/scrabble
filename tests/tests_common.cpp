#include "tests_common.hpp"

#include "catch.hpp"

#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>


bool files_equal(const std::string& path1, const std::string& path2)
{
  std::ifstream f1(path1), f2(path2);

  if (f1.fail() || f2.fail()) {
    return false; //file problem
  }

  if (f1.tellg() != f2.tellg()) {
    return false; //size mismatch
  }

  //seek back to beginning and use std::equal to compare contents
  f1.seekg(0, std::ifstream::beg);
  f2.seekg(0, std::ifstream::beg);
  return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                    std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(f2.rdbuf()));
}
