#ifndef _SERFUNC_H
#define _SERFUNC_H

#include "tlv.hpp"
#include "../Puiss4/p4.hpp"

#define CONNEXIONS_LIMIT 2

int serverCore(int sockfd);

int childWork(int *fds);

int process_tlv(Generic_tlv_t *tlv, int *fds, Puissance4_t *game);

#endif // _SERFUNC_H included.