#ifndef scrabble_game_builder_h
#define scrabble_game_builder_h

#include "scrabble_game.h"
#include "ai_player.h"
#include "human_player.h"
#include "superuser_player.h"
#include "scrabble_board.h"
#include "piece_source.h"

#include <string>

class Scrabble_Game_Builder
{
 public:
  Scrabble_Game_Builder() : m_game(NULL) {}

  void build_scrabble_game()
  {
    m_game = new Scrabble_Game();
  }

  template <class BOARD_BUILDER_CLASS>
  void build_game_board()
  {
    my_static_assert(m_game, "m_game was null");
    m_game->add_board(new Scrabble_Board(BOARD_BUILDER_CLASS::instance()));
  }

  template <class PLAYER_CLASS>
  void build_player(const std::string& name)
  {
    my_static_assert(m_game, "m_game was null");
    m_game->add_player(new PLAYER_CLASS(name, m_game));
  }

  template <class LETTER_SOURCE_CLASS>
  void build_piece_source()
  {
    my_static_assert(m_game, "m_game was null");
    m_game->add_piece_source(new LETTER_SOURCE_CLASS());
  }

  Scrabble_Game* get_game() const 
  {
    return m_game; 
  }

 private:
  //forbidden methods
  Scrabble_Game_Builder(const Scrabble_Game_Builder&);
  Scrabble_Game_Builder& operator=(const Scrabble_Game_Builder&);

  //internal state
  Scrabble_Game* m_game; // do not delete
};

#endif
