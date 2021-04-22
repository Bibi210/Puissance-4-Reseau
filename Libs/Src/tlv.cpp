#include "../tlv.hpp"

int send_tlv(Generic_tlv_t *to_send, int fdout) {
  int error = 0;

  error = write_all(fdout, &to_send->type, sizeof(uint8_t));
  if (error < 0)
    return error;

  error = write_all(fdout, &to_send->length, sizeof(uint8_t));
  if (error < 0)
    return error;

  display_TLV_msg(*to_send);
  if (to_send->length) {
    error = write_all(fdout, to_send->msg, to_send->length);
    if (error < 0)
      return error;

    //! Free for msg alloc in read_tlv or SEND_X
    free(to_send->msg);
  }

  return EXIT_SUCCESS;
}

int read_tlv(Generic_tlv_t *to_read_into, int fdin) {
  int error = -1;

  error = read_all(fdin, &to_read_into->type, sizeof(uint8_t));

  if (error < 0)
    return error;

  error = read_all(fdin, &to_read_into->length, sizeof(uint8_t));
  if (error < 0)
    return error;

  if (to_read_into->length) {
    //! Alloc for msg need a free
    to_read_into->msg =
        (uint8_t *)calloc(to_read_into->length, sizeof(uint8_t));
    if (to_read_into->msg == NULL)
      return -1;
    error = read_all(fdin, to_read_into->msg, to_read_into->length);
    if (error < 0)
      return error;
  }
  display_TLV_msg(*to_read_into);
  return EXIT_SUCCESS;
}

int retransmit_tlv(int fdin, int fdout) {
  Generic_tlv_t bridge;
  int error = 0;

  error = read_tlv(&bridge, fdin);
  if (error < 0)
    return error;

  error = send_tlv(&bridge, fdout);
  if (error < 0)
    return error;

  return EXIT_SUCCESS;
}

void add_pseudo(uint8_t Plen, const char *pseudo, uint8_t *msg) {
  if (Plen > 64)
    Plen = 64;

  msg[0] = Plen;
  for (size_t i = 0; i < Plen; i++) {
    msg[i + 1] = (uint8_t)pseudo[i];
  }
  return;
}

int pre_init_msg(uint8_t Type, uint8_t Len, Generic_tlv_t *to_init) {
  to_init->type = Type;
  to_init->length = Len;

  if (to_init->length) {
    to_init->msg = (uint8_t *)calloc(to_init->length, sizeof(uint8_t));
    if (to_init->msg == NULL)
      return -1;
  }

  return EXIT_SUCCESS;
}

int SEND_PSEUDO(Pseudo_t to_send, int fdout) {
  return SEND_PSEUDO(to_send.Size, to_send.Nickname.c_str(), fdout);
}
int SEND_PSEUDO(uint8_t Plen, const char *pseudo, int fdout) {
  Generic_tlv_t to_send;

  if (pre_init_msg(TYPE_PSEUDO, LEN_PSEUDO, &to_send) < 0)
    return -1;
  add_pseudo(Plen, pseudo, to_send.msg);
  return send_tlv(&to_send, fdout);
}

int SEND_START(Start_t to_send, int fdout) {
  Pseudo_t Ally = to_send.Client;
  Pseudo_t Ennemy = to_send.Opponent;
  return SEND_START(to_send.Pcolor, Ally.Size, Ally.Nickname.c_str(),
                    Ennemy.Size, Ennemy.Nickname.c_str(), fdout);
}

int SEND_START(uint8_t Pcolor, uint8_t Plen_A, const char *pseudo_A,
               uint8_t Plen_B, const char *pseudo_B, int fdout) {
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_START, LEN_START, &to_send) < 0)
    return -1;
  to_send.msg[0] = Pcolor;

  add_pseudo(Plen_A, pseudo_A, to_send.msg + 1);
  add_pseudo(Plen_B, pseudo_B, to_send.msg + 1 + 1 + 64);

  return send_tlv(&to_send, fdout);
}

int SEND_GRID(Grid_t to_send, int fdout){
  uint8_t State[2];
  State[0] = to_send.won_draw;
  State[1] = to_send.who;
  uint8_t Grid[GRID_SIZE];
  for(int i = 0; i < GRID_SIZE; i++){
    Grid[i] = to_send.Grid[i];
  }
  return SEND_GRID(State,Grid,fdout);
}

int SEND_GRID(uint8_t *State, uint8_t *Grid, int fdout) {
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_GRID, LEN_GRID, &to_send) < 0)
    return -1;

  for (size_t i = 0; i < STATE_SIZE; i++)
    to_send.msg[i] = State[i];

  for (size_t i = 0; i < GRID_SIZE; i++)
    to_send.msg[i + STATE_SIZE] = Grid[i];

  return send_tlv(&to_send, fdout);
}

