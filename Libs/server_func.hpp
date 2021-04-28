#ifndef _SERFUNC_H
#define _SERFUNC_H

#include "../Puiss4/p4.hpp"
#include "tlv.hpp"


extern "C" {
#include "util_func.h"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "tcp.h"
}

#define CONNEXIONS_LIMIT 2

typedef enum Validity {
  NOT_ACCEPTED,
  ACCEPTED,
} Validity_t;


int serverCore(int sockfd);

int childWork(int *fds);
int moveProcess(Generic_tlv_t *tlv, int *fds, Puissance4_t *game);
int moveProcessAux(Move_t move, int state, int *fds, Puissance4_t *game);
int process_tlv(Generic_tlv_t *tlv, int *fds, Puissance4_t *game);

#endif // _SERFUNC_H included.