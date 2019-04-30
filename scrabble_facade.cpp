#include "scrabble_facade.hpp"
#include "human_player.hpp"
#include "ai_player.hpp"
#include "scrabble_board.hpp"
#include "scrabble_game_builder.hpp"
#include "standard_board_builder.hpp"
#include "wwf_board_builder.hpp"
#include "standard_piece_source.hpp"
#include "scrabble_game.hpp"
#include "scrabble_tester.hpp"
#include "scrabble_config.hpp"

#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace std;

const std::string Scrabble_Facade::HELP =
  "scrabble --mode=(test|normal|command) \n"
  "   First step: you must pick your mode, options are test,normal, and command. \n"
  "   normal is default. \n"
  "\n"
  "scrabble --mode=test [--all ] [--test-one] [--test-two] [--test-three] [--test-four] [<config-options>] \n"
  "   If you pick test-mode, use the above options to select which tests will run. \n"
  "   --all is default. \n"
  "\n"
  "scrabble --mode=normal --add-player=<name>,(HUMAN|AI)  [<config-options>] \n"
  "   In normal mode, you and as many players as you'd like using --add-player. \n"
  "   This would be how you would run an ordinary scrabble game. \n"
  "   HUMAN     -> A normal, human-controlled player. \n"
  "   AI        -> A computer-controlled player. \n"
  "\n"
  "scrabble --mode=command [<config-options>] \n"
  "   Command mode is fairly simple. You control the state of the board and the \n"
  "   contents of a player's tray, then ask for the best move. This may be useful \n"
  "   for real-life games or other virtual scrabble games. \n"
  "\n"
  "<config-options> \n"
  "   These options can be used for any of the modes, however the vast majority \n"
  "   of the time, you won't need to change these: \n"
  "   --num-player-pieces=<num> \n"
  "       Change the number of pieces that go in players' trays. Default is 7. \n"
  "   --board-type=(STANDARD|WWF) \n"
  "       Change the type of board to be used. Default is STANDARD. \n"
  "   --produce-output=(yes|no) \n"
  "       Turn on all game output. Default is yes. \n"
  "   --color-output=(yes|no) \n"
  "       Turn on coloring of output to improve appearance. Default is yes. \n"
  "   --clear-screen-before-output=(yes|no) \n"
  "       Turn on screen-clearing before producing output. Default is yes. \n"
  "   --dictionary=<dict-file> \n"
  "       Choose the dictionary file that will be used. Default is TWL06.txt. \n"
  "   --piece-source-type=(STANDARD|<no other options at this time>) \n"
  "       Choose the type of piece source (AKA 'bag'). Default is STANDARD. \n"
  "   --max-log-msgs=<max log msgs> \n"
  "       Choose the maximum number of log msgs that will be displayed during \n"
  "       regular play. This can help keep your screen from filling up. Default is 10. \n"
  "   --constrained-square-limit=<const sqr limit> \n"
  "       Choose the breadth of the AI's searches. A high number may allow the AI to \n"
  "       discover better plays, but it will slow down the AIs searches. Default is 3. \n"
  "   --assert-fail-action=(GDB_ATTACH|EXCEPTION) \n"
  "       Choose the action that will be taken when asserts fail. Default is GDB_ATTACH. \n"
  "   --srand=<random seed> \n"
  "       Choose the random seed. This can allow you to repeat test results etc. \n"
  "       Default is -1, which means the 'time' function will be used to produce \n"
  "       a pseudo-random seed.\n"
  "\n"
  "\n"
  "EXAMPLES: \n"
  "  To watch the AI repeatedly play with itself :) \n"
  "  scrabble --mode=test --test-four \n"
  "\n"
  "  To play a normal game against the AI: \n"
  "  scrabble --mode=normal --add-player=<your-name>,HUMAN --add-player=ai_player,AI \n"
  "\n"
  "  To use this program to your advantage in a real scrabble game: \n"
  "  scrabble --mode=command \n";

