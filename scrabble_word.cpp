#include "scrabble_word.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
unsigned Scrabble_Word::score() const
////////////////////////////////////////////////////////////////////////////////
{
  unsigned word_multiplier = 1, pre_multiplier_total = 0;
  for (std::map<unsigned, pair<const Scrabble_Piece*, Bonus> >::const_iterator itr = m_word.begin();
       itr != m_word.end(); itr++) {
    unsigned base_pts = itr->second.first->get_point_val();
    Bonus bonus       = itr->second.second;
    switch(bonus) {
    case NONE:
      pre_multiplier_total += base_pts;
      break;
    case DBL_LET:
      pre_multiplier_total += base_pts*2;
      break;
    case TRP_LET:
      pre_multiplier_total += base_pts*3;
      break;
    case DBL_WRD:
      pre_multiplier_total += base_pts;
      word_multiplier *= 2;
      break;
    case TRP_WRD:
      pre_multiplier_total += base_pts;
      word_multiplier *= 3;
      break;
    default:
      my_assert(false, std::string("Missing case for: ") + obj_to_str(bonus));
    }
  }
  return pre_multiplier_total * word_multiplier;
}

////////////////////////////////////////////////////////////////////////////////
void Scrabble_Word::add_component(const Scrabble_Piece* piece, Bonus bonus, unsigned order)
////////////////////////////////////////////////////////////////////////////////
{
  my_assert(m_word.find(order) == m_word.end(),
            obj_to_str(order) + " was already in the word.");

  m_word[order] = pair<const Scrabble_Piece*, Bonus>(piece, bonus);
}

////////////////////////////////////////////////////////////////////////////////
std::string Scrabble_Word::get_word_str() const
////////////////////////////////////////////////////////////////////////////////
{
  std::string rv;
  for (std::map<unsigned, pair<const Scrabble_Piece*, Bonus> >::const_iterator itr = m_word.begin();
       itr != m_word.end(); itr++) {
    rv += itr->second.first->get_letter();
  }
  return rv;
}

////////////////////////////////////////////////////////////////////////////////
ostream& Scrabble_Word::operator<<(ostream& out) const
////////////////////////////////////////////////////////////////////////////////
{
  out << get_word_str();
  return out;
}

////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Scrabble_Word& sw)
////////////////////////////////////////////////////////////////////////////////
{
  return sw.operator<<(out);
}
