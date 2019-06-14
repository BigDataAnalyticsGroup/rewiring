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
 * methods.h
 *
 *  Created on: Mar 06, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef METHODS_H_
#define METHODS_H_

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    TR_MEMCPY = 1,

	COW_MEMCPY = 2,
	COW_MEMCPY_POPULATE = 3,

	COW_MEMCPY_HUGE = 4,
	COW_MEMCPY_HUGE_POPULATE = 5,

	COW_MEMCPY_SHUFFLE = 6,
	COW_MEMCPY_SHUFFLE_POPULATE = 7,

	COW_MEMCPY_SHUFFLE_HUGE = 8,
	COW_MEMCPY_SHUFFLE_HUGE_POPULATE = 9,

	TR_MEMCPY_SHUFFLE = 10,
	TR_MEMCPY_SHUFFLE_HUGE = 11,

	TR_PARTITIONING_WITH_HISTO = 12,
	MAPPED_PARTITIONING = 13,
	MAPPED_PARTITIONING_HUGE = 14,

	COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES = 15,
	COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES_HUGE = 16,

	TR_PARTITIONING_BLOCK_CHAINS = 17,
	MAPPED_PARTITIONING_BLOCK_CHAINS = 18,
	MAPPED_PARTITIONING_BLOCK_CHAINS_HUGE = 19,

	COW_HUGE_SMALL_MIX = 20,

	MAPPED_PARTITIONING_BLOCK_CHAINS_ADAPTIVE = 21,

	REWIRED_VECTOR = 22,
	REWIRED_VECTOR_HUGE = 23,
	STL_VECTOR = 24,

	REWIRED_VECTOR_PREPOPULATE = 30,
	REWIRED_VECTOR_HUGE_PREPOPULATE = 31,

	MAP_AND_POPULATE = 32,

	MREMAPED_VECTOR = 36,
	MREMAPED_VECTOR_HUGE = 37,

	ALLOCATION_REWIRING = 40,
	ALLOCATION_SOFTWARE_INDIRECTION = 41,
	ALLOCATION_MMAP = 42,

	SOFTWARE_IND_VECTOR_HUGE = 43,

	MREMAPED_PARTITIONING = 44,

} copyMethod_t;
#define numCopyMethod 47

const char* getMethodName(const copyMethod_t copy_method);

#endif /* METHODS_H_ */
