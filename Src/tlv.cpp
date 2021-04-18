#include "../Header/tlv.hpp"

int send_tlv(Generic_tlv_t* to_send, int fdout) {
  int error = 0;

  error = write_all(fdout, &to_send->type, sizeof(uint8_t));
  if (error < 0) return error;

  error = write_all(fdout, &to_send->length, sizeof(uint8_t));
  if (error < 0) return error;

 
  if (to_send->length){

    for (int i = 0; i < to_send->length; i++) {
    error = write_all(fdout, &to_send->length, sizeof(uint8_t));
    if (error < 0) return error;
    }
    //! Free for msg alloc in read_tlv or SEND_X
    free(to_send->msg);
  }
  return EXIT_SUCCESS;
}

int read_tlv(Generic_tlv_t* to_read_into, int fdin) {
  int error = 0;

  error = read_all(fdin, &to_read_into->type, sizeof(uint8_t));
  if (error < 0) return error;

  error = read_all(fdin, &to_read_into->length, sizeof(uint8_t));
  if (error < 0) return error;
  
  if (to_read_into->length)
  {
    //! Alloc for msg need a free
    to_read_into->msg = (uint8_t *)malloc(to_read_into->length);
    if(to_read_into->msg == NULL) return -1;

    for (int i = 0; i < to_read_into->length; i++) {
    error = read_all(fdin, &to_read_into->length, sizeof(uint8_t));
    if (error < 0) return error;
    }
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

void add_pseudo(uint8_t Plen,char* pseudo,uint8_t* msg){
  msg[0] = Plen;
  for (size_t i = 0; i < Plen; i++)
  {
    msg[i+1] = pseudo[i];
  }
  return;
}

int pre_init_msg(uint8_t Type, uint8_t Len, Generic_tlv_t *to_init){
  to_init->type = Type;
  to_init->length = Len;

  if (to_init->length){
    to_init->msg = (uint8_t *)calloc(to_init->length, sizeof(uint8_t));
    if (to_init->msg == NULL) return -1;
  }

  return EXIT_SUCCESS;
}

int SEND_PSEUDO(uint8_t Plen,char* pseudo,int fdout){
  Generic_tlv_t to_send;
  
  if (pre_init_msg(TYPE_PSEUDO,LEN_PSEUDO,&to_send) < 0) 
    return -1;
  add_pseudo(Plen,pseudo,to_send.msg);
  return send_tlv(&to_send, fdout);
}

int SEND_START(uint8_t Pcolor, uint8_t Plen_A, char *pseudo_A, uint8_t Plen_B, char *pseudo_B,int fdout){
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_START,LEN_START,&to_send) < 0) 
    return -1;
  to_send.msg[0] = Pcolor;

  add_pseudo(Plen_A,pseudo_A,to_send.msg + 1);
  add_pseudo(Plen_B,pseudo_B,to_send.msg + 1 + 1 + 64);

  return send_tlv(&to_send, fdout);
}

int SEND_GRID(uint8_t *State, uint8_t *Grid, int fdout){
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_GRID,LEN_GRID,&to_send) < 0) 
    return -1;

  for (size_t i = 0; i < STATE_SIZE; i++)
    to_send.msg[i] = State[i];
  
  for (size_t i = 0; i < GRID_SIZE; i++)
    to_send.msg[i + STATE_SIZE] = Grid[i];
  
  return send_tlv(&to_send, fdout);
}

int SEND_MOVE(uint8_t CDL, int fdout){
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_MOVE,LEN_MOVE,&to_send) < 0)
    return -1;

  to_send.msg[0] = CDL;

  return send_tlv(&to_send, fdout);
}

int SEND_MOVEACK(uint8_t CDL, uint8_t Ok, int fdout){
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_MOVEACK,LEN_MOVEACK,&to_send) < 0)
    return -1;
  to_send.msg[0] = CDL;
  to_send.msg[1] = Ok;

  return send_tlv(&to_send,fdout);
}

int SEND_CONCEDE(int fdout){
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_CONCEDE,LEN_CONCEDE,&to_send) < 0)
    return -1;
  
  return send_tlv(&to_send, fdout);
}

int SEND_DISCON(int fdout){
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_DISCON,LEN_DISCON,&to_send) < 0)
    return -1;
  
  return send_tlv(&to_send, fdout);
}



