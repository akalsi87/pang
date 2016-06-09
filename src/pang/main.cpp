/**
 * \file main.cpp
 * \date Jun 08, 2016
 */

#include "pang/game.hpp"

int main(int argc, const char* argv[])
{
    pang::setBinDir(argv[0]);
    pang::Game g;
    g.start();
    return 0;
}
