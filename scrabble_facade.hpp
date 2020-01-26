#ifndef scrabble_facade_h
#define scrabble_facade_h

#include "scrabble_config.hpp"

#include <string>
#include <memory>

class Scrabble_Game;

/**
 * This class act's as the outside world's only interface to this library.
 * In order words, entities outside this package would only need to make
 * calls to the methods below in order to use the program.
 *
 * Other languages can call into this libray via the scrabble C function
 */

extern "C" {

void c_scrabble(const int num_players, const char** players,
                const int num_ais,     const char** ais,
                const char* dictionary,
                const char* board,
                const char* tiles,
                const int random_seed,
                const bool gui);

}

////////////////////////////////////////////////////////////////////////////////
struct Scrabble_Facade
////////////////////////////////////////////////////////////////////////////////
{
  /**
   * play - Runs the game according to the options encoded in the arguments.
   */
  static void play(const std::vector<std::string>& players,
                   const std::vector<std::string>& ais,
                   const std::string& dictionary,
                   const Board_Type board,
                   const Piece_Source_Type tileset,
                   const int random_seed,
                   const Output_Type output);

  static std::shared_ptr<Scrabble_Game> get_test_game();

 private:

  static std::shared_ptr<Scrabble_Game> create_game(const Scrabble_Config& config);
};

#endif
