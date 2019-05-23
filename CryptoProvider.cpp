#include "CryptoProvider.h"

void encipherBlock(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
	unsigned int i;
	uint32_t v0 = v[0], v1 = v[1], sum = 0, delta = 0x9E3779B9;
	for (i = 0; i < num_rounds; i++) {
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
		sum += delta;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
	}
	v[0] = v0; v[1] = v1;
}

void decipherBlock(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
	unsigned int i;
	uint32_t v0 = v[0], v1 = v[1], delta = 0x9E3779B9, sum = delta*num_rounds;
	for (i = 0; i < num_rounds; i++) {
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
	}
	v[0] = v0; v[1] = v1;
}

BYTE* Encrypt(BYTE* data, unsigned int size, uint32_t const key[4], unsigned int& OutSize)
{
	//append block if needed
	unsigned int rem = size % 8;
	unsigned int toAppend = 8 - rem;

	BYTE* enc = new BYTE[size+toAppend];

	memcpy(enc, data, size);

	for (unsigned int i = 0; i < toAppend; i++)
	{
		*(enc + size + i) = (BYTE)toAppend;
	}

	//encrypt block wise
	unsigned int blocks = (size + toAppend) / 8;

	for (unsigned int i = 0; i < blocks; i++)
	{
		encipherBlock(32, (uint32_t*)(enc + 8 * i), key);
	}

	OutSize = size+toAppend;

	delete data;

	return enc;
}

BYTE* Decrypt(BYTE* data, unsigned int size, uint32_t const key[4], unsigned int& OutSize)
{
	BYTE* dec = new BYTE[size];
	memcpy(dec, data, size);

	//decrypt block wise
	unsigned int blocks = size / 8;

	for (unsigned int i = 0; i < blocks; i++)
	{
		decipherBlock(32, (uint32_t*)(dec + 8 * i), key);
	}

	unsigned int toDelete = (unsigned int)dec[size - 1];
	OutSize = size - toDelete;

	delete data;

	return dec;
}