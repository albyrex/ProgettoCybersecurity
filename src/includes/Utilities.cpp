#include "Utilities.h"
#include <iostream>

#define __STDC_WANT_LIB_EXT1__ 1
#include <stdlib.h> 
#include <string.h>

using namespace std;

typedef unsigned char byte;

void Utilities::eraseMemory(void* addr, const uint64_t size) {
	#ifdef __STDC_LIB_EXT1__
	
	memset_s(pointer, size, 0, size);
	
	#else
	
	uint64_t size_ = size;
	volatile byte* p = (volatile byte*)addr;
	while(size_--) {
		*p++ = 0;
	}
	
	#endif
}

void Utilities::printVector(vector<byte>& v) {
  cout << std::hex;
  for(uint32_t i = 0; i < v.size(); ++i){
    if(v[i] <= 0x0F)
		cout << '0';
    cout << (uint32_t)v[i] << " ";
  }
  cout <<std::dec << "  (" << v.size() << ")\n";
}