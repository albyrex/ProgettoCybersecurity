
#ifndef __VERIFIER_H__
#define __VERIFIER_H__

/*
 * Ricontrollato 14-04 21:00
*/

#include <vector>
#include "common.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

using namespace std;

class Verifier {

private:
	EVP_MD_CTX* ctx;
	EVP_PKEY* pubK;
	const EVP_MD* digestAlg;


public:
	Verifier(EVP_PKEY* pubK);
	~Verifier();

	void updateVerification(const vector<byte>& source);
	bool finalizeVerification(const vector<byte>& sign);

	void updateVerification(const void* source, const size_t size);
	bool finalizeVerification(const void* sign, const size_t size);

public:
	static bool staticVerification(EVP_PKEY* pubK, const vector<byte>& source, const vector<byte>& sign);
	static bool staticVerification(EVP_PKEY* pubK, const void* source, const size_t source_size, const void* sign, const uint32_t signSize);

};


#endif
