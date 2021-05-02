#include "../server_func.hpp"
// A faire :
// - Corriger potentiel bug

int serverCore(int sockfd) {
  int rc;
  int fds[CONNEXIONS_LIMIT];

  while (1) {
    for (size_t i = 0; i < CONNEXIONS_LIMIT; i++) {
      struct sockaddr_in6 addr;
      socklen_t addrlen = sizeof(addr);
      fds[i] = accept(sockfd, (struct sockaddr *)&addr, &addrlen);
      if (fds[i] <  0) {
        ERROR_HANDLER("accept()", fds[i]);
        i--;
      }

      printf("Connection from %s\n", str_of_sockaddr((struct sockaddr *)&addr));
    }

    pid_t pid;

    pid = fork();
    if (pid < 0) {
      ERROR_HANDLER("fork()", pid);
      ERROR_HANDLER("closeFds(fds)", closeFds(fds, CONNEXIONS_LIMIT));
      continue;
    } else if (pid == 0) {
      ERROR_SHUTDOWN("childWork(fds)", childWork(fds)); // Manage connexions

      ERROR_SHUTDOWN("closeFds(fds)", closeFds(fds, CONNEXIONS_LIMIT));

      exit(0);
    }

    // Parent
    ERROR_HANDLER("closeFds(fds)", closeFds(fds, CONNEXIONS_LIMIT));

    //! TODO Block Server wait wnohang
    while ((rc = waitpid(-1, NULL, WNOHANG)))
    {
      if (rc < 0)
      {
        ERROR_HANDLER("waitpid(-1, NULL, WNOHANG)", rc);
        break;
      }
    }
  }

  return sockfd;
}

int childWork(int *fds) {
  int rc;

  Pseudo_t pseudo[2];
  Start_t start[2];

  Generic_tlv_t tlv;

  for (size_t i = 0; i < CONNEXIONS_LIMIT; i++) {
    rc = read_tlv(&tlv, fds[i]); // Read tlv
    if (rc < 0) {
      ERROR_HANDLER("read_tlv(tlv, fds[i])", rc);
      return -1;
    }

    pseudo[i] = READ_PSEUDO(tlv.msg);
  }

  Puissance4_t game;
  gameInit(&game);

  Grid_t grid;
  grid.who = game.player;
  grid.won_draw = 0;
  grid.Grid = game.grid;

  srand(getpid());
  int color = rand() % 2;

  int first_player;

  for (size_t i = 0; i < CONNEXIONS_LIMIT; i++) {
    start[i].Client = pseudo[i];
    start[i].Opponent = pseudo[(i + 1) % 2];
    start[i].Pcolor = color;

    rc = SEND_START(start[i], fds[i]);
    if (rc < 0) {
      ERROR_HANDLER("SEND_START(start[i], fds[i])", rc);
      return -1;
    }

    rc = SEND_GRID(grid, fds[i]);
    if (rc < 0) {
      ERROR_HANDLER("SEND_GRID(grid, fds[i])", rc);
      return -1;
    }

    if (color == ROUGE) {
      first_player = i;
    }

    color = (color + 1) % 2; // switch player si color = 0 -> 1 / = 1 -> 0
  }

  if (first_player != 0) {
    // Reverse fds
    int temp = fds[0];
    fds[0] = fds[1];
    fds[1] = temp;
  }

  while (1) {
    rc = read_tlv(&tlv, fds[game.player]);
    if (rc < 0) {
      ERROR_HANDLER("read_tlv(tlv, fds[game.player])", rc);
      return -1;
    }
    
    // Decrypte tlv
    rc = process_tlv(&tlv, fds, &game);
    if (rc < 0) {
      ERROR_HANDLER("process_tlv(tlv, fds, &game)", rc);
      return -1;
    }

    if (rc == 1) { // Partie fini
      break;
    }
  }

  exit(0);
}

int process_tlv(Generic_tlv_t *tlv, int *fds, Puissance4_t *game) {
  int rc;
  int crc;

  if (tlv->type == TYPE_MOVE) {
    rc = moveProcess(tlv, fds, game);
    if (rc < 0) {
      ERROR_HANDLER("moveProcess(tlv, fds, game)", rc);
    }
  } else {
    switch (tlv->type) {
    case TYPE_CONCEDE: {
      rc = SEND_CONCEDE(fds[(game->player + 1) % 2]);
      if (rc < 0) {
        ERROR_HANDLER("SEND_CONCEDE(fds[(game->player + 1) % 2])", rc);
      }

      break;
    }

    case TYPE_DISCON: {
      rc = SEND_DISCON(fds[(game->player + 1) % 2]);
      if (rc < 0) {
        ERROR_HANDLER("SEND_DISCON(fds[(game->player + 1) % 2])", rc);
      }

      break;
    }

    default:
      fprintf(stderr, "Unknown tlv\n");
      break;
    }

    crc = closeFds(fds, CONNEXIONS_LIMIT);
    if (crc < 0) {
      ERROR_HANDLER("closeFds(fds, CONNEXIONS_LIMIT)", crc);
    }

    rc = (rc < 0 || crc < 0) ? -1 : 1;
  }
  
  destroy_tlv(tlv);
  return rc;
}

int moveProcess(Generic_tlv_t *tlv, int *fds, Puissance4_t *game) {
  int rc;

  Move_t move = READ_MOVE(tlv->msg);

  int m = parseUint8ToInt(move);

  int state = gameTurn(game, m);

  uint8_t who_to_send = game->player;
  Validity_t move_accepted = ACCEPTED;

  if (state == -1) {
    move_accepted = NOT_ACCEPTED;
  }

  Moveack_t moveack;
  moveack.Accepted = (bool)move_accepted;
  moveack.Col = move;

  Grid_t grid;
  grid.Grid = game->grid;
  grid.who = who_to_send;
  grid.won_draw = state;

  rc = SEND_MOVEACK(moveack, fds[who_to_send]);
  if (rc < 0) {
    ERROR_HANDLER("SEND_MOVEACK(moveack, fds[who_to_send])", rc);
    return -1;
  }

  rc = SEND_GRID(grid, fds[who_to_send]);
  if (rc < 0) {
    ERROR_HANDLER("SEND_GRID(grid, fds[who_to_send])", rc);
    return -1;
  }

  if (state != -1) {
    rc = SEND_MOVEACK(moveack, fds[(who_to_send + 1) % 2]);
    if (rc < 0) {
      ERROR_HANDLER("SEND_MOVEACK(moveack, fds[(who_to_send + 1) % 2])", rc);
      return -1;
    }

    rc = SEND_GRID(grid, fds[(who_to_send + 1) % 2]);
    if (rc < 0) {
      ERROR_HANDLER("SEND_GRID(grid, fds[(who_to_send + 1) % 2])", rc);
      return -1;
    }
  }

  if (state == WIN || state == DRAW) {
    return 1;
  }

  return 0;
}
