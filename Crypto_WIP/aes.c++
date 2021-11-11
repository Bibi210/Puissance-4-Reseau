#include "aes.hpp"
#include <random>
#include <ratio>

//* Block to Cipher = 128 bits = 16 octets = 16 * sizeof(uint8_t)
// State = 4*4*uint8_t; key = 4*4*uint8_t

pair<bitset<4>, bitset<4>> break_uint8(uint8_t to_break) {
  bitset<8> base_nb = {to_break};
  bitset<8> tmp = {base_nb >> 4};
  bitset<4> p_droite = {base_nb.to_ulong()};
  bitset<4> p_gauche = {tmp.to_ullong()};
  // cout << hex << p_gauche.to_ulong() << p_droite.to_ulong();
  return make_pair(p_gauche, p_droite);
}

int index(int x, int y, int line_size) { return x + (y * line_size); }

Aes::Aes() {
  initialize_aes_sbox();
  initialize_round_const();
}
void Aes::initialize_round_const() {
  round_const[0] = 1;
  for (size_t i = 1; i < round_const.size(); i++) {
    round_const[i] = 2 * round_const[i - 1];
    if (round_const[i - 1] >= 0x80) {
      round_const[i] ^= 0x11B;
    }
  }
}
array<uint8_t, 4 * 4> Aes::gen_key() {
  array<uint8_t, 4 * 4> output;
  random_device os_seed;

  // TODO Read More Info About this
  // Safe on linuxOS based and windowsOS but not others
  for (uint8_t *iter_out = output.begin(); iter_out != output.end();
       iter_out++) {
    *iter_out = os_seed() % 255;
  }
  return output;
}

void Aes::DisplayState(const array<uint8_t, 4 * 4> &state) {
  for (size_t i = 0; i < 16; i++) {
    if (i % 4 == 0) {
      cout << endl;
    }
    cout << hex << break_uint8(state[i]).first.to_ulong()
         << break_uint8(state[i]).second.to_ulong() << " ";
  }
  cout << endl;
}

void Aes::DisplayBoxs() {
  cout << "Sbox :";
  for (size_t i = 0; i < 256; i++) {
    if (i % 16 == 0) {
      cout << endl;
    }
    cout << hex << break_uint8(sbox[i]).first.to_ulong()
         << break_uint8(sbox[i]).second.to_ulong() << " ";
  }
  cout << endl;

  cout << "Un_Sbox :";
  for (size_t i = 0; i < 256; i++) {
    if (i % 16 == 0) {
      cout << endl;
    }
    cout << hex << break_uint8(un_sbox[i]).first.to_ulong()
         << break_uint8(un_sbox[i]).second.to_ulong() << " ";
  }
  cout << endl;
}

void Aes::initialize_aes_sbox() {
  // Source : https://en.wikipedia.org/wiki/Rijndael_S-box
  uint8_t p = 1, q = 1;

  /* 0 is a special case since it has no inverse */
  sbox[0] = 0x63;
  un_sbox[0x63] = 0;
  /* loop invariant: p * q == 1 in the Galois field */
  do {
    /* multiply p by 3 */
    p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);

    /* divide q by 3 (equals multiplication by 0xf6) */
    q ^= q << 1;
    q ^= q << 2;
    q ^= q << 4;
    q ^= q & 0x80 ? 0x09 : 0;

    /* compute the affine transformation */
    uint8_t xformed = q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);

    sbox[p] = xformed ^ 0x63;
    un_sbox[xformed ^ 0x63] = p;
  } while (p != 1);
}

void Aes::ShiftRow(array<uint8_t, 4 * 4> &word, int line, int nbshift) {
  int start_index = index(0, line, 4);
  int end_indice = index(4, line, 4);
  uint8_t start;
  while (nbshift % 4) {
    start = word[start_index];
    for (size_t i = start_index; i < end_indice; i++) {
      if (i + 1 == end_indice) {
        word[i] = start;
      } else {
        word[i] = word[i + 1];
      }
    }
    nbshift--;
  }
}

void Aes::ShiftRows(array<uint8_t, 4 * 4> &block) {
  for (size_t i = 0; i < 4 /* Line */; i++) {
    ShiftRow(block, i, i);
  }
}

void Aes::UnShiftRows(array<uint8_t, 4 * 4> &block) {
  for (size_t i = 0; i < 4 /* Line */; i++) {
    ShiftRow(block, i, 4 - i);
  }
}

