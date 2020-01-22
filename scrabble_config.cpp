#include "scrabble_config.hpp"
#include "scrabble_exception.hpp"

#include <cassert>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
Board_Type Scrabble_Config::str_to_board(const std::string& board_str)
////////////////////////////////////////////////////////////////////////////////
{
  if (board_str == "scrabble") {
    return STANDARD_BOARD;
  }
  else if (board_str == "wwf") {
    return WWF_BOARD;
  }
  else if (board_str == "wwf-solo") {
    return WWF_SOLO_BOARD;
  }
  else {
    my_require(false, std::string("Unknown board string: ") + board_str);
  }
}

////////////////////////////////////////////////////////////////////////////////
Piece_Source_Type Scrabble_Config::str_to_tileset(const std::string& tileset_str)
////////////////////////////////////////////////////////////////////////////////
{
  if (tileset_str == "scrabble") {
    return STANDARD_SOURCE;
  }
  else if (tileset_str == "wwf") {
    return STANDARD_SOURCE; // TODO: Need WWF tileset
  }
  else {
    my_require(false, std::string("Unknown tileset string: ") + tileset_str);
  }
}

////////////////////////////////////////////////////////////////////////////////
Scrabble_Config::Scrabble_Config(unsigned num_players,
                                 const std::vector<Player_Type>& player_types,
                                 const std::vector<std::string>& player_names,
                                 unsigned num_player_pieces,
                                 Board_Type board_type,
                                 bool produce_output,
                                 bool color_output,
                                 bool clear_screen_before_output,
                                 const std::string& dictionary,
                                 Piece_Source_Type piece_source_type,
                                 unsigned max_num_log_msgs_to_displ,
                                 unsigned constrained_square_limit) :
  m_num_players(num_players),
  m_player_types(player_types),
  m_player_names(player_names),
  m_num_player_pieces(num_player_pieces),
  m_board_type(board_type),
  m_produce_output(produce_output),
  m_color_output(color_output),
  m_clear_screen_before_output(clear_screen_before_output),
  m_dictionary(dictionary),
  m_piece_source_type(piece_source_type),
  m_max_num_log_msgs_to_displ(max_num_log_msgs_to_displ),
  m_constrained_square_limit(constrained_square_limit)
////////////////////////////////////////////////////////////////////////////////
{
  my_static_assert(num_players == player_types.size(),
                   "num_players did not match player_types.size()");
}
