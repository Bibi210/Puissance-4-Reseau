#ifndef TLV_PROGET_H
#define TLV_PROGET_H

#include "../Header/util_func.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>



enum TLV_types {
  TYPE_PSEUDO = 1,
  TYPE_START = 2,
  TYPE_GRID = 3,
  TYPE_MOVE = 4,
  TYPE_MOVEACK = 5,
  TYPE_CONCEDE = 6,
  TYPE_DISCON = 7
};

enum TLV_len {
  LEN_INFO_MSG = 0,
  LEN_PSEUDO = 65,
  LEN_START = 131,
  LEN_GRID = 6*7, //! Maybe Change for better
  LEN_MOVE = 1,
  LEN_MOVEACK = 2,
  
  LEN_CONCEDE = LEN_INFO_MSG,
  LEN_DISCON = LEN_INFO_MSG,
};

typedef struct Generic_tlv_t {
  uint8_t type;
  uint8_t length;
  uint8_t *msg;
} Generic_tlv_t;

/// Envoie un TLV sur fdout
int send_tlv(Generic_tlv_t* to_send, int fdout);

/// Lit un TLV sur fdin *bloquant*
int read_tlv(Generic_tlv_t* to_read_into,int fdin);

/// Lis un tlv de fdin et l'envoie sur fdout *bloquant*
int retransmit_tlv(int fdin, int fdout);

//! Only for debug purposes
void display_TLV_msg(Generic_tlv_t);

int SEND_PSEUDO(uint8_t Plen,char *msg,int fdout);

#endif // !TLV_PROGET_H