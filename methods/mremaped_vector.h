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
 * mremaped_vector.h
 *
 *  Created on: Sep 16, 2015
 *      Author: Felix Martin Schuhknecht
 */

#ifndef MREMAPED_VECTOR_H_
#define MREMAPED_VECTOR_H_

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "../useful_functions.h"
#include "../types.h"
#include "../timediff.h"

rewiredVector_t* mremapedvGetNewVector(const size_t initCapacity,		// the number of entries for which initially space is reserved
									const bool hugePages,				// use huge pages (2MB) or small pages(4KB)
									const bool doubleWhenFull,			// when set to true, the size of the space is doubled when full
																		// when set to false, the space is increased page by page
									measurement_t* const measurement);


// inlined
static inline void mremapedvPushBack(rewiredVector_t* const rv, const entry_t* const entry) {
	if(rv->numEntries == rv->capacity) {
		// we ran out of space
		// depending on the strategy, double the space or allocate only a single page

		// we double the capacity
		const size_t oldCapacityInPages = rv->capacity / rv->entriesPerPage;
		const size_t newCapacityInPages = oldCapacityInPages + (rv->doubleWhenFull ? oldCapacityInPages : 1);

		// create new region
		ERRNO_CHECK(
		rv->vmem = (entry_t*) mremap(rv->vmem,
									oldCapacityInPages * rv->pageSize,
									newCapacityInPages * rv->pageSize,
									MREMAP_MAYMOVE);
		VALID_PTR(rv->vmem, -1, "mremap rv->vmem", false);
		, "mremap rv->vmem", true);

	    // update capacity
	    rv->capacity += rv->doubleWhenFull ? rv->capacity : rv->entriesPerPage;
	}

	// standard case, simply insert
	rv->vmem[rv->numEntries++] = *entry;
}

// returns the address of the inserted element (the vmem area might have changed due to resize)
entry_t* mremapedvGetMem(const rewiredVector_t* const rv, size_t* numEntries);

void mremapedvFreeVector(rewiredVector_t* const rv);



void mremapedVector(wd_pt* const workingData,
					const size_t size,
					const bool hugePages,
					const bool doubleWhenFull,
					measurement_t* const measurement);

void freeMremapedVector(wd_pt* const workingData);

#endif /* MREMAPED_VECTOR_H_ */
