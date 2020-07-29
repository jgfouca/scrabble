#include "scrabble_game.hpp"
#include "player.hpp"
#include "scrabble_piece.hpp"
#include "indv_play.hpp"
#include "constraint.hpp"
#include "ai_player.hpp"
#include "scrabble_exception.hpp"
#include "scrabble_common.hpp"
#include "scrabble_facade.hpp"

#include "tests_common.hpp"

#include "catch.hpp"

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>
#include <sstream>

struct UnitWrap::FullTests
{

  /////////////////////////////////////////////////////////////////////////////
  static void test_one()
  /////////////////////////////////////////////////////////////////////////////
  {
    auto game = Scrabble_Facade::get_test_game();

    //Assemble and submit some plays

    //get player
    my_static_assert(game->m_players.size() > 0, "Game had no players");
    game->initialize();
    AI_Player* test_player = dynamic_cast<AI_Player*>(game->m_players[0]);
    my_static_assert(test_player, "First player needs to be AI for this test");

    Indv_Play play_obj;

    // We hard-code a number of plays to test the engine:

    std::string words[] = {"E-HIBIT", "OADS", "H_ATS", "N", "T", "URTS", "I", "B-CKSTA",
                           "ORED", "SC_R", "SIC_EN", "_P_TS", "SPINS", "VI___S", "AA", "_AC_T",
                           "BAD", "__TERB", "OM", "NESS", "_RIGI_AL", "O_S", "B_T", "K",
                           "A", "TO"};
    char wildcard_choices[] = {'X', '_', '_', '_', '_', '_', '_', 'A',
                               '_', '_', '_', '_', '_', '_', '_', '_',
                               '_', '_', '_', '_', '_', '_', '_', '_',
                               '_', '_'};
    std::string searches[] = {"*", "T*", "*E*", "#O", "A*", "H*", "S_T", "*B",
                              "B*", "*A#", "*K*", "S_I*", "#___#", "*SIT*", "##", "T__I*",
                              "", "", "BORED*", "TACIT*", "O____N*", "", "", "AS*",
                              "#T", "##"};
    unsigned max_sizes[] = {15, 8, 10, 2, 8,  8,  3, 8,
                            8,  8, 15, 7, 5,  12, 2, 8,
                            0,  0, 8,  9, 10, 0,  0, 8,
                            2,  2};
    unsigned min_sizes[] = {1, 2, 2, 2, 2, 2, 3, 2,
                            2, 2, 2, 3, 5, 4, 2, 4,
                            0, 0, 6, 6, 6, 0, 0, 3,
                            2, 2};
    std::string search_cont[] = {"EXHIBIT", "TOADS", "HEATS", "NO", "AT", "HURTS", "SIT", "BACKSTAB",
                                 "BORED", "SCAR", "SICKEN", "SPITS", "SPINS", "VISITS", "AA", "TACIT",
                                 "", "", "BOREDOM", "TACITNESS", "ORIGINAL", "", "", "ASK",
                                 "AT", "TO"};
    unsigned starting_row[] = {7, 8, 6, 8, 9, 8, 10, 0,
                               0, 1, 3, 1, 3, 10, 10, 5,
                               8, 11, 0, 5, 0, 1, 11, 11,
                               9, 2};
    unsigned starting_col[] = {2, 8, 2, 7, 9, 4, 3, 6,
                               7, 4, 3, 4, 9, 0, 7, 6,
                               11, 7, 11, 11, 11, 10, 6, 9,
                               1, 12};
    bool is_horiz[] = {true, false, false, false, false, false, false, false,
                       true, true, true, false, false, true, false, true,
                       false, true, true, true, false, true, true, true,
                       false, false};
    std::string err_msgs[] = {"", "", "", "", "", "", "", "",
                              "", "", "", "", "", "", "", "",
                              "MOVE B(8,11) A(9,11) D(10,11)  REJECTED: play did not connect with any words",
                              "MOVE T(11,9) E(11,10) R(11,11) B(11,12)  REJECTED: \"ASTERB\" is not a valid word", "", "", "",
                              "MOVE O(1,10) S(2,12)  REJECTED: all played letters must be linear",
                              "MOVE B(11,6) T(11,8)  REJECTED: tried to place on top of another piece", "",
                              "", ""};
    unsigned expected_scores[] = {61, 7, 13, 4, 4, 16, 3, 70,
                                  24, 14, 26, 14, 34, 9, 9, 7,
                                  0, 0, 13, 13, 20, 0, 0, 7,
                                  8, 11};

    std::vector<std::vector<std::pair<std::string, unsigned> > > compat_vectors(sizeof(starting_row)/sizeof(unsigned));
    std::vector<std::vector<unsigned> > max_length_vectors(sizeof(starting_row)/sizeof(unsigned));

    //Manually insert the compatibilities
    compat_vectors[3].push_back(std::pair<std::string, unsigned>("I#", 1));

    compat_vectors[9].push_back(std::pair<std::string, unsigned>("O#", 1));

    compat_vectors[12].push_back(std::pair<std::string, unsigned>("SICKEN#", 6));
    compat_vectors[12].push_back(std::pair<std::string, unsigned>("EXHIBIT#", 7));

    compat_vectors[14].push_back(std::pair<std::string, unsigned>("#D", 0));
    compat_vectors[14].push_back(std::pair<std::string, unsigned>("#S", 0));

    compat_vectors[24].push_back(std::pair<std::string, unsigned>("#I", 0));

    compat_vectors[25].push_back(std::pair<std::string, unsigned>("I#", 1));
    compat_vectors[25].push_back(std::pair<std::string, unsigned>("G#", 1));

    //Manually insert max lengths (hack for now)
    for (unsigned i = 0; i < sizeof(starting_row)/sizeof(unsigned); i++) {
      for (unsigned j = 0; j < searches[i].size(); j++) {
        if (searches[i][j] == '*') {
          max_length_vectors[i].push_back(15);
        }
      }
    }

    //MAIN LOOP
    for (unsigned i = 0; i < sizeof(starting_row)/sizeof(unsigned); i++) {

      //overwrite the player's letters so that they can make preset play
      unsigned piece_itr = 0;
      for (unsigned p = 0; p < words[i].size(); p++) {
        if (words[i][p] != '_') {
          Scrabble_Piece* piece = const_cast<Scrabble_Piece*>(test_player->m_pieces[piece_itr++]);
          piece->force_letter_change(words[i][p]);
        }
      }

      //overwrite the rest of the player's letters with 'E', this should clear any
      //wildcard pieces they might have had
      while (piece_itr < 7) {
        Scrabble_Piece* piece = const_cast<Scrabble_Piece*>(test_player->m_pieces[piece_itr++]);
        piece->force_letter_change('E');
      }
      test_player->remap();

      //create the play-object needed to create the expected word
      unsigned row = starting_row[i], col = starting_col[i];
      piece_itr = 0;
      for (unsigned p = 0; p < words[i].size(); p++) {
        if (words[i][p] != '_') {
          Scrabble_Piece* piece = const_cast<Scrabble_Piece*>(test_player->m_pieces[piece_itr++]);

          if (words[i][p] == '-') {
            piece->set_wildcard_value(wildcard_choices[i]);
          }
          //std::cout << *test_player << std::endl;
          play_obj.place_piece(row, col, (words[i][p] == '-') ? test_player->get_piece(wildcard_choices[i]) :
                               test_player->get_piece(words[i][p]));
        }
        row += (is_horiz[i] ? 0 : 1);
        col += (is_horiz[i] ? 1 : 0);
        if (words[i] == "O_S" && p == 1) {
          //special case, attempt diagonal placement
          row++;
        }
      }

      test_player->remap();

      //send the play to the game engine
      std::string real_err_msg;
      if ( (real_err_msg = game->evaluate_play(play_obj)) != err_msgs[i]) {
        my_static_assert(false, std::string("Error message: '") + real_err_msg + "'" +
                         " did not match expected error message: '" + err_msgs[i] + "'")
          }
      if (err_msgs[i] == "") {
        std::cout << "Trying: " << searches[i] << std::endl;

        //check that my search alg would have found this word
        std::vector<std::set<char> > req_sets;
        Constraint::convert_compat_req_to_set(game->get_valid_words(), compat_vectors[i], req_sets);
        Constraint constraint(searches[i], req_sets, max_length_vectors[i]);
        test_player->find_all_satisfying_strings(constraint, min_sizes[i], max_sizes[i]);
        my_static_assert(vector_contains(test_player->m_recent_result, search_cont[i]),
                         std::string("contrained search failed to find valid word: ") + search_cont[i]);

        //have game process the play
        game->process_legit_play(play_obj, test_player);
        my_static_assert(game->m_potential_score == expected_scores[i],
                         obj_to_str(game->m_potential_score) + " did not match expected score: " +
                         obj_to_str(expected_scores[i]));

        std::cout << "Created words: " << game->m_potential_words << std::endl;
        std::cout << *game << std::endl;
      }

      play_obj.clear();
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  static void test_two()
  /////////////////////////////////////////////////////////////////////////////
  {
    auto game = Scrabble_Facade::get_test_game();

    my_static_assert(game->m_players.size() > 0, "Game had no players");
    game->initialize();
    AI_Player* test_player = dynamic_cast<AI_Player*>(game->m_players[0]);
    my_static_assert(test_player, "First player needs to be AI for this test");

    /*
      static char text[63]; //should be enough to hold a line
      FILE* cmd_pipe;

      std::string letters[] = {"TIBIHXE", "SODKWEW", "QPEOR-E", "LMSCEIU", "MNBOIUP",
      "-AOEPLM", "PLASDUI", "IMNBXCS", "LAKSJDE", "PLQ-IDS"};

      std::string grep_searches[] = {};

      std::string my_searches[] = {};

      for (unsigned i = 0; i < (sizeof(letters)/sizeof(std::string)); ++i) {
      for (unsigned j = 0; j < letters[i].size(); ++j) {
      Scrabble_Piece* piece = const_cast<Scrabble_Piece*>(test_player->m_pieces[j]);
      piece->force_letter_change(letters[i][j]);
      }

      set<std::string> grep_results, my_results;

      //formulate grep command strings
      string cmd_str = string("grep -E ") + grep_searches[i] + " " + game->get_config().DICTIONARY();

      //run grep with popen
      cmd_pipe = popen(cmd_str.c_str(), "r");
      string output;
      while (!feof(cmd_pipe)) {
      fgets(text, sizeof(text), cmd_pipe);
      output = string(text);
      grep_results.insert(output);
      }
      pclose(cmd_pipe);
    */

    //JGF:
    //check that my search alg matches grep output
    //Constraint constraint(my_searches[i], compat_vectors[i], max_length_vectors[i], game->get_valid_words());
    //test_player->find_all_satisfying_strings(constraint, min_sizes[i], max_sizes[i]);
    //my_static_assert(vector_contains(test_player->m_recent_result, search_cont[i]),
    //                 std::string("contrained search failed to find valid word: ") + search_cont[i]);
    //}
  }

  /////////////////////////////////////////////////////////////////////////////
  static void test_three()
  /////////////////////////////////////////////////////////////////////////////
  {
    //
    //This test tests that the AI player is able to successfully make the first
    //depth-th plays of the game
    //

    for (unsigned depth = 1; depth <= 3; ++depth) {
      for (unsigned iteration = 0; iteration < 3; ++iteration) {
        auto game = Scrabble_Facade::get_test_game();

        my_static_assert(game->m_players.size() > 0, "Game had no players");
        game->initialize();
        AI_Player* test_player = dynamic_cast<AI_Player*>(game->m_players[0]);
        my_static_assert(test_player, "First player needs to be AI for this test");

        //hard-code AI letters for repeatability
        std::vector<std::string*> letters_for_depth;

        std::string letters_1[] = {"TIBIHXE", "SODKWEW", "QPEOR-E", "LMSCEIU", "MNBOIUP",
                                   "-AOEPLM", "PLASDUI", "IMNBXCS", "LAKSJDE", "PLQ-IDS"};

        std::string letters_2[] = {"CJAO-AE", "QPSDOAN", "ZVUISEW", "QWEUIOV", "ASDIPNA",
                                   "-SVWEJR", "HQEOQOG", "APFIQOJ", "QPBJKSJ", "FJA-QWE"};

        std::string letters_3[] = {"--APSDO", "BOOSAFP", "QWRNXCV", "KLGHFNV", "UIIOSDF",
                                   "UKLDFGH", "APSIOCS", "LLMLMS-", "ASDASUI", "ABSD-YT"};

        std::string letters_4[] = {"PQASLFX", "DFJKGAS", "SASDDIO", "KLGERTS", "POIESTY",
                                   "LOLERCO", "OMGSTSD", "QWSPX-O", "AEASESI", "BJAB-YZ"};

        std::string letters_5[] = {"JKASJDI", "JKGIKAJ", "SPZISJH", "QLIVHAE", "QPDIVBH",
                                   "LKGDFHJ", "OADSFGD", "QASDYRV", "YUIFFSD", "XCVB-CZ"};

        std::string* letter_srcs[] = {letters_1, letters_2, letters_3, letters_4, letters_5};

        my_static_assert(iteration < (sizeof(letters_1)/sizeof(std::string)),
                         std::string("iteration out of bounds: ") + obj_to_str(iteration));

        for (unsigned d = 0; d < depth; ++d) {
          for (unsigned i = 0; i < letter_srcs[d % 5][iteration].size(); ++i) {
            Scrabble_Piece* piece = const_cast<Scrabble_Piece*>(test_player->m_pieces[i]);
            piece->force_letter_change(letter_srcs[d % 5][iteration][i]);
          }
          test_player->remap();

          const Indv_Play& the_play = test_player->play();

          std::string real_err_msg;
          if ( (real_err_msg = game->evaluate_play(the_play)) != "") {
            std::cout << real_err_msg << std::endl;
            std::cout << the_play << std::endl;
            my_static_assert(false, "AIs should never make invalid play");
          }

          game->process_legit_play(the_play, test_player);
          std::cout << *game << std::endl;
        }
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  static void test_four()
  /////////////////////////////////////////////////////////////////////////////
  {
    //Ultimate test: AI plays until end of game.

    auto game = Scrabble_Facade::get_test_game();

    game->play();
  }

  /////////////////////////////////////////////////////////////////////////////
  static void test_five()
  /////////////////////////////////////////////////////////////////////////////
  {
    // Test that loading works, even if there's a letter imbalance due to
    // players using admin mode to cheat.

    const std::string save_dir = "saves";
    std::vector<std::string> saves_to_test = {"ella1", "holly1", "piece_overflow"};

    for (auto save_to_test : saves_to_test) {
      std::string save_path = save_dir + "/" + save_to_test;
      auto game = Scrabble_Facade::load_test_game(save_path);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  static void test_six()
  /////////////////////////////////////////////////////////////////////////////
  {
    // Baseline test

//#define GENERATE_BASELINES

    // Force a specific seed for repeatable results
    const int force_seed = 84;
    auto game = Scrabble_Facade::get_test_game(&force_seed);

    my_static_assert(game->m_players.size() > 0, "Game had no players");
    game->initialize();
    AI_Player* test_player = dynamic_cast<AI_Player*>(game->m_players[0]);
    my_static_assert(test_player, "First player needs to be AI for this test");

    static constexpr int moves_to_make = 20;
    static const std::string output_basename = "test_five.out";
    static const std::string gold_basename   = "test-files/test_five.gold";

    for (int i = 0; i < moves_to_make; ++i) {
      const auto& the_play = test_player->play();
      auto err_msg = game->evaluate_play(the_play);
      REQUIRE(err_msg == "");

      game->process_legit_play(the_play, test_player);

      std::ostringstream oss;
#ifdef GENERATE_BASELINES
      oss << gold_basename << "." << i;
#else
      oss << output_basename << "." << i;
#endif
      std::string output_filename = oss.str();
      game->save(output_filename);

#ifndef GENERATE_BASELINES
      oss.str("");
      oss.clear();

      oss << gold_basename << "." << i;
      std::string gold_filename = oss.str();

      REQUIRE(files_equal(output_filename, gold_filename));
#endif
    }
  }
};

namespace {

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("test_one", "[full]")
////////////////////////////////////////////////////////////////////////////////
{
  UnitWrap::FullTests::test_one();
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("test_two", "[full]")
////////////////////////////////////////////////////////////////////////////////
{
  UnitWrap::FullTests::test_two();
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("test_three", "[full]")
////////////////////////////////////////////////////////////////////////////////
{
  UnitWrap::FullTests::test_three();
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("test_four", "[full]")
////////////////////////////////////////////////////////////////////////////////
{
  UnitWrap::FullTests::test_four();
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("test_five", "[full]")
////////////////////////////////////////////////////////////////////////////////
{
  UnitWrap::FullTests::test_five();
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("test_six", "[full]")
////////////////////////////////////////////////////////////////////////////////
{
  UnitWrap::FullTests::test_six();
}

} // empty namespace
