#ifndef TLV_PROGET_H
#define TLV_PROGET_H

extern "C" {
#include "util_func.h"
#include <assert.h>
#include <bits/stdint-uintn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
}

#include "../Puiss4/p4.hpp"
#include <array>
#include <iostream>

using namespace std;

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
  LEN_GRID = (6 * 7) + 2, //! Maybe Change for better
  LEN_MOVE = 1,
  LEN_MOVEACK = 2,

  LEN_CONCEDE = LEN_INFO_MSG,
  LEN_DISCON = LEN_INFO_MSG,
};

typedef uint8_t *Body; //! Need Free

typedef struct Pseudo {
  uint8_t Size;
  string Nickname;
} Pseudo_t;
typedef struct Generic_tlv_t {

  uint8_t type;
  uint8_t length;
  Body msg;
} Generic_tlv_t;

typedef struct Start {
  uint8_t Pcolor;
  Pseudo_t Client;
  Pseudo_t Opponent;
} Start_t;

typedef struct Grid {
  uint8_t won_draw;
  uint8_t who;
  array<uint8_t, GRID_SIZE> Grid;
} Grid_t;

typedef uint8_t Move_t;

typedef struct Moveack {
  Move_t Col;
  bool Accepted;
} Moveack_t;

/// Envoie un TLV sur fdout
int send_tlv(Generic_tlv_t *to_send, int fdout);

/// Lit un TLV sur fdin *bloquant*
int read_tlv(Generic_tlv_t *to_read_into, int fdin);

/// Lis un tlv de fdin et l'envoie sur fdout *bloquant*
int retransmit_tlv(int fdin, int fdout);

/// Destroy TLV (free)
void destroy_tlv(Generic_tlv_t *to_destroy);

/// Mode Debug
void active_mode_debug(bool on_off);

bool get_debug_mode();

//! Only for debug purposes
void display_TLV_msg(Generic_tlv_t);

//! SENDING_FUNCS Alloc for msg free in send_tlv
int SEND_PSEUDO(Pseudo_t to_send, int fdout);
int SEND_PSEUDO(uint8_t Plen, const char *pseudo, int fdout);

/// Pcolor = 0 or 1
int SEND_START(uint8_t Pcolor, uint8_t Plen_A, const char *pseudo_A,
               uint8_t Plen_B, const char *pseudo_B, int fdout);
int SEND_START(Start_t to_send, int fdout);

/// State_size = 2 , Grid_size = 42
int SEND_GRID(uint8_t *State, uint8_t *Grid, int fdout);
int SEND_GRID(Grid_t to_send, int fdout);

int SEND_MOVE(uint8_t CDL, int fdout);
int SEND_MOVE(Move_t to_send, int fdout);

int SEND_MOVEACK(uint8_t CDL, uint8_t Ok, int fdout);
int SEND_MOVEACK(Moveack_t to_send, int fdout);

int SEND_CONCEDE(int fdout);
int SEND_DISCON(int fdout);
//! SENDING_FUNCS Alloc for msg free in send_tlv

//! READING_FUNCS Need a destroy call on to_read by Yourself
Pseudo_t READ_PSEUDO(Body To_read);
Start_t READ_START(Body To_read);
Grid_t READ_GRID(Body To_read);
Move_t READ_MOVE(Body To_read);
Moveack_t READ_MOVEACK(Body To_read);
//! READING_FUNCS Need a destroy call on to_read by Yourself

//! Util_func To test and make cleanner
void add_pseudo(uint8_t Plen, const char *pseudo, uint8_t *msg);
int pre_init_msg(uint8_t Type, uint8_t Len, Generic_tlv_t *to_init);

#endif // !TLV_PROGET_H
