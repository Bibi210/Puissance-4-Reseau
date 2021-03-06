#ifndef _P4_HPP
#define _P4_HPP

#include <array>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

#define COL_MAX 7
#define LIN_MAX 6

#define NB_DIR 4
#define POL 2

typedef enum Belonging_t : uint8_t {
  ROUGE = 0,
  JAUNE = 1,
  FREE = 2
} Belonging_t;

typedef enum Game_state {
  RUNNING,
  WIN,
  DRAW
} Game_state;

typedef struct Puissance4 {
  array<uint8_t, (COL_MAX * LIN_MAX)> grid;
  Belonging_t player;
  int turn;
} Puissance4_t;

void gameInit(Puissance4_t *game);
void gameShow(Puissance4_t *game);

// Apply move
int gameTurn(Puissance4_t *game, int col);

// return -1 if not valid, otherwise return the move's cell
int testValidity(int col, Puissance4_t *game);
int testWin(int move, Puissance4_t *game);
int testFollowing(int move, int direction, Puissance4_t *game);
int testDir(int direction, int next_case, int move);
string gameShowToString(array<uint8_t, (COL_MAX * LIN_MAX)> grid);

#endif // _P4_HPP included
