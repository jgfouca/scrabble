#ifndef scrabble_tester_h
#define scrabble_tester_h

class Scrabble_Game;

/**
 * This class contains the tests for this program. Instead of having small
 * unit tests for each individual class, I decided to write just a few big
 * tests that test that the program as-a-whole is behaving as intended. 
 *
 * JGF: A challenge that needs to be solved is validation/verification. How do 
 * we know if the AI, while appearing to operate correctly on the surface, 
 * is not being held-back by mistakes in the code or algorithm? We'll probably
 * need to set up some complex scenarios with a non-obvious optimal play and
 * see if the AI discovers it.
 */

////////////////////////////////////////////////////////////////////////////////
class Scrabble_Tester
////////////////////////////////////////////////////////////////////////////////
{
 public:
  /**
   * test_one - Tests that basic scrabble engine works correctly:
   *            1) Tests that word formation is done correctly (the game detects 
   *               that the right words are created for a play).
   *            2) Tests that scoring is done correctly.
   *            3) Tests that individual constrained searches appear to work.
   */
  static void test_one(Scrabble_Game* game, bool wait);

  /**
   * test_two - Uses grep to validate the results of constrained searches. This 
   *            is important because constrained searches are the heart of the 
   *            AI.
   */
  static void test_two(Scrabble_Game* game);

  /**
   * test_three - A broad test of the AI. The AI will play game_depth-th moves 
   *              starting from an empty board. The AI's pieces are hard-coded
   *              for repeatability.
   */
  static void test_three(Scrabble_Game* game, unsigned iteration, unsigned game_depth, bool wait);

  /**
   * test_four - The ultimate AI test: the AI plays a game from start to finish,
   *             just as it would during a real game. Nothing is hard-coded, 
   *             although it might be a good idea to set and record the random 
   *             seed used for each game. This will allow you to repeat a game
   *             if a bug pops up.
   */
  static void test_four(Scrabble_Game* game, bool wait);
};

#endif
