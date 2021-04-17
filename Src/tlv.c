#include "../Header/tlv.h"

int send_tlv(Generic_tlv_t* to_send, int fdout) {
  int error = 0;

  error = write_all(fdout, &to_send->type, sizeof(uint8_t));
  if (error < 0) return error;

  error = write_all(fdout, &to_send->length, sizeof(uint8_t));
  if (error < 0) return error;

  for (int i = 0; i < to_send->length; i++) {
    error = write_all(fdout, &to_send->length, sizeof(uint8_t));
    if (error < 0) return error;
  }

  //! Free msg alloc in read_tlv or SEND_X
  free(to_send->msg);

  return EXIT_SUCCESS;
}

int read_tlv(Generic_tlv_t* to_read_into, int fdin) {
  int error = 0;

  error = read_all(fdin, &to_read_into->type, sizeof(uint8_t));
  if (error < 0) return error;

  error = read_all(fdin, &to_read_into->length, sizeof(uint8_t));
  if (error < 0) return error;
  
  //! Alloc for msg need a free
  to_read_into->msg = malloc(to_read_into->length);
  if(to_read_into->msg == NULL) return -1;

  for (int i = 0; i < to_read_into->length; i++) {
    error = read_all(fdin, &to_read_into->length, sizeof(uint8_t));
    if (error < 0) return error;
  }
  return EXIT_SUCCESS;
}

int retransmit_tlv(int fdin, int fdout){
  Generic_tlv_t bridge;
  int error = 0;

  error = read_tlv(&bridge, fdin);
  if (error < 0) return error;

  error = send_tlv(&bridge, fdout);
  if (error < 0) return error;

  return EXIT_SUCCESS;
}

int SEND_PSEUDO(uint8_t Plen,char* pseudo,int fdout){
  Generic_tlv_t to_send = {TYPE_PSEUDO,LEN_PSEUDO};
  uint8_t* in_build_msg = calloc(LEN_PSEUDO,sizeof(uint8_t));
  if (in_build_msg == NULL) return -1;

  in_build_msg[0] = Plen;
  for (size_t i = 0; i < Plen; i++)
     in_build_msg[i+1] = pseudo[i];
  to_send.msg = in_build_msg; 

  return send_tlv(&to_send, fdout);
}



