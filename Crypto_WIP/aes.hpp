#ifndef AES_HPP
#define AES_HPP

#include <array>
#include <bitset>
#include <iostream>

using namespace std;

// Source : https://en.wikipedia.org/wiki/Rijndael_S-box
#define ROTL8(x, shift) ((uint8_t)((x) << (shift)) | ((x) >> (8 - (shift))))

class Aes {
public:
  Aes();
  ~Aes();
  array<uint8_t, 4 * 4> gen_key();
  array<uint8_t, 256> sbox;
  array<uint8_t, 256> un_sbox;
  array<int, 10> round_const;
  const static int blocksize = 4 * 4; // En Octet
  void initialize_aes_sbox();
  void initialize_round_const();

  void SubBlock(array<uint8_t, 4 * 4> &block);
  void UnSubBlock(array<uint8_t, 4 * 4> &block);

  void SubCase(uint8_t *word);
  void UnSubCase(uint8_t *word);

  void ShiftRow(array<uint8_t, 4 * 4> &word, const int line, int nbshifts);
  void ShiftRows(array<uint8_t, 4 * 4> &block);
  void UnShiftRows(array<uint8_t, 4 * 4> &block);

  void AddRoundKey(array<uint8_t, 4 * 4> &block,
                   array<uint8_t, 4 * 4> &roundkey);

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
};
#endif // aes_hpp included
