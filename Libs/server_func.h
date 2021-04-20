#ifndef _SERFUNC_H
#define _SERFUNC_H

#define CONNEXIONS_LIMIT 2

int serverCore(int sockfd);

int childWork(int fd, int *pipefd);

int closePipes(int *pipefd);

#endif // _SERFUNC_H included.