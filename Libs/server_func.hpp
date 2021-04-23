#ifndef _SERFUNC_H
#define _SERFUNC_H

#include "tlv.hpp"
#include "../Puiss4/game.hpp"

#define CONNEXIONS_LIMIT 2

int serverCore(int sockfd);

int childWork(int *fds);

int closeFds(int *fds);

void process_tlv(Generic_tlv_t *tlv, int *fds, int color, Puiss4 *game);

#endif // _SERFUNC_H included.