#include "../server_func.hpp"
#include "../../Puiss4/game.hpp"
#include "../tcp.h"
#include "../tlv.hpp"
#include "../util_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

// A faire :
// - Refaire le jeu
// - Mettre sous fonction partis réfactorable
// - Corriger potentiel bug

int serverCore(int sockfd) {
  int rc;

  int fds[CONNEXIONS_LIMIT];

  while (1) {
    for (size_t i = 0; i < CONNEXIONS_LIMIT; i++) {
      struct sockaddr_in6 addr = {0};
      socklen_t addrlen = sizeof(addr);
      fds[i] = accept(sockfd, (struct sockaddr *)&addr, &addrlen);
      ERROR_HANDLER("accept()", fds[i]);

      printf("Connection from %s\n", str_of_sockaddr((struct sockaddr *)&addr));
    }

    pid_t pid;

    pid = fork();
    if (pid < 0) {
      ERROR_HANDLER("fork()", pid);
      ERROR_HANDLER("closeFds(fds)", closeFds(fds));
    } else if (pid == 0) {
      childWork(fds); // Manage connexions
      ERROR_HANDLER("closeFds(fds)", closeFds(fds));

      exit(0);
    }

    // Parent
    ERROR_HANDLER("closeFds(fds)", closeFds(fds));

    while (1) {
      // Handle connexion
      pid = waitpid(-1, NULL, WNOHANG);
      ERROR_HANDLER("waitpid(-1, NULL, WNOHANG)", pid);
    }
  }

  return sockfd;
}

// Bouger dans utils
int closeFds(int *fds) {
  int rc;
  int failure = 0;

  for (int i = 0; i < CONNEXIONS_LIMIT; i++) {
    rc = close(fds[i]);
    if (rc < 0) {
      failure = 1;
      ERROR_HANDLER("close(fd)", rc);
    }
  }

  if (failure) {
    return -1;
  }

  return 0;
}

int childWork(int *fds) {
  int rc;
  size_t i;
  Pseudo_t pseudo[2];
  Start_t start[2];

  Generic_tlv_t *tlv;

  for (i = 0; i < CONNEXIONS_LIMIT; i++) {
    rc = read_tlv(tlv, fds[i]);
    ERROR_SHUTDOWN("read_tlv(tlv, fds[i])", rc);

    pseudo[i] = READ_PSEUDO(tlv->msg);
  }

  Puiss4 *game = new Puiss4();
  Grid_t grid;
  grid.who = ROUGE;
  grid.won_draw = 0;
  grid.Grid = game->Grid;

  int color = rand() % 2;

  for (i = 0; i < CONNEXIONS_LIMIT; i++) {
    start[i].Client = pseudo[i];
    start[i].Opponent = pseudo[(i + 1) % 2];
    start[i].Pcolor = color;
    if (color == ROUGE) {
      ERROR_SHUTDOWN("SEND_GRID(grid, fds[i])", SEND_GRID(grid, fds[i]));
    }
    ERROR_SHUTDOWN("SEND_START(start[i], fds[i])",
                   SEND_START(start[i], fds[i]));
  }

  color = ROUGE;

  while (1) {
    rc = read_tlv(tlv, fds[color]);
    ERROR_SHUTDOWN("read_tlv(tlv, fds[color])", rc);

    // Decrypte tlv
    process_tlv(tlv, fds, color, game);

    // Renvoie tlv approprié
    color = (color + 1) % 2; // switch player si color = 0 -> 1 / = 1 -> 0
  }

  exit(0);
}

void process_tlv(Generic_tlv_t *tlv, int *fds, int color, Puiss4 *game) {
  switch (tlv->type) {
  case TYPE_MOVE: {
    Move_t move = READ_MOVE(tlv->msg);
    if (game->Turn(move, color)) {
      ERROR_SHUTDOWN("SEND_MOVE(move, fds[i])",
                     SEND_MOVE(move, fds[(color + 1) % 2]));
    } else if (game->In_game == WIN) {
      // Gagant
      Moveack_t moveack;
      moveack.Accepted = 1;
      moveack.Col = move;

      Grid_t grid;
      grid.Grid = game->Grid;
      grid.who = color;
      grid.won_draw = WIN;

      ERROR_SHUTDOWN("SEND_MOVE(move, fds[i])",
                     SEND_MOVEACK(moveack, fds[color]));
      ERROR_SHUTDOWN("SEND_MOVE(move, fds[i])", SEND_GRID(grid, fds[color]));
    } else if (game->In_game == DRAW) {
      // Draw
      Moveack_t moveack;
      moveack.Accepted = 1;
      moveack.Col = move;

      Grid_t grid;
      grid.Grid = game->Grid;
      grid.who = color;
      grid.won_draw = DRAW;

      ERROR_SHUTDOWN("SEND_MOVE(move, fds[i])",
                     SEND_MOVEACK(moveack, fds[color]));
      ERROR_SHUTDOWN("SEND_MOVE(move, fds[i])", SEND_GRID(grid, fds[color]));
    } else {
      Moveack_t moveack;
      moveack.Accepted = 0;
      moveack.Col = move;

      Grid_t grid;
      grid.Grid = game->Grid;
      grid.who = color;
      grid.won_draw = RUNNING;

      ERROR_SHUTDOWN("SEND_MOVE(move, fds[i])",
                     SEND_MOVEACK(moveack, fds[color]));
      ERROR_SHUTDOWN("SEND_MOVE(move, fds[i])", SEND_GRID(grid, fds[color]));
    }
    break;
  }

  case TYPE_CONCEDE: {
    ERROR_SHUTDOWN("SEND_MOVE(move, fds[i])",
                   SEND_CONCEDE(fds[(color + 1) % 2]));
    closeFds(fds);
    break;
  }

  case TYPE_DISCON: {
    ERROR_SHUTDOWN("SEND_MOVE(move, fds[i])",
                   SEND_DISCON(fds[(color + 1) % 2]));
    closeFds(fds);
    break;
  }

  default:
    break;
  }
  destroy_tlv(tlv);
}