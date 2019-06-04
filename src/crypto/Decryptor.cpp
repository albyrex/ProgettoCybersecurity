#include "Decryptor.h"
#include "Utilities.h"
#include <string.h>
#include <iostream>

using namespace std;

Decryptor::Decryptor(const byte* key, const byte* iv) {
	plaintextLen = 0;
	ctx = EVP_CIPHER_CTX_new();
	memcpy(this->key, key, AES_128_KEY_SIZE);
	this->cypherAlg = EVP_aes_128_cbc();
	
	EVP_DecryptInit(ctx, this->cypherAlg, this->key, iv);
}

Decryptor::~Decryptor() {
	if(ctx)
		EVP_CIPHER_CTX_free(ctx);
	Utilities::eraseMemory(key,sizeof(key));
}

std::vector<byte> Decryptor::decrypt(const std::vector<byte>& source) {
	if(ctx == NULL)
		throw "Uninizializzato ctx";

	//Variabili di appoggio
	//  len -> la dimensione del decifrato di questo frammento
	uint32_t len;
	std::vector<byte> plaintext(source.size()+16);

	EVP_DecryptUpdate(ctx, plaintext.data(), (int*)&len, source.data(), source.size());
	plaintextLen += len;

	if(len < plaintext.size())
		plaintext.resize(len);

	return plaintext;
}

std::vector<byte> Decryptor::finalize() {
	if(ctx == NULL)
		throw "Uninizializzato ctx";

	uint32_t lastLen;
	std::vector<byte> plaintext(16);

	EVP_DecryptFinal(ctx, plaintext.data(), (int*)&lastLen);
	plaintextLen += lastLen;

	if(plaintext.size() > lastLen)
		plaintext.resize(lastLen);
	
	//Lo scopo di questa instanza è terminato
	EVP_CIPHER_CTX_free(ctx);
	ctx = NULL;

	return plaintext;
}

uint32_t Decryptor::getPlaintextLen() {
	return plaintextLen;
}

//====================================//

uint32_t Decryptor::staticDecrypt(const byte* key, const byte* iv, const void* source, const uint32_t sourceSize, void* dest) {
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit(ctx, EVP_aes_128_cbc(), key, iv);
	byte* tmpDest = new byte[sourceSize];

	uint32_t plaintextLen, lastLen;

	EVP_DecryptUpdate(ctx, tmpDest, (int*)&plaintextLen, (byte*)source, sourceSize);
	EVP_DecryptFinal(ctx, tmpDest + plaintextLen, (int*)&lastLen);

	plaintextLen += lastLen;

	memcpy(dest, tmpDest, plaintextLen);
	delete[] tmpDest;

	EVP_CIPHER_CTX_free(ctx);

	return plaintextLen;
}
