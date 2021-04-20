extern "C"{
    #include "Libs/tcp.h"
    #include "Libs/server_func.h"
}
#include "Libs/tlv.hpp"
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 1 + 1)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    const in_port_t port = atoi(argv[1]);

    int sockfd = install_server(port);
    ERROR_SHUTDOWN("install_server()", sockfd);

    printf("Listening on port %d\n", port);

    sockfd = serverCore(sockfd);

    ERROR_SHUTDOWN("close(sockfd)", close(sockfd));

    return EXIT_SUCCESS;
}
