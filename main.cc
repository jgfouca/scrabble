#include "scrabble_facade.h"

using namespace std;

int main(int argc, char** argv)
{
  Scrabble_Facade::instance().play(argc, argv);

  return 0;
}
