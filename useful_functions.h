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
 * useful_functions.h
 *
 *  Created on: Feb 11, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef USEFUL_FUNCTIONS_H_
#define USEFUL_FUNCTIONS_H_

#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>
#include <pthread.h>
#include "types.h"
#include "error.h"

void shuffle(size_t* const array, const size_t n);
void shuffle_varsize(void* const array, const size_t elemSize, const size_t n);
uint64_t urand64();
uint32_t urand32();
void store_nontemp_64B(void* dst, void* src);
size_t log2partitions(const size_t numPartitions);
bool isSorted(const entry_t* const p, const size_t n);
bool isSortedIndirect(entry_t** p, const size_t n);
bool safeFree(void** const ptr);

// reads the first byte of each page to trigger a page fault
// mem must be page aligned
// numBytes must be a multiple of the page size

typedef struct {
	char* mem;
	size_t numBytes;
	size_t pageSize;
	char** currentMem;
	pthread_mutex_t* remappedMutex;
} prefaultPagesData_t;

void* prefaultPagesAsync(void* data);

char prefaultPages(const char* mem,
						const size_t numBytes,
						const size_t pageSize,
						char** currentMem,
						pthread_mutex_t* remappedMutex);

#define SAFE_FREE(p) safeFree((void** const) &(p))
#define GET_MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define GET_BUCKET(VALUE, BITS) (VALUE >> BITS)
#define GET_BUCKET_MASKED(VALUE, BITS, OFFSET) ((VALUE << OFFSET) >> BITS)
#define ALIGN_NUMTUPLES(N) ((N + TUPLES_PER_CACHELINE - 1) & ~(TUPLES_PER_CACHELINE - 1))

bool isPowerOfTwo (const size_t x);

#endif /* USEFUL_FUNCTIONS_H_ */
