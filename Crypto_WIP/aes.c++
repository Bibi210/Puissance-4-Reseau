#include "aes.hpp"
#include <_types/_uint8_t.h>
#include <random>

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

template <typename T> T Aes::fromblock(vector<block_t> blocks) {
  // TODO if used in future project verify if data is a pointer
  size_t i = 0;
  uint8_t *bytes = (uint8_t *)malloc(sizeof(uint8_t) * (blocks.size() * 16));
  for (block_t block : blocks) {
    for (uint8_t current_byte : block) {
      bytes[i] = current_byte;
      i++;
    }
  }
  if (typeid(char *) == typeid(T))
    return reinterpret_cast<T>(bytes);
  T out = *reinterpret_cast<T *>(bytes);
  free(bytes);
  return out;
}
template <typename T>
vector<Aes::block_t> Aes::inblock(T to_break, size_t inlen) {
  const uint8_t *in = reinterpret_cast<const uint8_t *>(to_break);
  vector<block_t> out;
  random_device os_seed;
  block_t current_block = {
      (uint8_t)(os_seed() % 256)}; // Padding with random data
  size_t i = 0;
  for (; i < inlen; i++) {
    if (i % 16 == 0 && i) {
      out.push_back(current_block);
      current_block = {(uint8_t)(os_seed() % 256)};
    }
    current_block[i % 16] = in[i];
  }
  if (i % 16 != 0)
    out.push_back(current_block);
  return out;
}
//! Might be my Aes Weakness (Side Channel atk ?)
uint8_t Aes::galoimul(uint8_t a, uint8_t b) {
  uint8_t output = 0;
  while (b && a) {          // a != 0 and b != 0
    if (b & 1)              // Si premier bit == 1
      output ^= a;          // Galoi +
    if (a & (256 >> 1))     // Si prochain depasse size of uint8_t
      a = (a << 1) ^ 0x11b; // On diminue avec 0x11b (Me souvient plus Pk ?
                            // 0x11n exactement)
    else
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
    for (size_t i = 0; i < In_Calcul.size(); i++)
      In_Calcul[i] = galoimul(i, table);

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
    if (round_const[i - 1] >= 0x80)
      round_const[i] ^= 0x11B;
  }
}
Aes::key_t Aes::gen_key() {
  key_t key_out;
  random_device os_seed;
  // TODO Read More Info About this
  // Safe on linuxOS based and windowsOS but not others
  for (uint8_t *iter_out = key_out.begin(); iter_out != key_out.end();
       iter_out++)
    *iter_out = os_seed() % 256;

  return key_out;
}

void Aes::DisplayState(const block_t &state) {
  for (size_t i = 0; i < 16; i++) {
    if (i % 4 == 0)
      cout << endl;

    cout << hex << break_uint8(state[i]).first.to_ulong()
         << break_uint8(state[i]).second.to_ulong() << " ";
  }
  cout << endl;
}

