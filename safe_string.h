#ifndef safe_string_h
#define safe_string_h

#include <string>

/**
 * The point of this class is to provide bounds-checking for our strings when
 * we are in debug mode. In optimized mode, this class does not exist and its
 * name is simply an alias for std::string.
 */

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////////////////
class Safe_String : public std::string
////////////////////////////////////////////////////////////////////////////////
{
 public:
  Safe_String() : std::string() {}

  Safe_String(const char* str) : std::string(str) {}

  Safe_String(const std::string& str) : std::string(str) {}

  char& operator[](size_t index)
  {
    return std::string::at(index);
  }

  const char& operator[](size_t index) const
  {
    return std::string::at(index);
  }
};
#else
typedef std::string Safe_String;
#endif
#endif
