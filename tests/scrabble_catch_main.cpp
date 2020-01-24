#define CATCH_CONFIG_RUNNER

#include "scrabble_exception.hpp"
#include "scrabble_kokkos.hpp"

#include "catch.hpp"

int main (int argc, char **argv) {

  Kokkos::initialize(argc, argv);

  Catch::Session catch_session;
  auto cli = catch_session.cli();
  catch_session.cli(cli);

  my_require(catch_session.applyCommandLine(argc,argv)==0,
             "Error! Something went wrong while parsing command line.\n");

  // Run tests
  int num_failed = catch_session.run(argc, argv);

  Kokkos::finalize();

  // Return test result
  return num_failed != 0 ? 1 : 0;
}
