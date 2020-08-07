# Guide

Scrabble/Words with friends program + artificial intelligence (AI)

This project is for scrabble/words with friends enthusiasts. It supports human vs. human, 
human vs. AI, and AI vs. AI modes. This program can be used as a platform for evaluating AIs with
various tunings and strategies. Interactive (play) mode can also "help you" (cheat) in your meatspace
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

To each mode has its own help:

```bash
% $path-to-clone/scrabble play --help
```

Graphical mode is more convenient than text mode for human player games; it can be enabled with `-u` or `--gui`:
```bash
% $path-to-clone/scrabble play -p myname -u
```

This will start a game with a single AI player that will play a full solo game with itself.

Coming soon:
* Experimental mode
* Multithreading with Kokkos
* graphical python interface

# History

I initially wrote this over 10 years ago using svn and C++03 (pre C++11) at the beginning of my professional career. When I moved the project to Git/GitHub, I didn't see much value in preserving the history (I had made no effort to keep it clean) so I started fresh with a snapshot. My lack of experience at the time is evident in the lack of functional decomposition in some of the complex AI functions and in the lack of proper unit testing.

As of 2020/01, I'm beginning a fresh push to dramatically increase the sophistication of the code base.
