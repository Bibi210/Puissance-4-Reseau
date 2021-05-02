extern "C" {
#include "Libs/tcp.h"
}
#include "Libs/server_func.hpp"
#include "Libs/tlv.hpp"
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  const in_port_t port = atoi(argv[1]);
  bool debug_on = false;
  if (argc == 3 && atoi(argv[2])) {
    debug_on = true;
  }
  active_mode_debug(debug_on);

  int sockfd = install_server(port);
  ERROR_SHUTDOWN("install_server()", sockfd);

  printf("Listening on port %d\n", port);

  sockfd = serverCore(sockfd);

  ERROR_SHUTDOWN("close(sockfd)", close(sockfd));

  return EXIT_SUCCESS;
}
