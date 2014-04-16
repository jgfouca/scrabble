#ifndef scrabble_facade_h
#define scrabble_facade_h

#include "safe_string.h"

class Scrabble_Game;

/**
 * This class act's as the outside world's only interface to this program.
 * In order words, entities outside this package would only need to make
 * calls to the methods below in order to use the program. However, scrabble
 * is fairly complicated and there are many configurable items, so the client
 * will need to be familiar with a fairly complex option-set:
 *
 * (see HELP string in cc file)
 * 
 * Note: this class follows both the "singleton" and "facade" patterns.
 */

////////////////////////////////////////////////////////////////////////////////
class Scrabble_Facade
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * instance - Returns the global Scrabble_Facade instance.
   *            (This class is a Singleton)
   */
  static const Scrabble_Facade& instance();

  /**
   * play - Runs the game according to the options encoded in the arguments.
   */
  void play(int argc, char** argv) const;

  /**
   * HELP - A string describing how to use this program
   */
  static const Safe_String HELP;
  
 private:

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// FORBIDDEN METHODS /////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  Scrabble_Facade(const Scrabble_Facade&);
  Scrabble_Facade& operator=(const Scrabble_Facade&);

  //////////////////////////////////////////////////////////////////////////////
  ////////////////////////// INTERNAL METHODS //////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * Constructor - Private because this is a singleton class.
   */ 
  Scrabble_Facade() {}
  
  /**
   * create_game - Creates and returns a Scrabble_Game option based on the 
   *               configuration provided in the options.
   */
  Scrabble_Game* create_game() const;
};

#endif
