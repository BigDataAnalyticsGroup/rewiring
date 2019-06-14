/*
 * Copyright 2016 Information Systems Group, Saarland University

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
		limitations under the License.

*/

/*
 * useful_functions.c
 *
 *  Created on: Feb 11, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "useful_functions.h"

void shuffle_varsize(void* const array, const size_t elemSize, const size_t n) {
    if(n > 1) {
    	uint8_t* a = (uint8_t*) array;
        uint8_t t[elemSize];
    	for(size_t i = 0; i < n - 1; ++i) {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);

          memcpy(t, a + (j * elemSize), elemSize);
          memcpy(a + (j * elemSize), a + (i * elemSize), elemSize);
          memcpy(a + (i * elemSize), t, elemSize);
        }
    }
}

void shuffle(size_t* const array, const size_t n) {
    shuffle_varsize(array, sizeof(entry_t), n);
}

uint64_t urand64() {
  uint64_t hi = lrand48();
  uint64_t md = lrand48();
  uint64_t lo = lrand48();
  return (hi << 42) + (md << 21) + lo;
}

uint32_t urand32() {
  uint32_t r = lrand48();
  uint32_t msb = lrand48() % 2;
  msb <<= 31;
  return r | msb;
}

inline void store_nontemp_64B(void* dst, void* src) {
    register __m256i * d1 = (__m256i*) dst;
    register __m256i s1 = *((__m256i*) src);
    register __m256i * d2 = d1+1;
    register __m256i s2 = *(((__m256i*) src)+1);

    _mm256_stream_si256(d1, s1);
    _mm256_stream_si256(d2, s2);
}

size_t log2partitions(const size_t numPartitions) {
	return (31 - __builtin_clz(numPartitions)) * (numPartitions ? 1 : 0);
}

bool isSorted(const entry_t* const p, const size_t n) {
	bool sorted = true;
	for(size_t i = 0; i < n - 1; ++i) {
		if(p[i] > p[i+1]) {
			DEBUG_PRINT("Error: (p[%ld]= %ld) > (p[%ld] = %ld)\n", i, p[i], i+1, p[i+1]);
			sorted = false;
		}
	}
	return sorted;
}

bool isSortedIndirect(entry_t** p, const size_t n) {
	bool sorted = true;
	for(size_t i = 0; i < n - 1; ++i) {
		if(*(p[i]) > *(p[i+1])) {
			DEBUG_PRINT("Error: (p[%ld]= %ld) > (p[%ld] = %ld)\n", i, *(p[i]), i+1, *(p[i+1]));
			sorted = false;
		}
	}
	return sorted;
}

bool safeFree(void** const ptr) {
	notNull(ptr, "ptr");
	if(*ptr) {
		free(*ptr);
		*ptr = NULL;
		return true;
	}
	return false;
}

void* prefaultPagesAsync(void* data) {
	prefaultPagesData_t* pD = (prefaultPagesData_t*) data;
	prefaultPages(pD->mem, pD->numBytes, pD->pageSize, pD->currentMem, pD->remappedMutex);
	free(pD);
	return NULL;
}

char prefaultPages(const char* mem,
						const size_t numBytes,
						const size_t pageSize,
						char** currentMem,
						pthread_mutex_t* remappedMutex) {

	volatile char tmp = 0;
	const size_t numPages = numBytes / pageSize;
	pthread_mutex_lock(remappedMutex);
	const char* localMem = mem;
	for(size_t p = 0; p < numPages; ++p) {
		if(mem != *currentMem) {
			pthread_mutex_unlock(remappedMutex);
			return tmp;
		}
		tmp = *localMem;
		localMem += pageSize;
	}
	pthread_mutex_unlock(remappedMutex);

	return tmp;
}

bool isPowerOfTwo (const size_t x) {
  return ((x != 0) && !(x & (x - 1)));
}
