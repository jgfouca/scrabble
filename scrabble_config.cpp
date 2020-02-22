#include "scrabble_config.hpp"
#include "scrabble_exception.hpp"

#include <cassert>
#include <sstream>

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
    return WWF_SOURCE;
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
                                 Output_Type output,
                                 bool color_output,
                                 bool clear_screen_before_output,
                                 const std::string& dictionary,
                                 Piece_Source_Type piece_source_type,
                                 unsigned max_num_log_msgs_to_displ,
                                 unsigned constrained_square_limit,
                                 PyObject* py) :
  m_num_players(num_players),
  m_player_types(player_types),
  m_player_names(player_names),
  m_num_player_pieces(num_player_pieces),
  m_board_type(board_type),
  m_output(output),
  m_color_output(color_output),
  m_clear_screen_before_output(clear_screen_before_output),
  m_dictionary(dictionary),
  m_piece_source_type(piece_source_type),
  m_max_num_log_msgs_to_displ(max_num_log_msgs_to_displ),
  m_constrained_square_limit(constrained_square_limit),
  m_py(py)
////////////////////////////////////////////////////////////////////////////////
{
  my_static_assert(num_players == player_types.size(),
                   "num_players did not match player_types.size()");
  my_static_assert(!(py == nullptr && output == GUI),
                   "Cannot do GUI without py callback obj");
}

////////////////////////////////////////////////////////////////////////////////
Scrabble_Config::Scrabble_Config(std::istream& in, Output_Type output, PyObject* py) :
  m_num_players(),
  m_player_types(),
  m_player_names(),
  m_num_player_pieces(),
  m_board_type(),
  m_output(output),
  m_color_output(),
  m_clear_screen_before_output(),
  m_dictionary(),
  m_piece_source_type(),
  m_max_num_log_msgs_to_displ(),
  m_constrained_square_limit(),
  m_py(py)
////////////////////////////////////////////////////////////////////////////////
{
  std::string line;
  int rv;

  getline(in, line);
  my_require(line == "Config", "Expected 'Config'");

  getline(in, line);
  rv = sscanf(line.c_str(), "num players: %d", &m_num_players);
  my_require(rv == 1, "Bad num players format");

  getline(in, line);
  istringstream iss(line);
  for (unsigned i = 0; i < m_num_players; ++i) {
    int tmp;
    char skip;
    iss >> tmp >> skip;
    Player_Type t = static_cast<Player_Type>(tmp);
    m_player_types.push_back(t);
  }

  getline(in, line);
  std::string curr_name;
  for (char c : line) {
    if (c == '|') {
      m_player_names.push_back(curr_name);
      curr_name = "";
    }
    else {
      curr_name += c;
    }
  }

  getline(in, line);
  rv = sscanf(line.c_str(), "num pieces: %d", &m_num_player_pieces);
  my_require(rv == 1, "Bad num pieces format");

  getline(in, line);
  rv = sscanf(line.c_str(), "board type: %d", &m_board_type);
  my_require(rv == 1, "Bad board type format");

  char buf[64];
  getline(in, line);
  rv = sscanf(line.c_str(), "dict: %s", buf);
  my_require(rv == 1, "Bad dict format");
  m_dictionary = std::string(buf);

  getline(in, line);
  rv = sscanf(line.c_str(), "piece source: %d", &m_piece_source_type);
  my_require(rv == 1, "Bad piece source format");

  getline(in, line);
  my_require(line == "", "Expected blank line");
}

////////////////////////////////////////////////////////////////////////////////
ostream& Scrabble_Config::operator<<(ostream& out) const
////////////////////////////////////////////////////////////////////////////////
{
  out << "num players: " << m_num_players << "\n";
  for (auto item : m_player_types) {
    out << item << " ";
  }
  out << "\n";
  for (auto name : m_player_names) {
    out << name << "|";
  }
  out << "\n";
  out << "num pieces: " << m_num_player_pieces << "\n";
  out << "board type: " << m_board_type << "\n";
  out << "dict: " << m_dictionary << "\n";
  out << "piece source: " << m_piece_source_type << "\n";

  return out;
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Scrabble_Config& config)
////////////////////////////////////////////////////////////////////////////////
{
  return config.operator<<(out);
}
