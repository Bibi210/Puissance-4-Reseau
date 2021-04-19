extern "C" {
#include "Libs/tcp.h"
#include "Libs/util_func.h"
}
#include "Libs/tlv.hpp"
#include "Puiss4/game.hpp"
#include <cstdlib>
#include <iostream>
using namespace std;

array<uint8_t,MAX_X*MAX_Y> Grid;

// TODO All these functions
uint make_move();
void process_start();
void process_moveack();
void update_grid();
void process_concede();
void process_disconnect();


void process_tlv(Generic_tlv_t *in_process) {
  switch (in_process->type) {
  case TYPE_START:
    process_start();
    break;

  case TYPE_MOVEACK:
    process_moveack();
    break;

  case TYPE_GRID:
    update_grid();
    break;

  case TYPE_CONCEDE:
    process_concede();
    break;

  case TYPE_DISCON:
    process_disconnect();
    break;

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
    
  //TODO Loop on process_tlv
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