////////////////////////////////////////////////////////////////////////////////
const Scrabble_Facade& Scrabble_Facade::instance()
////////////////////////////////////////////////////////////////////////////////
{
  static Scrabble_Facade sf;
  return sf;
}

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Facade::play(int argc, char** argv) const
////////////////////////////////////////////////////////////////////////////////
{
  // Set up parsing of argv using getopt..

  //allocate variables to hold the options
  std::vector<Player_Type> player_types;
  std::vector<std::string> player_names;
  std::vector<std::string> tests_to_run;
  Game_Mode game_mode                   = INVALID;
  unsigned num_player_pieces            = 7;
  Board_Type board_type                 = STANDARD_BOARD;
  bool produce_output                   = true;
  bool color_output                     = true;
  bool clear_screen_before_output       = true;
  std::string dictionary                = "TWL06.txt";
  Piece_Source_Type piece_source_type   = STANDARD_SOURCE;
  unsigned max_num_log_msgs_to_displ    = 10;
  unsigned constrained_square_limit     = 3;
  Assert_Fail_Action assert_fail_action = GDB_ATTACH;
  unsigned rand_seed                    = time(0);

  //do the options parsing:
  if (argc == 1) {
    //if no args given, provide help
    cout << HELP << endl;
    return;
  }

  for (int opt_itr = 1; opt_itr < argc; ++opt_itr) {
    std::string opt = "", arg = "", full_arg = argv[opt_itr];

    //check for presence of a an arg that looks like a help request
    if (full_arg == "-h" || full_arg == "-help" || full_arg == "--help") {
      cout << HELP << endl;
      return;
    }

    //if necessary, split --opt=arg into opt, arg
    if (full_arg.find('=') != std::string::npos) {
      opt = full_arg.substr(0, full_arg.find('='));
      arg = full_arg.substr(full_arg.find('=') + 1);
    }
    else {
      opt = full_arg;
    }

    //process option
    if (opt == "--mode") {
      if (arg == "test") {
        game_mode = TEST;
      }
      else if (arg == "normal") {
        game_mode = NORMAL;
      }
      else if (arg == "command") {
        game_mode = COMMAND;
      }
      else {
        cerr << "Unknown game mode: " << arg << endl;
        return;
      }
    }
    else {
      if (game_mode == INVALID) {
        cerr << "Please choose mode option first." << endl;
        return;
      }

      if (opt == "--test-one"   || opt == "--test-two" ||
          opt == "--test-three" || opt == "--test-four" || opt == "--all") {
        if (game_mode != TEST) {
          cerr << "Can only use " << opt << " in test mode." << endl;
          return;
        }
        tests_to_run.push_back(opt);
      }
      else if (opt == "--add-player") {
        if (game_mode != NORMAL) {
          cerr << "Can only use " << opt << " in normal mode." << endl;
          return;
        }
        std::string player_name, type_string;
        Player_Type player_type;
        if (arg.find(',') != std::string::npos) {
          player_name = arg.substr(0, arg.find(','));
          type_string = arg.substr(arg.find(',') + 1);
        }
        else {
          cerr << "Player specification requires a comma to separate name from type" << endl;
          return;
        }

        if (type_string == "HUMAN") {
          player_type = HUMAN;
        }
        else if (type_string == "AI") {
          player_type = AI;
        }
        else {
          cerr << "Unknown player type: " << type_string << endl;
          return;
        }
        player_types.push_back(player_type);
        player_names.push_back(player_name);
      }
      else if (opt == "--num-player-pieces") {
        num_player_pieces = atoi(arg.c_str());
      }
      else if (opt == "--board-type") {
        if (arg == "STANDARD") {
          board_type = STANDARD_BOARD;
        }
        else if (arg == "WWF") {
          board_type = WWF_BOARD;
        }
        else {
          cerr << "Unknown board type: " << arg << endl;
          return;
        }
      }
      else if (opt == "--produce-output") {
        if (arg == "yes") {
          produce_output = true;
        }
        else if (arg == "no") {
          produce_output = false;
        }
        else {
          cerr << "Please provide 'yes' or 'no' to option " << opt << endl;
          return;
        }
      }
      else if (opt == "--color-output") {
        if (arg == "yes") {
          color_output = true;
        }
        else if (arg == "no") {
          color_output = false;
        }
        else {
          cerr << "Please provide 'yes' or 'no' to option " << opt << endl;
          return;
        }
      }
      else if (opt == "--clear-screen-before-output") {
        if (arg == "yes") {
          clear_screen_before_output = true;
        }
        else if (arg == "no") {
          clear_screen_before_output = false;
        }
        else {
          cerr << "Please provide 'yes' or 'no' to option " << opt << endl;
          return;
        }
      }
      else if (opt == "--dictionary") {
        dictionary = arg;
      }
      else if (opt == "--piece-source-type") {
        if (arg == "") {
          piece_source_type = STANDARD_SOURCE;
        }
        else {
          cerr << "Unknown piece-source type: " << arg << endl;
          return;
        }
      }
      else if (opt == "--max-log-msgs") {
        max_num_log_msgs_to_displ = atoi(arg.c_str());
      }
      else if (opt == "--constrained-square-limit") {
        constrained_square_limit = atoi(arg.c_str());
      }
      else if (opt == "--assert-fail-action") {
        if (arg == "GDB_ATTACH") {
          assert_fail_action = GDB_ATTACH;
        }
        else if (arg == "EXCEPTION") {
          assert_fail_action = EXCEPTION;
        }
        else {
          cerr << "Unknown assert-fail action: " << arg << endl;
          return;
        }
      }
      else if (opt == "--srand") {
        rand_seed = atoi(arg.c_str());
      }
      else {
        cerr << "Unknown option: " << opt << endl;
        return;
      }
    }
  }

  //all tests require a single AI player
  if (game_mode == TEST) {
    if (tests_to_run.empty()) {
      tests_to_run.push_back("--all");
    }
    player_types.push_back(AI);
    player_names.push_back("AI-player");
  }

  //command-mode uses a single SUPERUSER player
  if (game_mode == COMMAND) {
    player_types.push_back(SUPERUSER);
    player_names.push_back("super-user");
  }

  srand(rand_seed);

  static Scrabble_Config global_config(player_types.size(), player_types, player_names,
                                       game_mode, num_player_pieces, board_type,
                                       produce_output, color_output, clear_screen_before_output,
                                       dictionary, piece_source_type, max_num_log_msgs_to_displ,
                                       constrained_square_limit, assert_fail_action);

  Scrabble_Config::set_global_instance(&global_config);

  Scrabble_Game* game;

  if (game_mode == NORMAL || game_mode == COMMAND) {
    game = create_game();
    game->play();
    delete game;
  }
  else if (game_mode == TEST) {
    for (unsigned i = 0; i < tests_to_run.size(); ++i) {
      if (tests_to_run[i] == "--test-one" || tests_to_run[i] == "--all") {
        game = create_game();
        Scrabble_Tester::test_one(game, false);
        delete game;
      }
      if (tests_to_run[i] == "--test-two" || tests_to_run[i] == "--all") {
        game = create_game();
        Scrabble_Tester::test_two(game);
        delete game;
      }
      if (tests_to_run[i] == "--test-three" || tests_to_run[i] == "--all") {
        for (unsigned depth = 1; depth <= 10; ++depth) {
          for (unsigned itr = 0; itr < 10; ++itr) {
            game = create_game();
            Scrabble_Tester::test_three(game, itr, depth, false);
            delete game;
          }
        }
      }
      if (tests_to_run[i] == "--test-four" || tests_to_run[i] == "--all") {
        for (unsigned itr = 0; itr < 1000; ++itr) {
          rand_seed = (itr+1)*100;
          //rand_seed = 500;
          srand(rand_seed);
          game = create_game();
          cout << "Playing game with seed: " << rand_seed << endl;
          Scrabble_Tester::test_four(game, false);
          delete game;
        }
      }
    }
  }
  else {
    my_static_assert(false, "Invalid game mode.");
  }
}

