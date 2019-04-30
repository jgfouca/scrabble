#include "scrabble_config.h"
#include "scrabble_exception.h"

#include <cassert>

using namespace std;

const Scrabble_Config* Scrabble_Config::s_glob_instance = NULL;

////////////////////////////////////////////////////////////////////////////////
const Scrabble_Config& Scrabble_Config::instance()
////////////////////////////////////////////////////////////////////////////////
{
  //instance() was called before set_global_instance
  assert(s_glob_instance); //can't use my asserts, causes stack overflow

  return *s_glob_instance;
}

////////////////////////////////////////////////////////////////////////////////
Scrabble_Config::Scrabble_Config(unsigned num_players,
                                 const std::vector<Player_Type>& player_types,
                                 const std::vector<std::string>& player_names,
                                 Game_Mode game_mode,
                                 unsigned num_player_pieces,
                                 Board_Type board_type,
                                 bool produce_output,
                                 bool color_output,
                                 bool clear_screen_before_output,
                                 const std::string& dictionary,
                                 Piece_Source_Type piece_source_type, 
                                 unsigned max_num_log_msgs_to_displ, 
                                 unsigned constrained_square_limit,
                                 Assert_Fail_Action assert_fail_action) :
  m_num_players(num_players),
  m_player_types(player_types),
  m_player_names(player_names),
  m_game_mode(game_mode),
  m_num_player_pieces(num_player_pieces),
  m_board_type(board_type),
  m_produce_output(produce_output),
  m_color_output(color_output),
  m_clear_screen_before_output(clear_screen_before_output),
  m_dictionary(dictionary),
  m_piece_source_type(piece_source_type),
  m_max_num_log_msgs_to_displ(max_num_log_msgs_to_displ),
  m_constrained_square_limit(constrained_square_limit),
  m_assert_fail_action(assert_fail_action)
////////////////////////////////////////////////////////////////////////////////
{ 
  my_static_assert(num_players == player_types.size(), 
                   "num_players did not match player_types.size()");
}
  
////////////////////////////////////////////////////////////////////////////////
void Scrabble_Config::set_global_instance(const Scrabble_Config* new_inst)
////////////////////////////////////////////////////////////////////////////////
{
  my_static_assert(!s_glob_instance, 
                   "instance() was called before set_global_instance");
  s_glob_instance = new_inst;
}
