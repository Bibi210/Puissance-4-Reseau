#ifndef UTIL_FUNC_H
#define UTIL_FUNC_H

#define BUFSIZE 4096

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int error_gestion(const char *msg, int to_test,const char *file,int line, int mode);
#define ERROR_SHUTDOWN(msg, to_test)                                           \
  error_gestion(msg, to_test, __FILE__, __LINE__, -1)
#define ERROR_HANDLER(msg, to_test)                                            \
  error_gestion(msg, to_test, __FILE__, __LINE__, 0)

int write_all(int dst, void *buffer, size_t buffsize);
int read_all(int src, void *buffer, size_t buffsize);
int retransmit(int fdin, int fdout);
#endif // !UTIL_FUNC_H
