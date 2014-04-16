#ifndef safe_vector_h
#define safe_vector_h

#include <vector>

/**
 * The point of this class is to provide bounds-checking on our vectors when
 * we are in debug mode. In optimized mode, the name of this class does not
 * exist and its name simply servers as an alias for std::vector.
 */

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////////////////
template <class T>
class Safe_Vector : public std::vector<T>
////////////////////////////////////////////////////////////////////////////////
{
 public:
  typedef typename std::vector<T>::const_reference const_reference;
  typedef typename std::vector<T>::reference reference;
  

  Safe_Vector() : std::vector<T>() {}

  Safe_Vector(const std::vector<T>& vect) : std::vector<T>(vect) {}
  
  Safe_Vector(size_t num, const T& val) : std::vector<T>(num, val) {}

  Safe_Vector(size_t num) : std::vector<T>(num) {}
  
  reference operator[](size_t index)
  {
    return std::vector<T>::at(index);
  }
  
  const_reference operator[](size_t index) const
  {
    return std::vector<T>::at(index);
  }
};
#else
#define Safe_Vector std::vector
#endif
#endif
