#ifndef __SIMPLEHASH_H__
#define __SIMPLEHASH_H__


#include <vector>
#include <openssl/hmac.h>
#include <string.h>

#include "common.h"

using namespace std;


class SimpleHash {

private:
	SimpleHash();

public:
	static vector<byte> hash(const byte* source, const uint32_t source_len);
	static uint32_t hash(const byte* source, const uint32_t sourceSize, void* out);

};


#endif
