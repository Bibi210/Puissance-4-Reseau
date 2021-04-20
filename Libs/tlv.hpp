#ifndef TLV_PROGET_H
#define TLV_PROGET_H

extern "C" {
#include "util_func.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
}

#define STATE_SIZE 2
#define GRID_SIZE (6 * 7)

enum TLV_types : uint8_t {
  TYPE_PSEUDO = 1,
  TYPE_START = 2,
  TYPE_GRID = 3,
  TYPE_MOVE = 4,
  TYPE_MOVEACK = 5,
  TYPE_CONCEDE = 6,
  TYPE_DISCON = 7
};

enum TLV_len : uint8_t {
  LEN_INFO_MSG = 0,
  LEN_PSEUDO = 65,
  LEN_START = 131,
  LEN_GRID = 6 * 7, //! Maybe Change for better
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
int send_tlv(Generic_tlv_t *to_send, int fdout);

/// Lit un TLV sur fdin *bloquant*
int read_tlv(Generic_tlv_t *to_read_into, int fdin);

/// Lis un tlv de fdin et l'envoie sur fdout *bloquant*
int retransmit_tlv(int fdin, int fdout);

///Destroy TLV (free)
void destroy_tlv(Generic_tlv_t* to_destroy);

//! Only for debug purposes
void display_TLV_msg(Generic_tlv_t);

//! SENDING_FUNCS Alloc for msg free in send_tlv
int SEND_PSEUDO(uint8_t Plen, const char *pseudo, int fdout);

/// Pcolor = 0 or 1
int SEND_START(uint8_t Pcolor, uint8_t Plen_A, const char *pseudo_A,
               uint8_t Plen_B, const char *pseudo_B, int fdout);

/// State_size = 2 , Grid_size = 42
int SEND_GRID(uint8_t *State, uint8_t *Grid, int fdout);

int SEND_MOVE(uint8_t CDL, int fdout);
int SEND_MOVEACK(uint8_t CDL, uint8_t Ok, int fdout);
int SEND_CONCEDE(int fdout);
int SEND_DISCON(int fdout);
//! SENDING_FUNCS Alloc for msg need a free

//! Util_func To test and make cleanner
void add_pseudo(uint8_t Plen, const char *pseudo, uint8_t *msg);
int pre_init_msg(uint8_t Type, uint8_t Len, Generic_tlv_t *to_init);

#endif // !TLV_PROGET_H