////////////////////////////////////////////////////////////////////////////////
Scrabble_Game* Scrabble_Facade::create_game() const
////////////////////////////////////////////////////////////////////////////////
{
  Scrabble_Game_Builder builder;

  //build up the game
  builder.build_scrabble_game();

  if (Scrabble_Config::instance().BOARD_TYPE() == STANDARD_BOARD) {
    builder.build_game_board<Standard_Board_Builder>();
  }
  else if (Scrabble_Config::instance().BOARD_TYPE() == WWF_BOARD) {
    builder.build_game_board<Wwf_Board_Builder>();
  }
  else {
    my_static_assert(false, "unknown board type");
  }

  std::vector<Player_Type> player_types = Scrabble_Config::instance().PLAYER_TYPES();
  std::vector<std::string> player_names = Scrabble_Config::instance().PLAYER_NAMES();
  my_static_assert(player_types.size() == player_names.size(),
                   "Player-type vector size did not match that of player-name vector");

  for (unsigned i = 0; i < player_types.size(); ++i) {
    if (player_types[i] == HUMAN) {
      builder.build_player<Human_Player>(player_names[i]);
    }
    else if (player_types[i] == AI) {
      builder.build_player<AI_Player>(player_names[i]);
    }
    else if (player_types[i] == SUPERUSER) {
      builder.build_player<Superuser_Player>(player_names[i]);
    }
    else {
      my_static_assert(false, "unknown player type");
    }
  }

  if (Scrabble_Config::instance().PIECE_SOURCE_TYPE() == STANDARD_SOURCE) {
    builder.build_piece_source<Standard_Piece_Source>();
  }
  else {
    my_static_assert(false, "unknown source type");
  }

  return builder.get_game();
}
