extern "C" {
#include "Libs/tcp.h"
#include "Libs/util_func.h"
}
#include "Libs/tlv.hpp"
#include "Puiss4/game.hpp"
#include <cstdlib>
#include <iostream>
using namespace std;

string Pseudo;
string OPpseudo;
uint8_t Color;

int make_move_send(int serv_fd) {
  // TODO Take the grid and test if move valid
  Move_t to_send;
  cout << endl;
  cout << "Choisisez une colone : " << endl;
  cin >> to_send;
  return SEND_MOVE(to_send, serv_fd);
}

void process_tlv(Generic_tlv_t *in_process, int serv_fd) {
  switch (in_process->type) {
  case TYPE_START: {
    Start_t Received = READ_START(in_process->msg);
    Color = Received.Pcolor;
    Pseudo = Received.Client.Nickname;
    OPpseudo = Received.Opponent.Nickname;
  } break;

  case TYPE_MOVEACK: {
    int error = 0;
    Moveack_t Received = READ_MOVEACK(in_process->msg);
    if (!Received.Accepted) {
      error = make_move_send(serv_fd);
      ERROR_SHUTDOWN("SEND MOVE ", error);
    }

  } break;

  case TYPE_GRID: {
    int error = 0;
    Grid_t Received = READ_GRID(in_process->msg);
    cout << Puiss4::for_client(Received.Grid) << endl;
    switch (Received.won_draw) {
    case 0:
      if (Received.who == Color) {
        error = make_move_send(serv_fd);
        ERROR_SHUTDOWN("SEND MOVE ", error);
      }
      break;
    case 1:
      cout << "Player :" << +Received.who << " Won" << endl;
      exit(EXIT_SUCCESS);
    case 2:
      cout << "Draw " << endl;
      break;
    default:
      cerr << "Unknown State" << endl;
      break;
    }
  } break;

  case TYPE_CONCEDE: {
    cout << "You Won!" << endl;
    exit(EXIT_SUCCESS);
  } break;

  case TYPE_DISCON: {
    cout << "Op Disconnect" << endl;
    exit(EXIT_SUCCESS);
  } break;

  default:
    cerr << "Unknown TLV ignored " << endl;
    break;
  }
  destroy_tlv(in_process);
}

int game(int serv_fd, const char *pseudo) {
  Generic_tlv_t in_process;
  int error = 0;
  error = SEND_PSEUDO(strlen(pseudo), pseudo, serv_fd);
  if (error < 0)
    return error;

  error = read_tlv(&in_process, serv_fd);
  if (error < 0)
    return error;

  while (1) {
    read_tlv(&in_process, serv_fd);
    cout << "Tlv Reçu " << endl;
    process_tlv(&in_process, serv_fd);
  }
  return EXIT_SUCCESS;
}

void client_shutdown(int exitcode, void *fd) {
  (void)exitcode;
  int fd_serv = *(int *)fd;
  if (fd_serv > 0) {
    ERROR_HANDLER("Disconnect send ", SEND_DISCON(fd_serv));
    close(fd_serv);
  }
}

int main(int argc, const char **argv) {
  if (argc < 4) {
    cerr << "Usage: PROG IP PORT PSEUDO" << endl;
    return -1;
  }
  const char *pseudo = argv[3];
  int port = atoi(argv[2]);
  int serv_fd = install_client(argv[1], port);
  ERROR_SHUTDOWN("Install client", serv_fd);

  // Permet Lors de la fin du program de SEND_DISCON si possible
  on_exit(client_shutdown, &serv_fd);

  int error = 0;
  error = game(serv_fd, pseudo);
  ERROR_SHUTDOWN("Game process", error);

  return EXIT_SUCCESS;
}
