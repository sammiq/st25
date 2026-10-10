/* main.c - program entry: set up the host, then run the game. */

#include "platform.h"

void GameMain(int argc, char **argv);

int main(int argc, char **argv)
{
    argc = plat_init(argc, argv);
    GameMain(argc, argv);
    plat_quit();
    return 0;
}
