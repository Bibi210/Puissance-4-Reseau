#include "../util_func.h"

int write_all(int dst, void *buffer, size_t size) {
  int write_result;
  int to_write = size;
  char *save_buffer = buffer;
  int written = 0;

  while (to_write > 0) {
    write_result = write(dst, save_buffer, to_write);
    written += write_result;
    if (write_result < 0) {
      return write_result;
    }

    to_write -= write_result;
    save_buffer += write_result;
  }
  return written;
}

int read_all(int src, void *buffer, size_t size) {
  int read_result;
  int to_read = size;
  char *save_buffer = buffer;
  int readed = 0;
  while (to_read > 0) {
    read_result = read(src, save_buffer, to_read);
    readed += read_result;
    if (read_result < 0) {
      return read_result;
    }
    if (read_result == 0) {
      return readed;
    }

    to_read -= read_result;
    save_buffer += read_result;
  }
  return readed;
}

int error_gestion(const char *msg, int to_test,const char *file, int line, int mode) {
  int output = 0;
  if (to_test < 0) {
    fprintf(stderr, "%s error %s L:%d ", msg, file, line);
    perror("");
    output = -1;
  }
  if (output == -1 && mode == -1) {
    exit(output);
  }
  return output;
}

int retransmit(int fdin, int fdout) {
  char buf[BUFSIZE];
  while (1) {
    int rdc = read(fdin, buf, BUFSIZE);
    if (rdc < 0) {
      ERROR_HANDLER("read()", rdc);
      return -1;
    }
    if (rdc == 0) {
      return 0;
    }
    int wrc = write_all(fdout, buf, rdc);
    if (wrc < 0) {
      ERROR_HANDLER("write_all()", rdc);
      return -1;
    }
  }
}

//! Bouger dans utils
int closeFds(int *fds, int nb_con) {
  int rc;
  int failure = 0;

  for (int i = 0; i < nb_con; i++) {
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

//! prend que le premier chiffre de nb (ceci n'est pas un bug c'est une secret feature)
int parseUint8ToInt(uint8_t nb) {
  char parse[1] = {nb};

  return atoi(parse);
}