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
 *  rewired_vector.c
 *
 *  Created on: Mar 09, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "rewired_vector.h"

rewiredVector_t* rvGetNewVector(const size_t initCapacity,
								const size_t poolCapacity,
								const bool hugePages,
								const bool doubleWhenFull,
								measurement_t* const measurement) {

	timeval_t start, end;

	rewiredVector_t* rv = NULL;
	posix_memalign((void**) &rv, CACHELINE_SIZE, sizeof(*rv));

	rv->doubleWhenFull = doubleWhenFull;
	rv->numEntries = 0;

	rv->pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
	rv->entriesPerPage = rv->pageSize / sizeof(entry_t);

	// compute the pool capacity rounded to pages
	const size_t poolCapacityInPages = (poolCapacity * sizeof(entry_t) / rv->pageSize) + ((poolCapacity * sizeof(entry_t)) % rv->pageSize ? 1 : 0);

	// compute the initial capacity rounded to pages
	const size_t capacityInPages = (initCapacity * sizeof(entry_t) / rv->pageSize) + ((initCapacity * sizeof(entry_t)) % rv->pageSize ? 1 : 0);
	rv->capacity = capacityInPages * rv->entriesPerPage;

	DEBUG_PRINT("capacityInPages = %ld\n", capacityInPages);
	DEBUG_PRINT("rv->capacity = %ld\n", rv->capacity);

	// create the file for this vector
	const char* fileName = "vector";
	const char* pathToFilesystem = hugePages ? "/mnt/hugetlbfs/" : "/";
	rv->path = malloc(sizeof(*(rv->path)) * (strlen(pathToFilesystem) + strlen(fileName) + 1));
	strcpy(rv->path, pathToFilesystem);
	strcat(rv->path, fileName);

	measure(&start);
	if(hugePages) {
	    ERRNO_CHECK(
		rv->fd = open(rv->path, O_RDWR | O_CREAT, ACCESS_PERMISSION);
	    , "open", true);
	}
	else {
	    ERRNO_CHECK(
	    rv->fd = shm_open(rv->path, O_RDWR | O_CREAT, ACCESS_PERMISSION);
	    , "shm_open", true);
	    // truncate to pool size
	    ftruncate(rv->fd, poolCapacityInPages * rv->pageSize);
	}

	// initialize pool
    ERRNO_CHECK(
    uint8_t* pool = (uint8_t*) mmap(NULL,
    						  	  	poolCapacityInPages * rv->pageSize,
									PROT_READ | PROT_WRITE,
									MAP_SHARED,
									rv->fd, 0);
    VALID_PTR(pool, -1, "mmap pool", false);
    , "mmap pool", true);

    uint8_t* pageStart = pool;
    for(size_t i = 0; i < poolCapacityInPages; ++i) {
    	*pageStart = 0;
    	pageStart += rv->pageSize;
    }

	ERRNO_CHECK(
	munmap(pool, poolCapacityInPages * rv->pageSize);
	pool = NULL;
	, "munmap pool", true);

	measure(&end);
	printTimeDifference(&start, &end, SHM_SRC, measurement);


	measure(&start);
	// create initial mapping
    ERRNO_CHECK(
    rv->vmem = (entry_t*) mmap(NULL,
    						  capacityInPages * rv->pageSize,
							  PROT_READ | PROT_WRITE,
							  MAP_SHARED,
							  rv->fd, 0);
    VALID_PTR(rv->vmem, -1, "mmap rv->vmem", false);
    , "mmap rv->vmem", true);
	measure(&end);
	printTimeDifference(&start, &end, MMAP_SRC, measurement);

	return rv;
}

entry_t* rvGetMem(const rewiredVector_t* const rv, size_t* numEntries) {
	if(numEntries) {
		*numEntries = rv->numEntries;
	}
	return rv->vmem;
}

void rvFreeVector(rewiredVector_t* const rv) {

	ERRNO_CHECK(
	munmap(rv->vmem, rv->capacity / rv->entriesPerPage * rv->pageSize);
	rv->vmem = NULL;
	, "munmap rv->vmem", true);

	if(rv->pageSize == HUGE_PAGE_SIZE) {
		ERRNO_CHECK(
		close(rv->fd);
		rv->fd = -1;
		, "close", true);
		ERRNO_CHECK(
		unlink(rv->path);
		, "unlinck path", true);
	}
	else {
	    ERRNO_CHECK(
	    shm_unlink(rv->path);
	    , "shm_unlick path", true);
	}
	SAFE_FREE(rv->path);
	SAFE_FREE(rv);
}

void rewiredVector(wd_pt* const workingData,
					const size_t initCapacity,
					const size_t poolCapacity,
					const bool hugePages,
					const bool doubleWhenFull,
					const bool prepopulate,
					measurement_t* const measurement) {

	notNull(workingData, "workingData");

	// creating wdRewiredVector_t
	wdRewiredVector_t* newWorkingData = getWdRewiredVector();
	*workingData = newWorkingData;

	rewiredVector_t* rv = rvGetNewVector(initCapacity, poolCapacity, hugePages, doubleWhenFull, measurement);

	setWdRewiredVector(newWorkingData, rv);
}

void freeRewiredVector(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	// expecting wdRewiredVector_t
	wdRewiredVector_t* wd = (wdRewiredVector_t*) getWdRewiredVectorPtr(workingData);

	rvFreeVector(wd->rv);
}














