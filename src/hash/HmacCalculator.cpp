#include "HmacCalculator.h"
#include "Utilities.h"

HmacCalculator::HmacCalculator(const byte* key, const uint32_t keySize) {
	this->keySize = keySize;
	this->ctx = HMAC_CTX_new();
	memcpy(this->key,key,keySize);
	HMAC_Init_ex(this->ctx, key, keySize, EVP_sha256(), NULL);
}

HmacCalculator::~HmacCalculator(){
	if(this->ctx != NULL)
		HMAC_CTX_free(this->ctx);
	Utilities::eraseMemory(key,keySize);
}

void HmacCalculator::updateHash(const vector<byte>& source) {
	HMAC_Update(this->ctx, source.data(), source.size());
}

void HmacCalculator::updateHash(const void* source, const uint32_t size) {
	if(size > INT_MAX)
		exit(1);
	HMAC_Update(this->ctx, (byte*)source, size);
}

vector<byte> HmacCalculator::getHash() {
	vector<byte> res(SHA256_HASH_SIZE);
	this->getHash(res.data());
	return res;
}

void HmacCalculator::getHash(void* dest) {
	uint32_t hashSize;
	HMAC_Final(this->ctx, (byte*)dest, &hashSize);
	if(hashSize != SHA256_HASH_SIZE){
		exit(1); /*da controllare quale situazione di errore gestire */
	}

	//Reset del contesto
	HMAC_CTX_free(this->ctx);
	this->ctx = HMAC_CTX_new();
	HMAC_Init_ex(this->ctx, this->key, keySize, EVP_sha256(), NULL);
}

vector<byte> HmacCalculator::staticHash(const byte* key, const void* source, const uint32_t size, const uint32_t keySize) {
	if(size > INT_MAX)
		exit(1);
	HMAC_CTX* ctx = HMAC_CTX_new();
	HMAC_Init_ex(ctx, key, keySize, EVP_sha256(), NULL);
	HMAC_Update(ctx, (byte*)source, size);
	vector<byte> res(SHA256_HASH_SIZE);
	uint32_t hashSize;
	HMAC_Final(ctx, res.data(), &hashSize);
	if(hashSize != SHA256_HASH_SIZE){
		exit(1);
	}
	HMAC_CTX_free(ctx);
	return res;
}

uint32_t HmacCalculator::staticHash(const byte* key, const void* source, const uint32_t size, void* out, const uint32_t keySize) {
	if(size > INT_MAX)
		exit(1);
	HMAC_CTX* ctx = HMAC_CTX_new();
	HMAC_Init_ex(ctx, key, keySize, EVP_sha256(), NULL);
	HMAC_Update(ctx, (byte*)source, size);
	vector<byte> res(SHA256_HASH_SIZE);
	uint32_t hashSize;
	HMAC_Final(ctx, (byte*)out, &hashSize);
	if(hashSize != SHA256_HASH_SIZE){
		exit(1);
	}
	HMAC_CTX_free(ctx);
	return hashSize;
}