void Aes::DisplayBoxs() {
  cout << "Sbox :";
  for (size_t i = 0; i < 256; i++) {
    if (i % 16 == 0)
      cout << endl;

    cout << hex << break_uint8(sbox[i]).first.to_ulong()
         << break_uint8(sbox[i]).second.to_ulong() << " ";
  }
  cout << endl;

  cout << "Un_Sbox :";
  for (size_t i = 0; i < 256; i++) {
    if (i % 16 == 0)
      cout << endl;

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

void Aes::ShiftRow(block_t &word, int line, int nbshift) {
  int start_index = index(0, line, 4);
  int end_indice = index(4, line, 4);
  uint8_t start;
  while (nbshift % 4) {
    start = word[start_index];
    for (size_t i = start_index; i < end_indice; i++) {
      if (i + 1 == end_indice)
        word[i] = start;
      else
        word[i] = word[i + 1];
    }
    nbshift--;
  }
}

void Aes::ShiftRows(block_t &block) {
  for (size_t i = 0; i < 4 /* Line */; i++)
    ShiftRow(block, i, i);
}

void Aes::UnShiftRows(block_t &block) {
  for (size_t i = 0; i < 4 /* Line */; i++)
    ShiftRow(block, i, 4 - i);
}

void Aes::SubCase(uint8_t *Case) {
  pair<bitset<4>, bitset<4>> coords = break_uint8(*Case);
  *Case = sbox[coords.second.to_ulong() + (coords.first.to_ulong() * 16)];
}

void Aes::SubBlock(block_t &block) {
  for (size_t i = 0; i < block.size(); i++)
    SubCase(&block[i]);
}

void Aes::UnSubCase(uint8_t *Case) {
  pair<bitset<4>, bitset<4>> coords = break_uint8(*Case);
  *Case = un_sbox[coords.second.to_ulong() + (coords.first.to_ulong() * 16)];
}

void Aes::UnSubBlock(block_t &block) {
  for (size_t i = 0; i < block.size(); i++)
    UnSubCase(&block[i]);
}

void Aes::MixColumn(block_t &block, int nb_columns) {
  uint8_t tmp;
  block_t output = block;
  for (size_t i = 0; i < 4; i++) {
    tmp = 0;
    for (size_t j = 0; j < 4; j++)
      tmp ^=
          galoimul(block[index(nb_columns, j, 4)], MixMatrix[index(j, i, 4)]);

    output[index(nb_columns, i, 4)] = tmp;
  }
  block = output;
}
void Aes::MixColumns(block_t &block) {
  for (size_t i = 0; i < 4; i++)
    MixColumn(block, i);
}

void Aes::UnMixColumn(block_t &block, int nb_columns) {
  uint8_t tmp;
  block_t output = block;
  for (size_t i = 0; i < 4; i++) {
    block[index(nb_columns, i, 4)];
    tmp = 0;
    for (size_t j = 0; j < 4; j++)
      tmp ^= (galoimul(block[index(nb_columns, j, 4)],
                       Un_MixMatrix[index(j, i, 4)]));

    output[index(nb_columns, i, 4)] = tmp;
  }
  block = output;
}
void Aes::UnMixColumns(block_t &block) {
  for (size_t i = 0; i < 4; i++)
    UnMixColumn(block, i);
}

void Aes::AddRoundKey(block_t &block, const key_t &roundkey) {
  for (size_t i = 0; i < block.size(); i++)
    block[i] ^= roundkey[i];
}
array<Aes::key_t, 11> Aes::ExpendKey(const array<uint8_t, 4 * 4> &key) {
  array<key_t, 11> roundkeys;
  roundkeys[0] = key;
  key_t current_rkey;

  for (size_t i = 1; i < 11; i++) {
    current_rkey = roundkeys[i - 1];
    RotWord(current_rkey, 3);
    SubWord(current_rkey, 3);
    XorRecon(current_rkey, 3, current_rkey, 3, i);

    XorWord(current_rkey, 0, current_rkey, 0, current_rkey, 3);
    XorWord(current_rkey, 1, current_rkey, 1, current_rkey, 0);
    XorWord(current_rkey, 2, current_rkey, 2, current_rkey, 1);
    XorWord(current_rkey, 3, roundkeys[i - 1], 3, current_rkey, 2);
    roundkeys[i] = current_rkey;
  }

  return roundkeys;
}

void Aes::RotWord(key_t &key, int word_number) {
  int start_index = index(word_number, 0, 4);
  int end_indice = index(word_number, 3, 4);
  uint8_t start;
  start = key[start_index];

  for (size_t i = 0; i < 4; i++) {
    if (i + 1 == 4)
      key[index(word_number, i, 4)] = start;
    else
      key[index(word_number, i, 4)] = key[index(word_number, i + 1, 4)];
  }
}
void Aes::SubWord(key_t &key, int word_number) {
  for (size_t i = 0; i < 4; i++)
    SubCase(&key[index(word_number, i, 4)]);
}

void Aes::XorRecon(key_t &dst_key, int dst_word, key_t &src_key, int src_word,
                   int rc_turn) {
  dst_key[index(dst_word, 0, 4)] =
      src_key[index(src_word, 0, 4)] ^ round_const[rc_turn - 1];
}

void Aes::XorWord(key_t &dst_key, int dst_word, key_t &key_A, int nb_word_A,
                  key_t &key_B, int nb_word_B) {
  for (size_t i = 0; i < 4; i++)
    dst_key[index(dst_word, i, 4)] =
        key_A[index(nb_word_A, i, 4)] xor key_B[index(nb_word_B, i, 4)];
}

void Aes::block_encrypt(block_t &block, const array<key_t, 11> &expended_key) {
  AddRoundKey(block, expended_key[0]);
  for (size_t i = 1; i < 11; i++) {
    SubBlock(block);
    ShiftRows(block);
    if (i != 10)
      MixColumns(block);
    AddRoundKey(block, expended_key[i]);
  }
}

void Aes::block_decrypt(block_t &block, const array<key_t, 11> &expended_key) {
  for (size_t i = 10; i >= 1; i--) {
    AddRoundKey(block, expended_key[i]);
    if (i != 10)
      UnMixColumns(block);
    UnShiftRows(block);
    UnSubBlock(block);
  }
  AddRoundKey(block, expended_key[0]);
}
void Aes::Encrypt_ECB(vector<Aes::block_t> &blocks, const key_t &key) {
  const array<key_t, 11> keys = ExpendKey(key);
  for (size_t i = 0; i < blocks.size(); i++) {
    block_encrypt(blocks[i], keys);
  }
}

void Aes::Decrypt_ECB(vector<Aes::block_t> &blocks, const key_t &key) {
  const array<key_t, 11> keys = ExpendKey(key);
  for (size_t i = 0; i < blocks.size(); i++) {
    block_decrypt(blocks[i], keys);
  }
}

void Aes::Encrypt_CBC(vector<Aes::block_t> &blocks, const key_t &key) {
  const array<key_t, 11> keys = ExpendKey(key);
  block_t iv = gen_key();
  blocks.insert(blocks.begin(), iv);
  for (size_t i = 1; i < blocks.size(); i++) {
    for (size_t j = 0; j < iv.size(); j++) {
      blocks[i][j] ^= iv[j];
    }
    block_encrypt(blocks[i], keys);
    iv = blocks[i];
  }
}

void Aes::Decrypt_CBC(vector<Aes::block_t> &blocks, const key_t &key) {
  const array<key_t, 11> keys = ExpendKey(key);

  block_t iv = blocks[0];
  block_t prev;
  for (size_t i = 1; i < blocks.size(); i++) {
    prev = blocks[i];
    block_decrypt(blocks[i], keys);
    for (size_t j = 0; j < iv.size(); j++) {
      blocks[i][j] ^= iv[j];
    }
    iv = prev;
  }
  blocks.erase(blocks.begin());
}
template <typename T>
vector<Aes::block_t> Aes::Encrypt(T to_encrypt, const key_t &key, size_t size) {
  vector<block_t> blocks = inblock(to_encrypt, size);
  switch (Mode_Enc) {
  case ECB:
    Encrypt_ECB(blocks, key);
    break;
  case CBC:
    Encrypt_CBC(blocks, key);
    break;
  }
  return blocks;
}

template <typename T>
T Aes::Decrypt(vector<Aes::block_t> to_decrypt, const key_t &key, size_t size) {
  switch (Mode_Enc) {
  case ECB:
    Decrypt_ECB(to_decrypt, key);
    break;
  case CBC:
    Decrypt_CBC(to_decrypt, key);
    break;
  }
  return fromblock<T>(to_decrypt);
}
Aes::~Aes() {}

int main(int argc, const char **argv) {
  // TODO : Encrypt blocks 2 modes
  // TODO : Decrypt blocks 2 modes
  // TODO : From block list to byte

  // TODO Cleanup code

  Aes Crypt = Aes();
  Aes::key_t key = Crypt.gen_key();
  Crypt.Mode_Enc = Aes::ECB;

  string boom = "Je Suis un fan absolu de doctor who";
  long long crash = 6942;

  cout << "Clear : " << boom << endl;
  auto blocks = Crypt.Encrypt(boom.c_str(), key, boom.length());
  cout << "Cypher : " << Crypt.fromblock<char *>(blocks) << endl;
  char *out = Crypt.Decrypt<char *>(blocks, key);
  cout << "Decrypt : " << out << endl;
  free(out);

  cout << endl;

  cout << "Clear : " << crash << endl;
  blocks = Crypt.Encrypt(&crash, key);
  auto Cypher = Crypt.fromblock<long long>(blocks);
  cout << "Cypher : " << Cypher << endl;

  long long dec = Crypt.Decrypt<long long>(blocks, key);
  cout << "Decrypt : " << dec << endl;

  return 0;
}