#ifndef AES_HPP
#define AES_HPP

#include <array>
#include <bitset>
#include <iostream>
#include <unordered_map>

using namespace std;

// Source : https://en.wikipedia.org/wiki/Rijndael_S-box
#define ROTL8(x, shift) ((uint8_t)((x) << (shift)) | ((x) >> (8 - (shift))))

class Aes {
  //TODO For an other life Use references to bytes in blocks
public:
  Aes();
  ~Aes();
  array<uint8_t, 4 * 4> gen_key();

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

  void SubBlock(array<uint8_t, 4 * 4> &block);
  void UnSubBlock(array<uint8_t, 4 * 4> &block);

  void SubCase(uint8_t *word);
  void UnSubCase(uint8_t *word);

  void ShiftRow(array<uint8_t, 4 * 4> &word, const int line, int nbshifts);
  void ShiftRows(array<uint8_t, 4 * 4> &block);
  void UnShiftRows(array<uint8_t, 4 * 4> &block);

  void MixColumn(array<uint8_t, 4 * 4> &block, int nb_columns);
  void MixColumns(array<uint8_t, 4 * 4> &block);

  void UnMixColumn(array<uint8_t, 4 * 4> &block, int nb_columns);
  void UnMixColumns(array<uint8_t, 4 * 4> &block);

  void AddRoundKey(array<uint8_t, 4 * 4> &block,
                   const array<uint8_t, 4 * 4> &roundkey);

  array<array<uint8_t, 4 * 4>, 11> ExpendKey(const array<uint8_t, 4 * 4> &key);
  void CpyWord(array<uint8_t, 4 * 4> &Src_word, array<uint8_t, 4 * 4> &Dst_word,
               int where_src, int where_dst);
  void RotWord(array<uint8_t, 4 * 4> &block, int nb);
  void SubWord(array<uint8_t, 4 * 4> &block, int nb);

  void XorWord(array<uint8_t, 4 * 4> &dst_block, int dst_word,
               array<uint8_t, 4 * 4> &block_A, int nb_word_A,
               array<uint8_t, 4 * 4> &block_B, int nb_word_B);
  void XorRecon(array<uint8_t, 4 * 4> &dst_block, int dst_word,
                array<uint8_t, 4 * 4> &block, int nb_word, int turn);

  void DisplayState(const array<uint8_t, 4 * 4> &block);
  void DisplayBoxs();

  uint8_t galoimul(uint8_t a, uint8_t b);
  pair<bitset<4>, bitset<4>> break_uint8(uint8_t to_break);

  void block_encrypt(array<uint8_t, 4 * 4> &block,
                     const array<array<uint8_t, 4 * 4>, 11> &expended_key);

  void block_decrypt(array<uint8_t, 4 * 4> &block,
                     const array<array<uint8_t, 4 * 4>, 11> &expended_key);
};
#endif // aes_hpp included
