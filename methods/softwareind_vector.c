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
 *  softwareind_vector.c
 *
 *  Created on: Dec 2, 2015
 *      Author: Felix Martin Schuhknecht
 */

#include "softwareind_vector.h"

softwareIndVector_t* softwareIndGetNewVector(const size_t initCapacity,
									const size_t poolCapacity,
									const bool hugePages,
									const bool doubleWhenFull,
									measurement_t* const measurement) {

	timeval_t start, end;

	softwareIndVector_t* siv = NULL;
	posix_memalign((void**) &siv, CACHELINE_SIZE, sizeof(*siv));


	siv->doubleWhenFull = doubleWhenFull;
	siv->numEntries = 0;
	siv->currentDirSlot = 0;
	siv->currentPageSlot = 0;
	siv->pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
	siv->entriesPerPage = siv->pageSize / sizeof(entry_t);

	// compute the pool capacity rounded to pages
	const size_t poolCapacityInPages = (poolCapacity * sizeof(entry_t) / siv->pageSize) + ((poolCapacity * sizeof(entry_t)) % siv->pageSize ? 1 : 0);

	// compute the initial capacity rounded to pages
	const size_t capacityInPages = (initCapacity * sizeof(entry_t) / siv->pageSize) + ((initCapacity * sizeof(entry_t)) % siv->pageSize ? 1 : 0);
	siv->capacity = capacityInPages * siv->entriesPerPage;

	measure(&start);
	// allocate pool
    uint8_t* pool = NULL;
	posix_memalign((void**) &pool, HUGE_PAGE_SIZE, poolCapacityInPages * siv->pageSize);
	siv->pool = pool;

	// page fault pool
    uint8_t* pageStart = pool;
    for(size_t i = 0; i < poolCapacityInPages; ++i) {
    	*pageStart = 0;
    	pageStart += siv->pageSize;
    }
	measure(&end);
	printTimeDifference(&start, &end, SHM_SRC, measurement);

	measure(&start);
	// allocate directory
    entry_t** dir = NULL;
    posix_memalign((void**) &dir, CACHELINE_SIZE, capacityInPages * sizeof(*dir));

	// create initial mapping
    uint8_t* mem = (uint8_t*) pool;
    for(size_t i = 0; i < capacityInPages; ++i) {
    	dir[i] = (entry_t*) (mem + (i * siv->pageSize));
    }

    siv->dir = dir;
    siv->writePtr = dir[0];
	measure(&end);
	printTimeDifference(&start, &end, MMAP_SRC, measurement);

	return siv;
}

entry_t** softwareIndGetMem(const softwareIndVector_t* const siv, size_t* numEntries, size_t* pageSize) {
	if(numEntries) {
		*numEntries = siv->numEntries;
	}
	if(pageSize) {
		*pageSize = siv->pageSize;
	}
	return siv->dir;
}

void softwareIndFreeVector(softwareIndVector_t* const siv) {
	notNull(siv, "siv");

	SAFE_FREE(siv->dir);
	SAFE_FREE(siv->pool);
	SAFE_FREE(siv);
}

void softwareIndVector(wd_pt* const workingData,
					const size_t initCapacity,
					const size_t poolCapacity,
					const bool hugePages,
					const bool doubleWhenFull,
					measurement_t* const measurement) {

	notNull(workingData, "workingData");

	// creating wdSoftwareIndVector_t
	wdSoftwareIndVector_t* newWorkingData = getWdSoftwareIndVector();
	*workingData = newWorkingData;

	softwareIndVector_t* siv = softwareIndGetNewVector(initCapacity, poolCapacity, hugePages, doubleWhenFull, measurement);

	setWdSoftwareIndVector(newWorkingData, siv);
}

void freeSoftwareIndVector(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	// expecting wdSoftwareIndVector_t
	wdSoftwareIndVector_t* wd = (wdSoftwareIndVector_t*) getWdSoftwareIndVectorPtr(workingData);

	softwareIndFreeVector(wd->siv);
}














