#ifndef TESTS_COMMON_HPP
#define TESTS_COMMON_HPP

/*
 * Unit test infrastructure for scrabble unit tests.
 *
 * Entities can friend UnitWrap to give unit tests
 * access to private members.
 *
 * All unit test impls should be within an inner struct.
 */

struct UnitWrap
{
  struct FullTests;
};

#endif
