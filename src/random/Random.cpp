#include "Random.h"
#include <climits>
#include <openssl/rand.h>

uint32_t Random::getRandomUint32() {
	uint32_t r;
	int success;
	do{
		success = RAND_bytes((byte*)&r, sizeof(r));
	}while(success != 1);
	return r;
}

uint64_t Random::getRandomUint64() {
	uint64_t r;
	int success;
	do{
		success = RAND_bytes((byte*)&r, sizeof(r));
	}while(success != 1);
	return r;
}

/*template <class T>
T Random::getRandom() {
	T r;
	int success;
	do{
		success = RAND_bytes((byte*)&r, sizeof(r));
	}while(success != 1);
	return r;
}*/

void Random::getRandomBytes(void* buffer, const uint32_t howManyBytes) {
	if(howManyBytes > INT_MAX)
		exit(1);
	int success;
	do{
		success = RAND_bytes((byte*)buffer, howManyBytes);
	}while(success != 1);
}