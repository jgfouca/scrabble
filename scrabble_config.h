#ifndef scrabble_config_h
#define scrabble_config_h

#include <iostream>
#include <string>
#include <vector>

enum Board_Type {STANDARD_BOARD};
enum Player_Type {HUMAN, AI, SUPERUSER};
enum Piece_Source_Type {STANDARD_SOURCE};
enum Assert_Fail_Action {GDB_ATTACH, EXCEPTION};
enum Game_Mode {TEST, NORMAL, COMMAND, INVALID};

/**
 * This class encapsulates everything that is configurable in this program.
 * Note that this class is a singleton.
 */

////////////////////////////////////////////////////////////////////////////////
class Scrabble_Config
////////////////////////////////////////////////////////////////////////////////
{
 public:
  static const Scrabble_Config& instance();
  
  unsigned                  NUM_PLAYERS()                const { return m_num_players               ; }
  const std::vector<Player_Type>& PLAYER_TYPES()               const { return m_player_types              ; }
  const std::vector<std::string>& PLAYER_NAMES()               const { return m_player_names              ; }
  Game_Mode                 GAME_MODE()                  const { return m_game_mode                 ; }
  unsigned                  NUM_PLAYER_PIECES()          const { return m_num_player_pieces         ; }
  Board_Type                BOARD_TYPE()                 const { return m_board_type                ; }
  bool                      PRODUCE_OUTPUT()             const { return m_produce_output            ; }
  bool                      COLOR_OUTPUT()               const { return m_color_output              ; }
  bool                      CLEAR_SCREEN_BEFORE_OUTPUT() const { return m_clear_screen_before_output; }
  const std::string&              DICTIONARY()                 const { return m_dictionary                ; }
  Piece_Source_Type         PIECE_SOURCE_TYPE()          const { return m_piece_source_type         ; } 
  unsigned                  MAX_NUM_LOG_MSGS_TO_DISPL()  const { return m_max_num_log_msgs_to_displ ; } 
  unsigned                  CONSTRAINED_SQUARE_LIMIT()   const { return m_constrained_square_limit  ; }
  Assert_Fail_Action        ASSERT_FAIL_ACTION()         const { return m_assert_fail_action        ; }
  
 private:
  Scrabble_Config(unsigned num_players,
                  const std::vector<Player_Type>& player_types,
                  const std::vector<std::string>& player_names,
                  Game_Mode game_mode,
                  unsigned num_player_pieces            = 7,
                  Board_Type board_type                 = STANDARD_BOARD,
                  bool produce_output                   = true,
                  bool color_output                     = true,
                  bool clear_screen_before_output       = true,
                  const std::string& dictionary         = "TWL06.txt",
                  Piece_Source_Type piece_source_type   = STANDARD_SOURCE, 
                  unsigned max_num_log_msgs_to_displ    = 10, 
                  unsigned constrained_square_limit     = 3,
                  Assert_Fail_Action assert_fail_action = GDB_ATTACH);
  
  //forbidden methods
  Scrabble_Config(const Scrabble_Config&);
  Scrabble_Config& operator=(const Scrabble_Config&);
  
  static void set_global_instance(const Scrabble_Config* new_inst);
  
  static const Scrabble_Config* s_glob_instance;

  unsigned                 m_num_players;
  std::vector<Player_Type> m_player_types;
  std::vector<std::string> m_player_names;
  Game_Mode                m_game_mode;
  unsigned                 m_num_player_pieces;
  Board_Type               m_board_type;
  bool                     m_produce_output;
  bool                     m_color_output;
  bool                     m_clear_screen_before_output;
  std::string              m_dictionary;
  Piece_Source_Type        m_piece_source_type; 
  unsigned                 m_max_num_log_msgs_to_displ; 
  unsigned                 m_constrained_square_limit;
  Assert_Fail_Action       m_assert_fail_action;
  
  friend class Scrabble_Facade;
};

#endif
