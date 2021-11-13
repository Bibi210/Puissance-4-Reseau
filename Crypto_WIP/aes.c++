#include "aes.hpp"
#include <array>
#include <bitset>
#include <climits>
#include <cstdlib>
#include <random>
#include <ratio>

//* Block to Cipher = 128 bits = 16 octets = 16 * sizeof(uint8_t)
// State = 4*4*uint8_t; key = 4*4*uint8_t

pair<bitset<4>, bitset<4>> Aes::break_uint8(uint8_t to_break) {
  bitset<8> base_nb = {to_break};
  bitset<8> tmp = {base_nb >> 4};
  bitset<4> p_droite = {base_nb.to_ulong()};
  bitset<4> p_gauche = {tmp.to_ullong()};
  // cout << hex << p_gauche.to_ulong() << p_droite.to_ulong();
  return make_pair(p_gauche, p_droite);
}

int index(int x, int y, int line_size) { return x + (y * line_size); }

template <typename T>
array<array<uint8_t, 4 * 4>, (sizeof(T) / 16) + 1> break_inblock(T tobreak) {
  array<array<uint8_t, 4 * 4>, (sizeof(T) / 16) + 1> output;
  for (size_t j = 0; j < output.size(); j++)
    output[j].fill(0);

  size_t i = 0, j = 0;
  uint8_t *bytes = (uint8_t *)&tobreak;
  for (size_t i = 0; i < sizeof(T); i++) {
    output[j][i % 16] = bytes[i];
    if (i % 16 == 0 && i)
      j++;
  }
  return output;
}

template <typename T>
T rebuild_from_block(
    array<array<uint8_t, 4 * 4>, (sizeof(T) / 16) + 1> tobuild) {
  int i = 0;
  uint8_t bytes[sizeof(T)];
  for (array<uint8_t, 16> block : tobuild) {
    for (uint8_t byte : block) {
      if (i < sizeof(T)) {
        bytes[i] = byte;
        i++;
      }
    }
  }
  return *(T *)bytes;
}

//! Might be my Aes Weakness (Side Channel atk ?)
uint8_t Aes::galoimul(uint8_t a, uint8_t b) {
  uint8_t output = 0;
  while (b && a) {          // a != 0 and b != 0
    if (b & 1)              // Si premier bit == 1
      output ^= a;          // Galoi +
    if (a & (256 >> 1)) {   // Si prochain depasse size of uint8_t
      a = (a << 1) ^ 0x11b; // On diminue avec 0x11b (Me souvient plus Pk ?
                            // 0x11n exactement)
    } else
      a <<= 1;
    b >>= 1; // B tant vers 0
  }
  return output;
}

void Aes::initialize_multiplytables() { //! Not used slow wrong implementati
  array<uint8_t, 256> In_Calcul;
  array<int, 7> multiplications = {1, 2, 3, 9, 11, 13, 14};
  int table;
  for (int t = 0; t < multiplications.size(); t++) {
    table = multiplications[t];
    for (size_t i = 0; i < In_Calcul.size(); i++) {
      In_Calcul[i] = galoimul(i, table);
    }
    precalculated_multiples[table] = In_Calcul;
  }
}

Aes::Aes() {
  initialize_aes_sbox();
  initialize_round_const();
  initialize_multiplytables();
  MixMatrix = {2, 3, 1, 1, 1, 2, 3, 1, 1, 1, 2, 3, 3, 1, 1, 2};
  Un_MixMatrix = {14, 11, 13, 9, 9, 14, 11, 13, 13, 9, 14, 11, 11, 13, 9, 14};
}
void Aes::initialize_round_const() {
  round_const[0] = 1;
  for (size_t i = 1; i < round_const.size(); i++) {
    round_const[i] = round_const[i - 1] << 1;
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
    *iter_out = os_seed() % 256;
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

void Aes::MixColumn(array<uint8_t, 4 * 4> &block, int nb_columns) {
  uint8_t tmp;
  array<uint8_t, 4 * 4> output = block;
  for (size_t i = 0; i < 4; i++) {
    tmp = 0;
    for (size_t j = 0; j < 4; j++) {
      tmp ^=
          galoimul(block[index(nb_columns, j, 4)], MixMatrix[index(j, i, 4)]);
    }
    output[index(nb_columns, i, 4)] = tmp;
  }
  block = output;
}
void Aes::MixColumns(array<uint8_t, 4 * 4> &block) {
  for (size_t i = 0; i < 4; i++) {
    MixColumn(block, i);
  }
}

void Aes::UnMixColumn(array<uint8_t, 4 * 4> &block, int nb_columns) {
  uint8_t tmp;
  array<uint8_t, 4 * 4> output = block;
  for (size_t i = 0; i < 4; i++) {
    block[index(nb_columns, i, 4)];
    tmp = 0;
    for (size_t j = 0; j < 4; j++) {
      tmp ^= (galoimul(block[index(nb_columns, j, 4)],
                       Un_MixMatrix[index(j, i, 4)]));
    }
    output[index(nb_columns, i, 4)] = tmp;
  }
  block = output;
}
void Aes::UnMixColumns(array<uint8_t, 4 * 4> &block) {
  for (size_t i = 0; i < 4; i++) {
    UnMixColumn(block, i);
  }
}

void Aes::AddRoundKey(array<uint8_t, 4 * 4> &block,
                      const array<uint8_t, 4 * 4> &roundkey) {
  for (size_t i = 0; i < block.size(); i++) {
    block[i] ^= roundkey[i];
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

void Aes::block_encrypt(array<uint8_t, 4 * 4> &block,
                        const array<array<uint8_t, 4 * 4>, 11> &expended_key) {
  AddRoundKey(block, expended_key[0]);
  for (size_t i = 1; i < 11; i++) {
    SubBlock(block);
    ShiftRows(block);
    if (i != 10)
      MixColumns(block);
    AddRoundKey(block, expended_key[i]);
  }
}

void Aes::block_decrypt(array<uint8_t, 4 * 4> &block,
                        const array<array<uint8_t, 4 * 4>, 11> &expended_key) {

  for (size_t i = 10; i >= 1; i--) {
    AddRoundKey(block, expended_key[i]);
    if (i != 10)
      UnMixColumns(block);
    UnShiftRows(block);
    UnSubBlock(block);
  }
  AddRoundKey(block, expended_key[0]);
}
Aes::~Aes() {}

struct B {
  const char *key;
};

int main(int argc, const char **argv) {
  // TODO : Encrypt blocks 2 modes
  // TODO : Decrypt blocks 2 modes
  // TODO : From block list to byte

  // TODO Cleanup code

  Aes Crypt = Aes();
  B t;
  t.key = string("Boom").data();
  auto blocks = break_inblock(t);

  B output = rebuild_from_block<B>(blocks);
  cout << output.key << endl;
  return 0;
}