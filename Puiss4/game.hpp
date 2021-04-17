#ifndef Puiss4_h
#define Puiss4_h

#include <array>
#include <bits/stdint-uintn.h>
#include <iostream>
#include <string>
#include <utility>

#define NONE_CASE 2
#define MAX_X 7
#define MAX_Y 6

using namespace std;

class Puiss4 {
  typedef uint8_t Player;
  typedef uint8_t Case;
  typedef uint8_t Col;

public:
  static const Player player_A = 0;
  static const Player player_B = 1;

  Puiss4();

  // Toujours en jeu ?
  bool In_game;
  Player winner;

  //*Coups possibles
  // Play the Turn and Return if the Turn is accepted or not
  bool Turn(Player who_played, Col where);

  // Call end_game
  void Abandon(Player who_gaveup);
  //*Coups possibles

  string to_string();

private:
  // Grille de 6*7 cases;
  array<Case, 42> Grid;

  // Check if the Turn close the game and call end_game
  void Is_wining_turn(Player who_played, Case where);

  // Check if a Case is valid
  bool Is_valid_case(int where);

  // Set winner and In_game
  void end_game(Player winner);
};

#endif // !Puiss4_h