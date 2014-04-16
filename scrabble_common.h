#ifndef scrabble_common_h
#define scrabble_common_h

#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

/**
 * File contains some utility methods that are not associated with any
 * particular class. 
 */

//method below should word for any object that defines << and all the primitives
template <class T>
std::string obj_to_str(const T& val)
{
  std::ostringstream ss;
  ss << val;
  return std::string(ss.str());
}

template <class A, class B>
std::ostream& operator<<(std::ostream& out, const std::pair<A, B>& p)
{
  out << "(" << p.first << ", " << p.second << ")";
  return out;
}

template <class T>
bool vector_contains(const std::vector<const T*>& vect, const T& item)
{
  for (unsigned i = 0; i < vect.size(); ++i) {
    if (*vect[i] == item) {
      return true;
    }
  }
  return false;
}

#endif
