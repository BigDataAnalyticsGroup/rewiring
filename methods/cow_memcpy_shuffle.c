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
 * cow_memcpy_shuffle.c
 *
 *  Created on: Feb 11, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "cow_memcpy_shuffle.h"

typedef void (*remap_ft)(entry_t** const,
						const filedescriptor_t* const,
						const size_t,
						const size_t,
						const size_t,
						const size_t*,
						const size_t,
						const bool,
						const bool,
						measurement_t* const);

void shuffleUsingMmap(entry_t** const dst,
						const filedescriptor_t* const fd,
						const size_t pagesPerChunk,
						const size_t sizeRoundedToPageBoundary,
						const size_t pageSize,
						const size_t* shuffling,
						const size_t numChunks,
						const bool populate,
						const bool hugePages,
						measurement_t* const measurement) {

	timeval_t start, end;

	// create the anonymous virtual memory area that we will remap below
    measure(&start);
	ERRNO_CHECK(
	*dst = (entry_t*) mmap(NULL,
	    				   sizeRoundedToPageBoundary,
						   PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_NORESERVE,
						   *fd,
						   0);
    VALID_PTR(*dst, -1, "mmap dst", false);
    , "mmap dst", true);
    measure(&end);
    printTimeDifference(&start, &end, MMAP_DST, measurement);

    // perform the remapping at chunk granularity
    const size_t bytesPerChunk = pagesPerChunk * pageSize;
    const size_t entriesPerPage = pageSize / sizeof(entry_t);
    const size_t entriesPerChunk = bytesPerChunk / sizeof(entry_t);
    measure(&start);
	volatile entry_t tmp = 0;
    for(size_t c = 0; c < numChunks; ++c) {
    	entry_t* const chunk = *dst + (entriesPerChunk * c);
    	const off_t offset = shuffling[c] * bytesPerChunk;
        ERRNO_CHECK(
    	mmap(chunk, bytesPerChunk, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, *fd, offset);
        , "remap dst", true);
        if(populate) {
        	for(size_t p = 0; p < pagesPerChunk; ++p) {
        		tmp = *(chunk + p * entriesPerPage);
        	}
        }
    }
    measure(&end);
    printTimeDifference(&start, &end, REMAP_DST, measurement);
    printf("%" PRIu64 "\n", tmp);
}

void shuffleUsingRemapFilePages(entry_t** const dst,
								const filedescriptor_t* const fd,
								const size_t pagesPerChunk,
								const size_t sizeRoundedToPageBoundary,
								const size_t pageSize,
								const size_t* shuffling,
								const size_t numChunks,
								const bool populate,
								const bool hugePages,
								measurement_t* const measurement) {

	timeval_t start, end;

	// create the anonymous virtual memory area that we will remap below
    measure(&start);
	ERRNO_CHECK(
	*dst = (entry_t*) mmap(NULL,
	    				   sizeRoundedToPageBoundary,
						   PROT_READ | PROT_WRITE,
						   MAP_SHARED,
						   *fd,
						   0);
    VALID_PTR(*dst, -1, "mmap dst", false);
    , "mmap dst", true);
    measure(&end);
    printTimeDifference(&start, &end, MMAP_DST, measurement);

    // perform the remapping at chunk granularity
    const size_t bytesPerChunk = pagesPerChunk * pageSize;
    const size_t entriesPerChunk = bytesPerChunk / sizeof(entry_t);
    measure(&start);
    for(size_t c = 0; c < numChunks; ++c) {
    	entry_t* const chunk = *dst + (entriesPerChunk * c);
    	const off_t offset = shuffling[c] * bytesPerChunk;
        ERRNO_CHECK(
        remap_file_pages(chunk, bytesPerChunk, 0, offset / pageSize, 0);
        , "remap dst", true);
    }

    measure(&end);
    printTimeDifference(&start, &end, REMAP_DST, measurement);
}




void cowMemcpyShuffle(wd_pt* const workingData,
									const size_t size,
									const size_t pagesPerChunk,
									const bool populate,
									const bool hugePages,
									const bool remapFilePages,
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
    ERRNO_CHECK(
    filedescriptor_t fd = hugePages ? open("/mnt/hugetlbfs/intcpy", O_RDWR | O_CREAT, 0666) : shm_open("/intcpy", O_RDWR | O_CREAT, 0666);
    , hugePages ? "open" : "shm_open", true);
    ERRNO_CHECK(
    if(!hugePages) ftruncate64(fd, sizeRoundedToPageBoundary);
    , "ftruncate src", true);
    measure(&end);
    printTimeDifference(&start, &end, SHM_SRC, measurement);

    measure(&start);
    ERRNO_CHECK(
    entry_t* src = (entry_t*) mmap(NULL, sizeRoundedToPageBoundary, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    VALID_PTR(src, -1, "mmap src", false);
    , "mmap src", true);
    measure(&end);
    printTimeDifference(&start, &end, MMAP_SRC, measurement);

    // initialization of src array
    measure(&start);
    entry_t* wrtPtr = src;
    for(size_t i = 0; i < size; i++) {
        wrtPtr[i] = i;
    }
    measure(&end);
    printTimeDifference(&start, &end, INIT_SHM_SRC, measurement);

    // create the shuffling
    size_t* shuffling = (size_t*) malloc(sizeof(size_t) * numChunks);
    for(size_t c = 0; c < numChunks; ++c) shuffling[c] = c;
    srand(SEED);
    shuffle(shuffling, numChunks);

    // create the anonymous virtual memory area that we will remap below
    entry_t* dst = NULL;
    remap_ft remap = remapFilePages ? &shuffleUsingRemapFilePages : &shuffleUsingMmap;
    (*remap)(&dst, &fd, pagesPerChunk, sizeRoundedToPageBoundary, pageSize, shuffling, numChunks, populate, hugePages, measurement);

    SAFE_FREE(shuffling);

    setWdStandard(newWorkingData, src, dst, fd);
    printf("\n");
}

