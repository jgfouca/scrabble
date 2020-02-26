#ifndef TESTS_COMMON_HPP
#define TESTS_COMMON_HPP

#include <string>

/*
 * Unit test infrastructure for scrabble unit tests.
 *
 * Entities can friend UnitWrap to give unit tests
 * access to private members.
 *
 * All unit test impls should be within an inner struct.
 */

bool files_equal(const std::string& path1, const std::string& path2);

struct UnitWrap
{
  struct FullTests;
};

#endif
