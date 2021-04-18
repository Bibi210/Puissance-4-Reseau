extern "C"{
    #include "Header/tcp.h"
}
#include "Header/tlv.hpp"
#include <iostream>

using namespace std;

int main(int argc, const char** argv) {
    if (argc < 3) {
        cerr << "Usage: PROG IP PORT" << endl;
        return -1;
    }
    int port = atoi(argv[2]);
    install_client(argv[1],port);

    return EXIT_SUCCESS;
}