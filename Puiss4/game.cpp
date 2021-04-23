#include "game.hpp"

bool Puiss4::Turn(Player who_played, Col col_where) {
  if (col_where > MAX_X)
    return false;
  if (who_played != player_A && player_B != who_played)
    return false;
  // Permet de Placer Le jeton a la bonne hauteur
  for (int lin = 0; lin < MAX_Y; lin++) {

    // Pseudo 2D
    Case In_process = col_where * (MAX_X - 1) + lin;

    if (Grid[In_process] == NONE_CASE) {
      Grid[In_process] = who_played;

      // Test fin du jeu
      Is_wining_turn(who_played, In_process);

      return true;
    }
  }
  return false;
}

void Puiss4::Abandon(Player who_gaveup) {
  return who_gaveup == player_B ? end_game(player_A) : end_game(player_B);
}

// On commence a tester a l'endroit du dernier tour
void Puiss4::Is_wining_turn(Player who_played, Case where) {
  array<uint8_t, 3> WinningPossible = {1, 6, 7};
  // Un peu Crade a lire mais opti +ou-
  for (auto var : WinningPossible) {
    for (int i = 0; i < 4; i++) {
      int to_test = (var * i) + where;
      if (Is_valid_case(to_test)) {
        if (Grid[to_test] != who_played) {
          break;
        }
      } else {
        break;
      }
      if (i == 3)
        return end_game(who_played);
    }

    for (int i = 0; i < 4; i++) {
      int to_test_reverse = (var * (-i)) + where;
      if (Is_valid_case(to_test_reverse)) {
        if (Grid[to_test_reverse] != who_played) {
          break;
        }
      } else {
        break;
      }
      if (i == 3)
        return end_game(who_played);
    }
  }
}

void Puiss4::end_game(Player winner) {
  In_game = WIN;
  this->winner = winner;
  return;
}

bool Puiss4::Is_valid_case(int case_nb) {
  return case_nb > 0 && case_nb < (uint8_t)Grid.size();
}

Puiss4::Puiss4() {
  In_game = RUNNING;
  Grid.fill(NONE_CASE);
}

string Puiss4::to_string() {
  string output = string();
  for (int lin = MAX_Y - 1; lin >= 0; lin--) {
    for (int col = 0; col < MAX_X; col++) {
      Case In_process = col * (MAX_X - 1) + lin;
      output.append("[");

      switch (Grid[In_process]) {
      case player_A:
        output.push_back('X');
        break;
      case player_B:
        output.push_back('O');
        break;
      case NONE_CASE:
        output.push_back(' ');
        break;
      default:
        output.push_back('E');
        break;
      }

      output.append("]");
    }
    output.push_back('\n');
  }
  return output;
}

string Puiss4::for_client(array<Case, (7 * 6)> client_grid) {
  string output = string();
  for (int lin = MAX_Y - 1; lin >= 0; lin--) {
    for (int col = 0; col < MAX_X; col++) {
      Case In_process = col * (MAX_X - 1) + lin;
      output.append("[");

      switch (client_grid[In_process]) {
      case player_A:
        output.push_back('X');
        break;
      case player_B:
        output.push_back('O');
        break;
     case NONE_CASE:
        output.push_back(' ');
        break;
      default:
        output.push_back('E');
        break;
      }
      output.append("]");
    }
    output.push_back('\n');
  }
  return output;
}
