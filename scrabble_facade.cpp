#include "scrabble_facade.hpp"
#include "human_player.hpp"
#include "ai_player.hpp"
#include "scrabble_board.hpp"
#include "scrabble_game_builder.hpp"
#include "standard_board_builder.hpp"
#include "wwf_solo_board_builder.hpp"
#include "wwf_board_builder.hpp"
#include "standard_piece_source.hpp"
#include "wwf_piece_source.hpp"
#include "scrabble_game.hpp"
#include "scrabble_config.hpp"
#include "scrabble_kokkos.hpp"

#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Facade::play(const std::vector<std::string>& players,
                           const std::vector<std::string>& ais,
                           const std::string& dictionary,
                           const Board_Type board,
                           const Piece_Source_Type tileset,
                           const int random_seed)
////////////////////////////////////////////////////////////////////////////////
{
  //allocate variables to hold the options
  std::vector<Player_Type> player_types;
  std::vector<std::string> player_names;
  std::vector<std::string> tests_to_run;
  unsigned num_player_pieces            = 7;
  bool produce_output                   = true;
  bool color_output                     = true;
  bool clear_screen_before_output       = true;
  unsigned max_num_log_msgs_to_displ    = 10;
  unsigned constrained_square_limit     = 3;

  srand(random_seed);

  for (const string& name : players) {
    player_names.push_back(name);
    player_types.push_back(HUMAN);
  }

  for (const string& name : ais) {
    player_names.push_back(name);
    player_types.push_back(AI);
  }

  Scrabble_Config config(player_types.size(), player_types, player_names,
                         num_player_pieces, board,
                         produce_output, color_output, clear_screen_before_output,
                         dictionary, tileset, max_num_log_msgs_to_displ,
                         constrained_square_limit);

  auto game = create_game(config);
  game->play();
}

////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scrabble_Game> Scrabble_Facade::get_test_game()
////////////////////////////////////////////////////////////////////////////////
{
  // Default config with one AI player
  std::vector<std::string> names = {"testbot"};
  std::vector<Player_Type> types = {AI};

  static Scrabble_Config default_config(types.size(), types, names);

  return create_game(default_config);
}

////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Scrabble_Game> Scrabble_Facade::create_game(const Scrabble_Config& config)
////////////////////////////////////////////////////////////////////////////////
{
  Scrabble_Game_Builder builder;

  //build up the game
  builder.build_scrabble_game(config);

  if (config.BOARD_TYPE() == STANDARD_BOARD) {
    builder.build_game_board<Standard_Board_Builder>();
  }
  else if (config.BOARD_TYPE() == WWF_BOARD) {
    builder.build_game_board<Wwf_Board_Builder>();
  }
  else if (config.BOARD_TYPE() == WWF_SOLO_BOARD) {
    builder.build_game_board<Wwf_Solo_Board_Builder>();
  }
  else {
    my_static_assert(false, "unknown board type");
  }

  std::vector<Player_Type> player_types = config.PLAYER_TYPES();
  std::vector<std::string> player_names = config.PLAYER_NAMES();
  my_static_assert(player_types.size() == player_names.size(),
                   "Player-type vector size did not match that of player-name vector");

  for (unsigned i = 0; i < player_types.size(); ++i) {
    if (player_types[i] == HUMAN) {
      builder.build_player<Human_Player>(player_names[i]);
    }
    else if (player_types[i] == AI) {
      builder.build_player<AI_Player>(player_names[i]);
    }
    else {
      my_static_assert(false, "unknown player type");
    }
  }

  if (config.PIECE_SOURCE_TYPE() == STANDARD_SOURCE) {
    builder.build_piece_source<Standard_Piece_Source>();
  }
  else if (config.PIECE_SOURCE_TYPE() == WWF_SOURCE) {
    builder.build_piece_source<Wwf_Piece_Source>();
  }
  else {
    my_static_assert(false, "unknown source type");
  }

  return std::shared_ptr<Scrabble_Game>(builder.get_game());
}

////////////////////////////////////////////////////////////////////////////////
void c_scrabble(const int num_players, const char** players,
                const int num_ais,     const char** ais,
                const char* dictionary,
                const char* board,
                const char* tileset,
                const int random_seed)
////////////////////////////////////////////////////////////////////////////////
{
  std::vector<std::string> cxx_players(num_players), cxx_ais(num_ais);
  std::string cxx_dict(dictionary);
  std::string cxx_board_str(board);
  std::string cxx_tileset_str(tileset);

  for (int i = 0; i < num_players; ++i) {
    cxx_players[i] = std::string(players[i]);
  }
  for (int i = 0; i < num_ais; ++i) {
    cxx_ais[i] = std::string(ais[i]);
  }

  Board_Type cxx_board = Scrabble_Config::str_to_board(cxx_board_str);
  Piece_Source_Type cxx_tileset = Scrabble_Config::str_to_tileset(cxx_tileset_str);

  const char* ignore = "";
  int ignore_i = 1;
  Kokkos::initialize(ignore_i, const_cast<char**>(&ignore));

  Scrabble_Facade::play(cxx_players, cxx_ais, dictionary, cxx_board, cxx_tileset, random_seed);

  Kokkos::finalize();
}
