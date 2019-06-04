#ifndef __SIGNER_H__
#define __SIGNER_H__


#include <vector>
#include "common.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>

using namespace std;

class Signer {

private:
	EVP_MD_CTX* ctx;
	EVP_PKEY* privK;
	const EVP_MD* digestAlg;


public:
	Signer(EVP_PKEY* privK);
	~Signer();

	void updateSign(const vector<byte>& source);
	vector<byte> finalizeSign();

	void updateSign(const void* source, const size_t source_size);
	uint32_t finalizeSign(void* sign);

public:
	static vector<byte> staticSign(EVP_PKEY* privK, const vector<byte>& source);
	static uint32_t staticSign(void* dest, EVP_PKEY* privK, const void* source, const size_t source_size);
	
};


#endif