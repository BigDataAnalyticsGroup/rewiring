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
 * memcpy_shuffle.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "memcpy_shuffle.h"

void traditionalMemcpyShuffle(wd_pt* const workingData,
								const size_t size,
								const size_t pagesPerChunk,
								const bool hugePages,
								const bool initialized,
								measurement_t* const measurement) {
	// creating wdStandard_t
	wdStandard_t* newWorkingData = getWdStandard();
	*workingData = newWorkingData;

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;

	timeval_t start, end;
    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (pageSize - ((size * sizeof(entry_t)) % pageSize));
    const size_t numPages = sizeRoundedToPageBoundary / pageSize;
    const size_t numChunks = (numPages - (numPages % pagesPerChunk != 0 ? 1 : 0)) / pagesPerChunk;

    // allocation of src array
    measure(&start);
    entry_t* src = (entry_t*) malloc(sizeRoundedToPageBoundary);
    measure(&end);
    printTimeDifference(&start, &end, MALLOC_SRC, measurement);

    // initialization of src array
    measure(&start);
    entry_t* wrtPtr = src;
    for(size_t i = 0; i < size; i++) {
        wrtPtr[i] = i;
    }
    measure(&end);
    printTimeDifference(&start, &end, INIT_MALLOC_SRC, measurement);

    // create the shuffling
    size_t* shuffling = (size_t*) malloc(sizeof(size_t) * numChunks);
    for(size_t c = 0; c < numChunks; ++c) shuffling[c] = c;
    srand(SEED);
    shuffle(shuffling, numChunks);

    // allocation of dst array
    measure(&start);
    entry_t* dst = (entry_t*) malloc(sizeRoundedToPageBoundary);
    measure(&end);
    printTimeDifference(&start, &end, MALLOC_DST, measurement);

    // initialize dst if requested
    if(initialized) {
    	memset(dst, 0, sizeRoundedToPageBoundary);
    }

    // copying src to dst array in chunks accordingly to shuffling
    const size_t bytesPerChunk = pagesPerChunk * pageSize;
    const size_t entriesPerChunk = bytesPerChunk / sizeof(entry_t);
    measure(&start);
    for(size_t c = 0; c < numChunks; ++c) {
    	entry_t* const chunk = dst + (entriesPerChunk * c);
    	const size_t offset = shuffling[c] * entriesPerChunk;
    	memcpy(chunk, src + offset, bytesPerChunk);
    }
    measure(&end);
    printTimeDifference(&start, &end, MEMCPY, measurement);

    SAFE_FREE(shuffling);

    printf("\n");

    setWdStandard(newWorkingData, src, dst, -1);
}



