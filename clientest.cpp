#include <bits/stdint-uintn.h>
#include <unistd.h>
extern "C" {
#include "Libs/tcp.h"
#include "Libs/util_func.h"
}
#include "Libs/tlv.hpp"
#include "Puiss4/game.hpp"
#include <cstdlib>
#include <iostream>
using namespace std;

int main() {
  Generic_tlv_t to_read_into;
  Moveack_t Begin = {3,true};
  
  cout << "Welcome to test" << endl;

  int tunnel[2];
  ERROR_SHUTDOWN("pipe",pipe(tunnel));
  int pid = fork();
  if (pid == 0){
    ERROR_SHUTDOWN("Send ",SEND_MOVEACK(Begin,tunnel[1]));
    exit(0);
  }

  close(tunnel[1]);
  sleep(1);
  ERROR_SHUTDOWN("Read", read_tlv(&to_read_into, tunnel[0]));

  destroy_tlv(&to_read_into);

  return 0;
}