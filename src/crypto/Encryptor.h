#ifndef __ENCRYPTOR_H__
#define __ENCRYPTOR_H__


#include <vector>
#include "common.h"
#include "protocol.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#define AES_128_KEY_SIZE 16

using namespace std;

class Encryptor {

private:
	EVP_CIPHER_CTX* ctx;
	const EVP_CIPHER* cypherAlg;
	byte key[AES_128_KEY_SIZE];
	uint32_t ciphertextLen;

public:
	Encryptor(const byte* key, const byte* iv);
	~Encryptor();
	uint32_t getCiphertextLen();

	//Funzioni per cifrare cose in più passaggi (es. blocchi di file)
	std::vector<byte> encrypt(const std::vector<byte>& source);
	std::vector<byte> finalize();

public:
	static uint32_t staticEncrypt(const byte* key, const byte* iv, const void* source, const uint32_t sourceSize, void* dest);
	static vector<byte> staticEncrypt(const byte* key, const byte* iv, const void* source, const uint32_t sourceSize);

};


#endif
