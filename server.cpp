extern "C"{
    #include "Header/tcp.h"
    #include "Header/server_func.h"
}
#include "Header/tlv.hpp"
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

    int rc;

    int sockfd = install_server(port);
    ERROR_HANDLER("install_server()", sockfd);

    printf("Listening on port %d\n", port);

    sockfd = serverCore(sockfd);

    ERROR_HANDLER("close(sockfd)", close(sockfd));

    return EXIT_SUCCESS;
}
