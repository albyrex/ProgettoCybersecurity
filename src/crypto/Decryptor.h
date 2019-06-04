#ifndef __DECRYPTOR_H__
#define __DECRYPTOR_H__


#include <vector>
#include "common.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#define AES_128_KEY_SIZE 16

using namespace std;

class Decryptor {

private:
	EVP_CIPHER_CTX* ctx;
	uint32_t plaintextLen;
	const EVP_CIPHER* cypherAlg;
	byte key[AES_128_KEY_SIZE];

public:
	Decryptor(const byte* key, const byte* iv);
	~Decryptor();
	uint32_t getPlaintextLen();

	//Funzioni per decifrare cose in più passaggi (es. blocchi di file)
	std::vector<byte> decrypt(const std::vector<byte>& source);
	std::vector<byte> finalize();


public:
	static uint32_t staticDecrypt(const byte* key, const byte* iv, const void* source, const uint32_t sourceSize, void* dest);

};


#endif
