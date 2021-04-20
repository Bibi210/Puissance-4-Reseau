#ifndef _SERFUNC_H
#define _SERFUNC_H

#define CONNEXIONS_LIMIT 2

int serverCore(int sockfd);

int childWork(int *fds);

int closeFds(int *fds);

#endif // _SERFUNC_H included.