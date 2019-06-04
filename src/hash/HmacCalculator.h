#ifndef __HASHCALCULATOR_H__
#define __HASHCALCULATOR_H__

#include <vector>
#include <openssl/conf.h>
#include <openssl/hmac.h>
#include <string.h>

#include "common.h"

#define SHA256_HASH_SIZE 32
#define SHA256_KEY_SIZE  32

using namespace std;


class HmacCalculator{

	HMAC_CTX* ctx;
	byte key[SHA256_KEY_SIZE];
	uint32_t keySize;

public:
	HmacCalculator(const byte* key, const uint32_t keySize = SHA256_KEY_SIZE);
	~HmacCalculator();

	//Funzioni di update. Una con i vector ed una con i
	//puntatori
	void updateHash(const vector<byte>& source);
	void updateHash(const void* source, const uint32_t size);

	//Funzione che ritorna l'hash calcolato. Anche qui sia con
	//i vector che con i puntatori
	vector<byte> getHash();
	void getHash(void* dest);

public:
	//Funzione statica, usata per esempio per i messaggi
	static vector<byte> staticHash(const byte* key, const void* source, const uint32_t size, const uint32_t keySize = SHA256_KEY_SIZE);
	static uint32_t staticHash(const byte* key, const void* source, const uint32_t size, void* out, const uint32_t keySize = SHA256_KEY_SIZE);

};


#endif
