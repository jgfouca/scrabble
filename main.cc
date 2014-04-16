#include "scrabble_facade.h"

#include "mpi.h"

using namespace std;

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);
  
  Scrabble_Facade::instance().play(argc, argv);

  MPI_Finalize();
  
  return 0;
}
