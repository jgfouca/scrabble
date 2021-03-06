#include "player.hpp"
#include "scrabble_config.hpp"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
Player::Player(const std::string& name, Scrabble_Game* the_game) :
  m_name(name), m_score(0), m_the_game(the_game)
////////////////////////////////////////////////////////////////////////////////
{
  //fill player's tray with NULL pieces
  m_pieces.resize(the_game->get_config().NUM_PLAYER_PIECES());
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    m_pieces[i] = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Player::add_piece(const Scrabble_Piece* new_piece)
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(m_char_piece_map.size() == get_num_pieces(),
            std::string("Piece-map and tray sizes are out of sync: ") +
            " piece-map-size=" + obj_to_str(m_char_piece_map.size()) +
            " tray-size=" + obj_to_str(get_num_pieces()));

  //loop over tray-indeces. When we find a NULL value, insert there
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    if (!m_pieces[i]) {
      m_pieces[i] = new_piece;
      m_char_piece_map.insert(pair<char, const Scrabble_Piece*>(new_piece->get_letter(), new_piece));

      my_assert(!(new_piece->is_wildcard() && new_piece->get_letter() != '-'),
                std::string("wild-card piece should have had letter '-'") +
                ", instead found: " + obj_to_str(new_piece->get_letter()));
      return;
    }
  }
  //should never give a player too many pieces
  my_assert(false, "Tried to give player too many pieces.");
}

////////////////////////////////////////////////////////////////////////////////
void Player::remove_piece(const Scrabble_Piece* piece)
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(m_char_piece_map.size() == get_num_pieces(),
            std::string("Piece-map and tray sizes are out of sync: ") +
            " piece-map-size=" + obj_to_str(m_char_piece_map.size()) +
            " tray-size=" + obj_to_str(get_num_pieces()));

  //loop over tray until we find the exact same piece (matching addrs), then
  //remove the piece.
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    if (m_pieces[i] == piece) {
      m_pieces[i] = NULL;
      remap(); //inefficient, but that's OK
      return;
    }
  }
  //this operation should never fail
  my_assert(false, "Tried to remove a piece that the player did not have.");
}

////////////////////////////////////////////////////////////////////////////////
const Indv_Play& Player::play()
////////////////////////////////////////////////////////////////////////////////
{
  m_current_play.clear();

  //leave this part to children
  make_play();

  remap();
  return m_current_play;
}

////////////////////////////////////////////////////////////////////////////////
void Player::game_over()
////////////////////////////////////////////////////////////////////////////////
{
  //The game has notified us that the game has been completed, we must deduct
  //the value of our "stash" from our total
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    if (m_pieces[i] != NULL) {
      if (m_pieces[i]->get_point_val() > m_score) {
        //we cannot go negative
        m_score = 0;
        break;
      }
      else {
        m_score -= m_pieces[i]->get_point_val();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
const Scrabble_Piece* Player::observe_piece(unsigned idx) const
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(idx < get_num_pieces(), "Observed out of bounds");
  unsigned count = 0;
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    if (count == idx && m_pieces[i] != nullptr) {
      return m_pieces[i];
    }
    count += (m_pieces[i] != NULL);
  }
  my_assert(false, "Should never make it here");
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
unsigned Player::get_num_pieces() const
////////////////////////////////////////////////////////////////////////////////
{
  unsigned count = 0;
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    count += (m_pieces[i] != NULL);
  }
  return count;
}

////////////////////////////////////////////////////////////////////////////////
void Player::remap() const
////////////////////////////////////////////////////////////////////////////////
{
  m_char_piece_map.clear();
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    if (m_pieces[i]) {
      //my_assert(!m_pieces[i]->is_played(), "Piece was already played");
      char letter = m_pieces[i]->is_wildcard() ? '-' : m_pieces[i]->get_letter();
      m_char_piece_map.insert(pair<char, const Scrabble_Piece*>(letter,m_pieces[i]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
const Scrabble_Piece* Player::get_piece(char c) const
////////////////////////////////////////////////////////////////////////////////
{
  multimap<char, const Scrabble_Piece*>::iterator itr;

#ifndef NDEBUG
  //check the invariant that every piece in the map is also in the std::vector
  for (itr = m_char_piece_map.begin(); itr != m_char_piece_map.end(); itr++) {
    my_assert (has_piece(itr->second), "map and tray are out of sync");
  }
#endif

  //check that the player is attempting to play a real letter
  my_require(Scrabble_Piece::is_valid_letter(c), std::string("'") + obj_to_str(c) + "' is not a valid letter.");

  //look up the letter in the map, NOT the tray
  itr = m_char_piece_map.find(c);

  if (itr == m_char_piece_map.end()) {
    //we don't have the exact piece, but we can use a wildcard piece instead
    itr = m_char_piece_map.find('-');
    my_require(itr != m_char_piece_map.end(), "Player tried to play, but does not have '" + obj_to_str(c) + "'");

    //a wild-card is required to meet this request, change it's wildcard value
    itr->second->set_wildcard_value(c);
  }

  //remove the piece from the map, NOT the tray, and return it
  const Scrabble_Piece* rv = itr->second;
  m_char_piece_map.erase(itr);
  return rv;
}

////////////////////////////////////////////////////////////////////////////////
bool Player::has_piece(const Scrabble_Piece* piece) const
////////////////////////////////////////////////////////////////////////////////
{
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    if (m_pieces[i] == piece) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
ostream& Player::operator<<(ostream& out) const
////////////////////////////////////////////////////////////////////////////////
{
  out << "Player " << m_name << " has score " << m_score << endl;
  out << "  current pieces: ";
  for (unsigned i = 0; i < m_pieces.size(); i++) {
    if (m_pieces[i]) {
      out << *(m_pieces[i]) << " ";
    }
  }
  out << endl;
  out << "  current piece-map: ";
  for (multimap<char, const Scrabble_Piece*>::const_iterator itr = m_char_piece_map.begin();
       itr != m_char_piece_map.end(); itr++) {
    out << "(" << itr->first << "," << *(itr->second) << ") ";
  }

  return out;
}

////////////////////////////////////////////////////////////////////////////////
istream& Player::operator>>(istream& in)
////////////////////////////////////////////////////////////////////////////////
{
  std::string line;
  char buf[64];
  auto& piece_source = m_the_game->get_piece_source();

  getline(in, line);
  int rv = sscanf(line.c_str(), "Player %s has score %d", buf, &m_score);
  my_require(rv == 2, "Bad player score format");

  getline(in, line);
  std::string prefix = "  current pieces:";
  std::string real_pref = line.substr(0, prefix.size());
  my_require(real_pref == prefix, "Bad player tray");
  line = line.substr(prefix.size());

  for (char c : line) {
    if (c != ' ') {
      add_piece(piece_source.get_piece(c));
    }
  }

  // skip piece map
  getline(in, line);

  return in;
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Player& player)
////////////////////////////////////////////////////////////////////////////////
{
  return player.operator<<(out);
}

////////////////////////////////////////////////////////////////////////////////
istream& operator>>(istream& in, Player& player)
////////////////////////////////////////////////////////////////////////////////
{
  return player.operator>>(in);
}
