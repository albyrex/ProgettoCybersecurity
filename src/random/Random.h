#ifndef __RANDOM_H__
#define __RANDOM_H__


#include <stdint.h>

typedef unsigned char byte;

namespace Random {
	
	uint32_t getRandomUint32();
	
	uint64_t getRandomUint64();
	
	/*template <class T>
	T getRandom();*/
	
	void getRandomBytes(void* buffer, const uint32_t howManyBytes);
	
};


#endif