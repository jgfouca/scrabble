#include "scrabble_point_map.h"
#include "scrabble_common.h"
#include "scrabble_exception.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
const Scrabble_Point_Map& Scrabble_Point_Map::instance()
////////////////////////////////////////////////////////////////////////////////
{
  static Scrabble_Point_Map glob_instance;
  return glob_instance;
}

////////////////////////////////////////////////////////////////////////////////
Scrabble_Point_Map::Scrabble_Point_Map()
////////////////////////////////////////////////////////////////////////////////
{
  //iterate over every valid letter (A-Z)
  for (char letter = 'A'; letter <= 'Z'; letter++) {
    //check if 'letter' is a 1-pt letter
    if (letter == 'E' || letter == 'A' || letter == 'I' || letter == 'O' ||
        letter == 'N' || letter == 'R' || letter == 'T' || letter == 'L' ||
        letter == 'S' || letter == 'U') {
      //add the 1-pt letters to the map
      m_point_map[letter] = 1;
    }
    //check if 'letter' is a 2-pt letter
    else if (letter == 'D' || letter == 'G') {
      //add the 2-pt letters to the map
      m_point_map[letter] = 2;
    }
    //check if 'letter' is a 3-pt letter
    else if (letter == 'B' || letter == 'C' || letter == 'M' || letter == 'P') {
      //add the 3-pt letters to the map
      m_point_map[letter] = 3;
    }
    //check if 'letter' is a 4-pt letter
    else if (letter == 'F' || letter == 'H' || letter == 'V' ||
             letter == 'W' || letter == 'Y') {
      //add the 4-pt letters to the map
      m_point_map[letter] = 4;
    }
    //check if 'letter' is a 5-pt letter
    else if (letter == 'K') {
      //add the 5-pt letters to the map
      m_point_map[letter] = 5;
    }
    //check if 'letter' is a 8-pt letter
    else if (letter == 'J' || letter == 'X') {
      //add the 8-pt letters to the map
      m_point_map[letter] = 8;
    }
    //check if 'letter' is a 10-pt letter
    else if (letter == 'Q' || letter == 'Z') {
      //add the 10-pt letters to the map
      m_point_map[letter] = 10;
    }
    else {
      my_static_assert(false, 
                       obj_to_str(letter) + " should have fallen into one of the above categories");
    }
  }
  //wild-card is worth 0
  m_point_map['-'] = 0;
}

////////////////////////////////////////////////////////////////////////////////
unsigned Scrabble_Point_Map::get_point_val(char letter) const
////////////////////////////////////////////////////////////////////////////////
{
  my_static_assert(m_point_map.find(letter) != m_point_map.end(),
                   obj_to_str(letter) + " was not in the point-map.");

  return m_point_map.find(letter)->second;
}
