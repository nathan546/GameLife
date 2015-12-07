## Synopsis

This program outputs John Conway's Game of Life to a terminal using a statically allocated initial configuration.

Another feature was added to also output the game to an animated GIF image Most browser's can display animated GIFs.  Give it a try.  The terminal will begin by asking if you want to create a GIF over a number of cycles of the game. The GIF will then be created in the running directory as "output.gif."

## Compiling

g++ -o GameLife GameLife.cpp

You can ignore any deprecated compilation warnings.  The gif library being used is C-based, thus the C++ compiler may complain about deprecated string usages.
