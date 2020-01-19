#ifndef scrabble_exception_h
#define scrabble_exception_h

#include "scrabble_common.hpp"
#include "scrabble_config.hpp"

#include <unistd.h>
#include <cstdlib>
#include <string>

/**
 * This class is intended to be thrown whenever an assertion fails. Since this
 * program is relatively straight-forward, we expect correct behavior for
 * everything except when human-interaction is involved. Therefore, all errors
 * in the program should end up as assertion failures. That makes this class
 * quite important when it comes to debugging. To maximize flexibility, the
 * default behavior of this class is to print the process id of the running
 * program and then sleep forever. This will allow the user to attach gdb to
 * this process, which should allow for the best-possible postmortem analysis
 * of the problem.
 */

////////////////////////////////////////////////////////////////////////////////
class Assert_Exception : public std::exception
////////////////////////////////////////////////////////////////////////////////
{
 public:
  Assert_Exception(unsigned line,
                   const std::string& file,
                   const std::string& msg) :
    m_line(line), m_file(file), m_message(msg)
  {
    //the calling of a constructor implies an error has occured
    //what we do from here depends on the configuration
    if (Scrabble_Config::instance().ASSERT_FAIL_ACTION() == GDB_ATTACH) {
      std::cout << "Error: " << what() << std::endl;
      std::cout << "Process ID: " << getpid() << std::endl;
      std::cout << "Attach via command: 'gdb scrabble <pid>'" << std::endl;
      sleep(999999999);
    }
  }

  virtual ~Assert_Exception() throw() {}

  virtual const char* what() const throw()
  {
    m_full_msg = m_file + ":" + obj_to_str<unsigned>(m_line)
      + " " + m_message + "\n";
    return m_full_msg.c_str();
  }

  const std::string& message() const
  {
    return m_message;
  }

 private:
  unsigned            m_line;
  std::string         m_file;
  std::string         m_message;
  mutable std::string m_full_msg;
};

/**
 * When human input is in correct, the throwing of an exception is not
 * correlated with a program bug. For those purposes, the exception
 * class below should be used.
 */

////////////////////////////////////////////////////////////////////////////////
class Scrabble_Exception : public std::exception
////////////////////////////////////////////////////////////////////////////////
{
 public:
  Scrabble_Exception(const std::string& msg) : m_message(msg) {}

  virtual ~Scrabble_Exception() throw() {}

  virtual const char* what() const throw()
  {
    return m_message.c_str();
  }

  const std::string& message() const
  {
    return m_message;
  }
 private:
  std::string m_message;
};

/**
 * Below, we define our assertion macros. Note that asserts do nothing when we are
 * not in debug mode.
 *
 * Use the "my_static_assert" when you don't want to (or can't) output *this
 *
 * Use my_require to check user input
 */
#ifndef NDEBUG
#define my_assert(expr, message) if (!static_cast<bool>(expr)) { throw Assert_Exception(__LINE__, __FILE__, std::string("For object: ") + obj_to_str(*this) + "\n" + message); }
#define my_static_assert(expr, message) if (!static_cast<bool>(expr)) { throw Assert_Exception(__LINE__, __FILE__, message); }
#else
#define my_assert(expr, message)
#define my_static_assert(expr, message)
#endif
#define my_require(expr, message) if (!static_cast<bool>(expr)) { throw Scrabble_Exception(message); }

#endif
