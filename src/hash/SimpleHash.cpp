#include "SimpleHash.h"
#include "Utilities.h"

#define SHA256_HASH_SIZE 32

vector<byte> SimpleHash::hash(const byte* source, const uint32_t source_len) {
	EVP_MD_CTX *mdctx;
	if((mdctx = EVP_MD_CTX_create()) == NULL) {
		exit(1);
	}
	if(1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
		exit(1);
	}

	if(1 != EVP_DigestUpdate(mdctx, source, source_len)) {
		exit(1);
	}
	
	vector<byte> digest(256);
	uint32_t digest_len;

	if(1 != EVP_DigestFinal_ex(mdctx, digest.data(), &digest_len)) {
		exit(1);
	}
	digest.resize(digest_len);

	EVP_MD_CTX_destroy(mdctx);
	
	return digest;
}

uint32_t SimpleHash::hash(const byte* source, const uint32_t sourceSize, void* out) {
	EVP_MD_CTX *mdctx;
	
	if((mdctx = EVP_MD_CTX_create()) == NULL) {
		exit(1);
	}
	if(1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
		exit(1);
	}

	if(1 != EVP_DigestUpdate(mdctx, source, sourceSize)) {
		exit(1);
	}
	
	uint32_t digest_len;

	if(1 != EVP_DigestFinal_ex(mdctx, (byte*)out, &digest_len)) {
		exit(1);
	}

	EVP_MD_CTX_destroy(mdctx);
	
	return digest_len;
}