void Aes::SubCase(uint8_t *Case) {
  pair<bitset<4>, bitset<4>> coords = break_uint8(*Case);
  *Case = sbox[coords.second.to_ulong() + (coords.first.to_ulong() * 16)];
}

void Aes::SubBlock(array<uint8_t, 4 * 4> &block) {
  for (size_t i = 0; i < block.size(); i++) {
    SubCase(&block[i]);
  }
}

void Aes::UnSubCase(uint8_t *Case) {
  pair<bitset<4>, bitset<4>> coords = break_uint8(*Case);
  *Case = un_sbox[coords.second.to_ulong() + (coords.first.to_ulong() * 16)];
}

void Aes::UnSubBlock(array<uint8_t, 4 * 4> &block) {
  for (size_t i = 0; i < block.size(); i++) {
    UnSubCase(&block[i]);
  }
}

void Aes::AddRoundKey(array<uint8_t, 4 * 4> &block,
                      array<uint8_t, 4 * 4> &roundkey) {
  for (size_t i = 0; i < block.size(); i++) {
  }
}

array<array<uint8_t, 4 * 4>, 11>
Aes::ExpendKey(const array<uint8_t, 4 * 4> &key) {
  array<array<uint8_t, 4 * 4>, 11> roundkeys;
  roundkeys[0] = key;
  array<uint8_t, 4 * 4> currentround;

  for (size_t i = 1; i < 11; i++) {
    currentround = roundkeys[i - 1];
    RotWord(currentround, 3);
    SubWord(currentround, 3);
    XorRecon(currentround, 3, currentround, 3, i);

    XorWord(currentround, 0, currentround, 0, currentround, 3);
    XorWord(currentround, 1, currentround, 1, currentround, 0);
    XorWord(currentround, 2, currentround, 2, currentround, 1);
    XorWord(currentround, 3, roundkeys[i - 1], 3, currentround, 2);
    roundkeys[i] = currentround;
  }

  return roundkeys;
}

void Aes::RotWord(array<uint8_t, 4 * 4> &block, int word_number) {
  int start_index = index(word_number, 0, 4);
  int end_indice = index(word_number, 3, 4);
  uint8_t start;
  start = block[start_index];

  for (size_t i = 0; i < 4; i++) {
    if (i + 1 == 4) {
      block[index(word_number, i, 4)] = start;
    } else {
      block[index(word_number, i, 4)] = block[index(word_number, i + 1, 4)];
    }
  }
}
void Aes::SubWord(array<uint8_t, 4 * 4> &block, int word_number) {
  for (size_t i = 0; i < 4; i++) {
    SubCase(&block[index(word_number, i, 4)]);
  }
}
void Aes::CpyWord(array<uint8_t, 4 * 4> &Src_word,
                  array<uint8_t, 4 * 4> &Dst_word, int where_src,
                  int where_dst) {
  for (size_t i = 0; i < 4; i++) {
    Dst_word[index(where_dst, i, 4)] = Src_word[index(where_src, i, 4)];
  }
}

void Aes::XorRecon(array<uint8_t, 4 * 4> &dst_block, int dst_word,
                   array<uint8_t, 4 * 4> &block, int nb_word, int turn) {
  dst_block[index(dst_word, 0, 4)] =
      block[index(nb_word, 0, 4)] ^ round_const[turn - 1];
}

void Aes::XorWord(array<uint8_t, 4 * 4> &dst_block, int dst_word,
                  array<uint8_t, 4 * 4> &block_A, int nb_word_A,
                  array<uint8_t, 4 * 4> &block_B, int nb_word_B) {
  for (size_t i = 0; i < 4; i++) {
    dst_block[index(dst_word, i, 4)] =
        block_A[index(nb_word_A, i, 4)] xor block_B[index(nb_word_B, i, 4)];
  }
}
Aes::~Aes() {}

int main(int argc, const char **argv) {
  // TODO : (Un-Invert Column)
  Aes Crypt = Aes();
  array<uint8_t, 4 * 4> test = {0x2b, 0x28, 0xab, 0x09, 0x7e, 0xae, 0xf7, 0xcf,
                                0x15, 0xd2, 0x15, 0x4f, 0x16, 0xa6, 0x88, 0x3c};

  auto t = Crypt.ExpendKey(Crypt.gen_key());
  for (int i = 0; i < t.size(); i++) {
    cout << dec << "Key : " << i;
    Crypt.DisplayState(t[i]);
  }

  return 0;
}