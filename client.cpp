#include <bits/stdint-uintn.h>
#include <unistd.h>
extern "C" {
#include "Libs/tcp.h"
#include "Libs/util_func.h"
}
#include "Libs/tlv.hpp"
#include "Puiss4/p4.hpp"
#include <cstdlib>
#include <iostream>
using namespace std;

int main() { 
  cout << "Welcome to test" << endl;

  int tunnel[2];
  ERROR_SHUTDOWN("pipe",pipe(tunnel));
  int pid = fork();
  if (pid == 0){
    Move_t Begin = 1;
    ERROR_SHUTDOWN("Send ",SEND_MOVE(Begin,tunnel[1]));
    exit(0);
  }

  close(tunnel[1]);
  sleep(1);
  Generic_tlv_t to_read_into;
  ERROR_SHUTDOWN("Read", read_tlv(&to_read_into, tunnel[0]));
  Move_t test = 26;
  test = READ_MOVE(to_read_into.msg);
  cout << +test << endl;

  destroy_tlv(&to_read_into);

  return 0;
}