int SEND_MOVE(uint8_t CDL, int fdout) {
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_MOVE, LEN_MOVE, &to_send) < 0)
    return -1;

  to_send.msg[0] = CDL;
  return send_tlv(&to_send, fdout);
}


int SEND_MOVEACK(Moveack_t to_send,int fdout) {
  return SEND_MOVEACK(to_send.Col,to_send.Accepted,fdout);
}

int SEND_MOVEACK(uint8_t CDL, uint8_t Ok, int fdout) {
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_MOVEACK, LEN_MOVEACK, &to_send) < 0)
    return -1;
  to_send.msg[0] = CDL;
  to_send.msg[1] = Ok;

  return send_tlv(&to_send, fdout);
}

int SEND_CONCEDE(int fdout) {
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_CONCEDE, LEN_CONCEDE, &to_send) < 0)
    return -1;

  return send_tlv(&to_send, fdout);
}

int SEND_DISCON(int fdout) {
  Generic_tlv_t to_send;
  if (pre_init_msg(TYPE_DISCON, LEN_DISCON, &to_send) < 0)
    return -1;

  return send_tlv(&to_send, fdout);
}

Pseudo_t READ_PSEUDO(Body To_read) {
  //! Check if Nickname Survives
  Pseudo_t output;
  output.Size = To_read[0];
  for (size_t i = 1; i <= output.Size; i++) {
    output.Nickname.push_back(To_read[i]);
  }

  return output;
}

Start_t READ_START(Body To_read) {
  Start_t output;
  output.Pcolor = To_read[0];
  output.Client = READ_PSEUDO(To_read + 1);
  output.Opponent = READ_PSEUDO(To_read + 1 + 1 + 64);
  return output;
}

Grid_t READ_GRID(Body To_read) {
  Grid_t output;
  output.won_draw = To_read[0];
  output.who = To_read[1];
  for (int i = 0; i < LEN_GRID; i++) {
    output.Grid[i] = To_read[i + 2];
  }
  return output;
}

Move_t READ_MOVE(Body To_read) { return To_read[0]; }

Moveack_t READ_MOVEACK(Body To_read) {
  Moveack output;
  output.Accepted = false;
  output.Col = READ_MOVE(To_read);
  if (To_read[1])
    output.Accepted = true;
  return output;
}

void destroy_tlv(Generic_tlv_t *to_destroy) {
  if (to_destroy->length) {
    free(to_destroy->msg);
  }
}

void display_TLV_msg(Generic_tlv_t to_see) {
  cout << "Debug TLV" << endl;
  switch (to_see.type) {
  case TYPE_CONCEDE:
    cout << "TYPE_CONCEDE" << endl;
    break;
  case TYPE_DISCON:
    cout << "TYPE_DISCON" << endl;
    break;
  case TYPE_GRID: {
    Grid_t To_test = READ_GRID(to_see.msg);
    cout << "TYPE_GRID" << endl;
    cout << "Won : " << +To_test.won_draw << endl;
    cout << "Who : " << +To_test.who << endl;
    cout << "Grid :" << endl;
    cout << Puiss4::for_client(To_test.Grid) << endl;
    break;
  }
  case TYPE_MOVEACK: {
    Moveack_t to_test = READ_MOVEACK(to_see.msg);
    cout << "TYPE_MOVEACK" << endl;
    cout << boolalpha;
    cout << "Accepted ? " << to_test.Accepted << endl;
    cout << "Col : " << +to_test.Col << endl;
    break;
  }
  case TYPE_MOVE: {
    Move_t To_test = READ_MOVE(to_see.msg);
    cout << "TYPE_MOVE" << endl;
    cout << "Col : " << +To_test << endl;
    break;
  }
  case TYPE_PSEUDO: {
    Pseudo_t To_test = READ_PSEUDO(to_see.msg);
    cout << "TYPE_PSEUDO" << endl;
    cout << "Pseudo : " << To_test.Nickname << endl;
    cout << "Pseudo size : " << +To_test.Size << endl;
    break;
  }
  case TYPE_START: {
    Start_t To_test = READ_START(to_see.msg);
    cout << "TYPE_START" << endl;
    cout << "Color : " << +To_test.Pcolor << endl;
    cout << "Pseudo Ally :" << To_test.Client.Nickname << endl;
    cout << "Pseudo Ennemy :" << To_test.Opponent.Nickname << endl;
    break;
  }
  default:
    cout << "Unknown TLV" << endl;
    break;
  }
  cout << "Data Size: " << +to_see.length << endl;
  cout << endl;
  cout << endl;
  return;
}