#ifndef scrabble_config_h
#define scrabble_config_h

#include <iostream>
#include <string>
#include <vector>

//struct _object;
//typedef _object PyObject;

using PyObject = bool(unsigned, unsigned, unsigned*, unsigned*, char*);

enum Board_Type {STANDARD_BOARD, WWF_BOARD, WWF_SOLO_BOARD};
enum Player_Type {HUMAN, AI};
enum Piece_Source_Type {STANDARD_SOURCE, WWF_SOURCE};
enum Output_Type {TEXT, GUI, EXPERIMENT};
enum GUI_Event {TILES, PLAY, BOARD_INIT, CHECK_PLAY, CONFIRM_PLAY};

/**
 * This class encapsulates everything that is configurable in this program.
 * Note that this class is a singleton.
 */

////////////////////////////////////////////////////////////////////////////////
class Scrabble_Config
////////////////////////////////////////////////////////////////////////////////
{
 public:

  Scrabble_Config(unsigned num_players,
                  const std::vector<Player_Type>& player_types,
                  const std::vector<std::string>& player_names,
                  unsigned num_player_pieces            = 7,
                  Board_Type board_type                 = STANDARD_BOARD,
                  Output_Type output                    = TEXT,
                  bool color_output                     = true,
                  bool clear_screen_before_output       = true,
                  const std::string& dictionary         = "TWL06.txt",
                  Piece_Source_Type piece_source_type   = STANDARD_SOURCE,
                  unsigned max_num_log_msgs_to_displ    = 10,
                  unsigned constrained_square_limit     = 3,
                  PyObject* py                          = nullptr);

  unsigned                  NUM_PLAYERS()                const { return m_num_players               ; }
  const std::vector<Player_Type>& PLAYER_TYPES()         const { return m_player_types              ; }
  const std::vector<std::string>& PLAYER_NAMES()         const { return m_player_names              ; }
  unsigned                  NUM_PLAYER_PIECES()          const { return m_num_player_pieces         ; }
  Board_Type                BOARD_TYPE()                 const { return m_board_type                ; }
  Output_Type               OUTPUT()                     const { return m_output                    ; }
  bool                      COLOR_OUTPUT()               const { return m_color_output              ; }
  bool                      CLEAR_SCREEN_BEFORE_OUTPUT() const { return m_clear_screen_before_output; }
  const std::string&        DICTIONARY()                 const { return m_dictionary                ; }
  Piece_Source_Type         PIECE_SOURCE_TYPE()          const { return m_piece_source_type         ; }
  unsigned                  MAX_NUM_LOG_MSGS_TO_DISPL()  const { return m_max_num_log_msgs_to_displ ; }
  unsigned                  CONSTRAINED_SQUARE_LIMIT()   const { return m_constrained_square_limit  ; }
  PyObject*                 PY_CALLBACK()                const { return m_py                        ; }

  static Board_Type str_to_board(const std::string& board_str);

  static Piece_Source_Type str_to_tileset(const std::string& tileset_str);

 private:

  unsigned                 m_num_players;
  std::vector<Player_Type> m_player_types;
  std::vector<std::string> m_player_names;
  unsigned                 m_num_player_pieces;
  Board_Type               m_board_type;
  Output_Type              m_output;
  bool                     m_color_output;
  bool                     m_clear_screen_before_output;
  std::string              m_dictionary;
  Piece_Source_Type        m_piece_source_type;
  unsigned                 m_max_num_log_msgs_to_displ;
  unsigned                 m_constrained_square_limit;
  PyObject*                m_py;
};

#endif
