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
 * rewird_vector.h
 *
 *  Created on: Mar 09, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef REWIRED_VECTOR_H_
#define REWIRED_VECTOR_H_

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include "../useful_functions.h"
#include "../types.h"
#include "../timediff.h"

rewiredVector_t* rvGetNewVector(const size_t initCapacity,				// the number of entries for which initially space is reserved
								const size_t poolCapacity,				// the number of entries of the pool
								const bool hugePages,					// use huge pages (2MB) or small pages (4KB)
								const bool doubleWhenFull,
								measurement_t* const measurement);	// when set to true, the size of the space is doubled when full
																		// when set to false, the space is increased page by page

// inlined
static inline void rvPushBack(rewiredVector_t* const rv, const entry_t* const entry) {
	if(rv->numEntries == rv->capacity) {
		// we ran out of space
		// depending on the strategy, double the space or allocate only a single page
		// try to mremap the virtual memory area

		// we double the capacity
		const size_t oldCapacityInPages = rv->capacity / rv->entriesPerPage;
		const size_t newCapacityInPages = oldCapacityInPages + (rv->doubleWhenFull ? oldCapacityInPages : 1);

	    // unmap old region
		ERRNO_CHECK(
		munmap(rv->vmem, oldCapacityInPages * rv->pageSize);
		rv->vmem = NULL;
		, "munmap rv->vmem", true);

		// create new region
		ERRNO_CHECK(
		rv->vmem = (entry_t*) mmap(NULL,
								   newCapacityInPages * rv->pageSize,
								   PROT_READ | PROT_WRITE,
								   MAP_SHARED,
								   rv->fd, 0);
		VALID_PTR(rv->vmem, -1, "mmap rv->vmem", false);
		, "mmap rv->vmem", true);

	    // update capacity
	    rv->capacity += rv->doubleWhenFull ? rv->capacity : rv->entriesPerPage;
	}

	// standard case, simply insert
	rv->vmem[rv->numEntries++] = *entry;
}

// returns the address of the inserted element (the vmem area might have changed due to resize)
entry_t* rvGetMem(const rewiredVector_t* const rv, size_t* numEntries);

void rvFreeVector(rewiredVector_t* const rv);



void rewiredVector(wd_pt* const workingData,
					const size_t initCapacity,
					const size_t poolCapacity,
					const bool hugePages,
					const bool doubleWhenFull,
					const bool prepopulate,
					measurement_t* const measurement);

void freeRewiredVector(wd_pt* const workingData);

#endif /* REWIRED_VECTOR_H_ */
