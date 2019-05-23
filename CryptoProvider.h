#pragma once
#include <Windows.h>
#include <stdint.h>

//XTEA
/* take 64 bits of data in v[0] and v[1] and 128 bits of key[0] - key[3] */
void encipherBlock(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);
void decipherBlock(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);

BYTE* Encrypt(BYTE* data, unsigned int size, uint32_t const key[4], unsigned int& OutSize);
BYTE* Decrypt(BYTE* data, unsigned int size, uint32_t const key[4], unsigned int& OutSize);