#ifndef AES_HPP
#define AES_HPP

#include <_types/_uint8_t.h>
#include <array>
#include <bitset>
#include <cstddef>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

using namespace std;

// Source : https://en.wikipedia.org/wiki/Rijndael_S-box
#define ROTL8(x, shift) ((uint8_t)((x) << (shift)) | ((x) >> (8 - (shift))))

class Aes {

public:
  typedef array<uint8_t, 4 * 4> key_t;
  typedef array<uint8_t, 4 * 4> block_t;
  Aes();
  ~Aes();
  template <typename T>
  vector<Aes::block_t> Encrypt(T to_encrypt, const key_t &key,
                               size_t size = sizeof(T));

  template <typename T>
  T Decrypt(vector<Aes::block_t> to_decrypt, const key_t &key,
            size_t size = sizeof(T));

  key_t gen_key();
  enum Encrypt_Mode {
    ECB,
    CBC,
  };
  Encrypt_Mode Mode_Enc = CBC;
  template <typename T>
  vector<block_t> inblock(T to_break, size_t inlen = sizeof(T));
  template <typename T> T fromblock(vector<block_t> blocks);

private:
  array<uint8_t, 256> sbox;
  array<uint8_t, 256> un_sbox;
  array<int, 10> round_const;
  unordered_map<int, array<uint8_t, 256>>
      precalculated_multiples; //! Not used slow

  array<uint8_t, 4 * 4> MixMatrix;
  array<uint8_t, 4 * 4> Un_MixMatrix;

  void initialize_aes_sbox();
  void initialize_round_const();
  void initialize_multiplytables();

  void SubBlock(block_t &block);
  void UnSubBlock(block_t &block);

  void SubCase(uint8_t *word);
  void UnSubCase(uint8_t *word);

  void ShiftRow(block_t &word, const int line, int nbshifts);
  void ShiftRows(block_t &block);
  void UnShiftRows(block_t &block);

  void MixColumn(block_t &block, int nb_columns);
  void MixColumns(block_t &block);

  void UnMixColumn(block_t &block, int nb_columns);
  void UnMixColumns(block_t &block);

  void AddRoundKey(block_t &block, const key_t &roundkey);

  array<key_t, 11> ExpendKey(const key_t &key);
  void RotWord(key_t &key, int word_number);
  void SubWord(key_t &key, int word_number);
  void XorWord(key_t &dst_key, int dst_word, key_t &Key_A, int nb_word_A,
               key_t &Key_B, int nb_word_B);
  void XorRecon(key_t &dst_key, int dst_word, key_t &key, int nb_word,
                int turn);

  void DisplayState(const block_t &block);
  void DisplayBoxs();

  uint8_t galoimul(uint8_t a, uint8_t b);
  pair<bitset<4>, bitset<4>> break_uint8(uint8_t to_break);

  void block_encrypt(block_t &block, const array<key_t, 11> &expended_key);
  void block_decrypt(block_t &block, const array<key_t, 11> &expended_key);

  void Encrypt_ECB(vector<Aes::block_t> &blocks, const key_t &key);
  void Decrypt_ECB(vector<Aes::block_t> &blocks, const key_t &key);

  void Decrypt_CBC(vector<Aes::block_t> &blocks, const key_t &key);
  void Encrypt_CBC(vector<Aes::block_t> &blocks, const key_t &key);
};
#endif // aes_hpp included
