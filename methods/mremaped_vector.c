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
 *  mremaped_vector.c
 *
 *  Created on: Sep 16, 2015
 *      Author: Felix Martin Schuhknecht
 */

#include "mremaped_vector.h"

rewiredVector_t* mremapedvGetNewVector(const size_t initCapacity,
									const bool hugePages,
									const bool doubleWhenFull,
									measurement_t* const measurement) {

	timeval_t start, end;

	// use the rewired vector struct here too, because of the similarity
	rewiredVector_t* rv = NULL;
	posix_memalign((void**) &rv, CACHELINE_SIZE, sizeof(*rv));

	rv->doubleWhenFull = doubleWhenFull;
	rv->numEntries = 0;
	rv->pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
	rv->entriesPerPage = rv->pageSize / sizeof(entry_t);
	rv->fd = -1;

	// compute the initial capacity rounded to pages
	const size_t capacityInPages = (initCapacity * sizeof(entry_t) / rv->pageSize) + ((initCapacity * sizeof(entry_t)) % rv->pageSize ? 1 : 0);
	rv->capacity = capacityInPages * rv->entriesPerPage;
	DEBUG_PRINT("capacityInPages = %ld\n", capacityInPages);
	DEBUG_PRINT("rv->capacity = %ld\n", rv->capacity);

	measure(&start);
	// create initial mapping to anonymous
    ERRNO_CHECK(
    rv->vmem = (entry_t*) mmap(NULL,
    						  capacityInPages * rv->pageSize,
							  PROT_READ | PROT_WRITE,
							  MAP_PRIVATE | MAP_ANONYMOUS,
							  -1, 0);
    VALID_PTR(rv->vmem, -1, "mmap rv->vmem", false);
    , "mmap rv->vmem", true);
	measure(&end);
	printTimeDifference(&start, &end, MMAP_SRC, measurement);

	return rv;
}

entry_t* mremapedvGetMem(const rewiredVector_t* const rv, size_t* numEntries) {
	if(numEntries) {
		*numEntries = rv->numEntries;
	}
	return rv->vmem;
}

void mremapedvFreeVector(rewiredVector_t* const rv) {

	ERRNO_CHECK(
	munmap(rv->vmem, rv->capacity / rv->entriesPerPage * rv->pageSize);
	rv->vmem = NULL;
	, "munmap rv->vmem", true);

	SAFE_FREE(rv);
}

void mremapedVector(wd_pt* const workingData,
					const size_t size,
					const bool hugePages,
					const bool doubleWhenFull,
					measurement_t* const measurement) {

	notNull(workingData, "workingData");

	// creating wdRewiredVector_t
	wdRewiredVector_t* newWorkingData = getWdRewiredVector();
	*workingData = newWorkingData;

	rewiredVector_t* rv = mremapedvGetNewVector(size, hugePages, doubleWhenFull, measurement);

	setWdRewiredVector(newWorkingData, rv);
}

void freeMremapedVector(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	// expecting wdRewiredVector_t
	wdRewiredVector_t* wd = (wdRewiredVector_t*) getWdRewiredVectorPtr(workingData);

	mremapedvFreeVector(wd->rv);
}














