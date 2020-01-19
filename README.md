# scrabble
Scrabble/Words with friends program + artificial intelligence (AI)

This project is for scrabble/words with friends enthusiasts. It supports human vs. human, 
human vs. AI, and AI vs. AI modes. The human interface is a bit clunky, so the primary use 
case revolves around the AIs. This program can be used as a platform for evaluating AIs with
various tunings and strategies. The command-mode can also "help you" (cheat) in your meatspace
scrabble games or words with friends games on your phone/ipad.

In order to take advantage of various languages strong suits, the front-end of the application is in python3
(good CLI libraries and easy graphics) and the engine is in C++ (high-performance, multithreading with Kokkos).

You will need Cmake to build the code and python3 to run the front-end:

```bash
% cmake $path-to-clone
...
% make -j8
...
% $path-to-clone/scrabble --help
```

As a quick demonstration of the AI, you can do:

```bash
% $path-to-clone/scrabble play -a bot1
```

This will start a game with a single AI player that will play a full solo game with itself.

Coming soon:
* Experimental mode
* Multithreading with Kokkos
