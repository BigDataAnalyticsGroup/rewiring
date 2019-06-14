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
 * allocation_softwaredir.c
 *
 *  Created on: Oct 19, 2015
 *      Author: Felix Martin Schuhknecht
 */

#include "allocation_softwaredir.h"

void allocationSoftwareIndirection(wd_pt* const workingData,
						const size_t size,
						const bool hugePages,
						const bool randomMapping,
						measurement_t* const measurement) {

	// creating wdSoftwareIndirection_t
	wdSoftwareIndirection_t* newWorkingData = getWdSoftwareIndirection();
	*workingData = newWorkingData;

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;

	timeval_t start, end;
    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (HUGE_PAGE_SIZE - ((size * sizeof(entry_t)) % HUGE_PAGE_SIZE));
    const size_t numSoftwarePages = sizeRoundedToPageBoundary / pageSize;

	// allocation of pool (in any case backed by huge pages)
    entry_t* pool = NULL;
	posix_memalign((void**)&pool, HUGE_PAGE_SIZE, sizeRoundedToPageBoundary);
    memset(pool, 0, sizeRoundedToPageBoundary);

    // map pages from the pool by a new vspace
    entry_t** dir = NULL;
    posix_memalign((void**)&dir, CACHELINE_SIZE, numSoftwarePages * sizeof(*dir));

    if(randomMapping) {
    	// random mapping: grab the pages randomly from the pool (simulates fragmentation, done by a mmap/page, basically the extreme case)
    	// compute page shuffling
    	size_t* pageIDs = malloc(sizeof(*pageIDs) * numSoftwarePages);
    	for(size_t i = 0; i < numSoftwarePages; ++i) pageIDs[i] = i;
    	shuffle(pageIDs, numSoftwarePages);

        measure(&start);
        char* mem = (char*) pool;
        for(size_t i = 0; i < numSoftwarePages; ++i) {
        	dir[i] = (entry_t*) (mem + (pageIDs[i] * pageSize));
        }
        measure(&end);
        printTimeDifference(&start, &end, MMAP_DST, measurement);
        free(pageIDs);
    }
    else {
    	// sequential mapping: grab the pages sequentially from the pool (can be done by a single mmap)
        measure(&start);
        char* mem = (char*) pool;
        for(size_t i = 0; i < numSoftwarePages; ++i) {
        	dir[i] = (entry_t*) (mem + (i * pageSize));
        }
        measure(&end);
        printTimeDifference(&start, &end, MMAP_DST, measurement);
    }

    printf("\n");

    setWdSoftwareIndirection(newWorkingData, dir, pool);
}

void allocationSoftwareIndirectionCleanup(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	// expecting wdSoftwareIndirection_t
	wdSoftwareIndirection_t* wd = (wdSoftwareIndirection_t*) getWdSoftwareIndirectionPtr(workingData);

	SAFE_FREE(wd->dir);
	SAFE_FREE(wd->pool);
}

