#include "Encryptor.h"
#include <string.h>
#include "Utilities.h"

Encryptor::Encryptor(const byte* key, const byte* iv) {
	ciphertextLen = 0;
	ctx = EVP_CIPHER_CTX_new();
	memcpy(this->key, key, AES_128_KEY_SIZE);
	this->cypherAlg = EVP_aes_128_cbc();

	EVP_EncryptInit(ctx, this->cypherAlg, key, iv);
}

Encryptor::~Encryptor() {
	if(ctx)
		EVP_CIPHER_CTX_free(ctx);
	Utilities::eraseMemory(key,sizeof(key));
}

std::vector<byte> Encryptor::encrypt(const std::vector<byte>& source) {
	if(ctx == NULL)
		throw "Uninizializzato ctx";

	//Variabili di appoggio:
	//  len -> la dimensione del cifrato di questo frammento
	uint32_t len;
	std::vector<byte> ciphertext(source.size());

	EVP_EncryptUpdate(ctx, ciphertext.data(), (int*)&len, source.data(), source.size());
	ciphertextLen += len;

	if(len < ciphertext.size())
		ciphertext.resize(len);

	return ciphertext;
}

std::vector<byte> Encryptor::finalize() {
	if(ctx == NULL)
		throw "Uninizializzato ctx";

	uint32_t paddingLen;
	std::vector<byte> ciphertext(16);

	EVP_EncryptFinal(ctx, ciphertext.data(), (int*)&paddingLen);
	ciphertextLen += paddingLen;

	if(ciphertext.size() > paddingLen)
		ciphertext.resize(paddingLen);
	
	//Lo scopo di questa instanza è terminato
	EVP_CIPHER_CTX_free(ctx);
	ctx = NULL;

	return ciphertext;
}

uint32_t Encryptor::getCiphertextLen() {
	return ciphertextLen;
}

//=====================================================//

uint32_t Encryptor::staticEncrypt(const byte* key, const byte* iv, const void* source, const uint32_t sourceSize, void* dest) {
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit(ctx, EVP_aes_128_cbc(), key, iv);

	uint32_t ciphertextLen, lastLen;

	EVP_EncryptUpdate(ctx, (byte*)dest, (int*)&ciphertextLen, (byte*)source, sourceSize);
	EVP_EncryptFinal(ctx, (byte*)dest + ciphertextLen, (int*)&lastLen);

	ciphertextLen += lastLen;

	EVP_CIPHER_CTX_free(ctx);

	return ciphertextLen;
}

vector<byte> Encryptor::staticEncrypt(const byte* key, const byte* iv, const void* source, const uint32_t sourceSize) {
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit(ctx, EVP_aes_128_cbc(), key, iv);

	uint32_t ciphertextLen, lastLen;

	vector<byte> dest(SIZE_AFTER_AES(sourceSize));
	
	EVP_EncryptUpdate(ctx, dest.data(), (int*)&ciphertextLen, (byte*)source, sourceSize);
	EVP_EncryptFinal(ctx, dest.data() + ciphertextLen, (int*)&lastLen);

	ciphertextLen += lastLen;

	EVP_CIPHER_CTX_free(ctx);

	return dest;
}
