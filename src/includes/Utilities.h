#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <stdint.h>
#include <vector>
#include "common.h"

namespace Utilities {
	
	void eraseMemory(void* addr, const uint64_t size);
	void printVector(std::vector<byte>& v);
	
};


#endif
