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
 * allocation_rewiring.c
 *
 *  Created on: Oct 19, 2015
 *      Author: Felix Martin Schuhknecht
 */

#include "allocation_rewiring.h"

void allocationRewiring(wd_pt* const workingData,
						const size_t size,
						const bool hugePages,
						const bool randomMapping,
						const bool populate,
						measurement_t* const measurement) {

	// creating wdStandard_t
	wdStandard_t* newWorkingData = getWdStandard();
	*workingData = newWorkingData;

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;

	timeval_t start, end;
    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (pageSize - ((size * sizeof(entry_t)) % pageSize));
    const size_t numPages = sizeRoundedToPageBoundary / pageSize;

	// allocation of pool
    ERRNO_CHECK(
    filedescriptor_t fd = hugePages ? open("/mnt/hugetlbfs/allocationRewiring", O_RDWR | O_CREAT, 0666) : shm_open("/allocationRewiring", O_RDWR | O_CREAT, 0666);
    , hugePages ? "open" : "shm_open", true);
    ERRNO_CHECK(
    if(!hugePages) ftruncate(fd, sizeRoundedToPageBoundary);
    , "ftruncate src", true);

    // map a tmp vspace to the pool and initialize it
    ERRNO_CHECK(
    entry_t* pool = (entry_t*) mmap(NULL, sizeRoundedToPageBoundary, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    VALID_PTR(pool, -1, "mmap pool", false);
    , "mmap pool", true);
    memset(pool, 0, sizeRoundedToPageBoundary);

    printf("sizeRoundedToPageBoundary = %ld\n", sizeRoundedToPageBoundary);

    // map pages from the pool by a new vspace
    entry_t* dst = NULL;
    if(randomMapping) {
    	// random mapping: grab the pages randomly from the pool (simulates fragmentation, done by a mmap/page, basically the extreme case)
    	// compute page shuffling
    	size_t* pageIDs = malloc(sizeof(*pageIDs) * numPages);
    	for(size_t i = 0; i < numPages; ++i) pageIDs[i] = i;
    	shuffle(pageIDs, numPages);

        measure(&start);
    	// reserve vspace
    	ERRNO_CHECK(
        dst = (entry_t*) mmap(NULL, sizeRoundedToPageBoundary, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        VALID_PTR(newWorkingData->dst, -1, "mmap dst", false);
        , "mmap dst", true);
    	// map page-wise
    	for(size_t i = 0; i < numPages; ++i) {
    		char* pageToMap = ((char*) dst) + i * pageSize;
        	ERRNO_CHECK(
            mmap(pageToMap, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, pageIDs[i] * pageSize);
            VALID_PTR(pageToMap, -1, "mmap pageToMap", false);
            , "mmap pageToMap", true);
    	}
        measure(&end);
        printTimeDifference(&start, &end, MMAP_DST, measurement);
        free(pageIDs);
    }
    else {
    	// sequential mapping: grab the pages sequentially from the pool (can be done by a single mmap)
        measure(&start);
    	ERRNO_CHECK(
        dst = (entry_t*) mmap(NULL, sizeRoundedToPageBoundary, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        VALID_PTR(dst, -1, "mmap dst", false);
        , "mmap dst", true);
        measure(&end);
        printTimeDifference(&start, &end, MMAP_DST, measurement);
    }

    if(populate) {
    	volatile char tmp = ' ';
    	char* toPopulate = (char*) dst;
    	for(size_t i = 0; i < numPages; ++i) {
    		tmp = *toPopulate;
    		toPopulate += pageSize;
    	}
    	printf("%c", tmp);
    }

    printf("\n");

	ERRNO_CHECK(
    munmap(pool, sizeRoundedToPageBoundary);
	pool = NULL;
    , "munmap pool", true);

    setWdStandard(newWorkingData, NULL, dst, fd);
}

void allocationRewiringCleanup(wd_pt* const workingData,
								const size_t size,
								const bool hugePages) {
	notNull(workingData, "workingData");
	// expecting wdStandard_t
	wdStandard_t* wd = (wdStandard_t*) getWdStandardPtr(workingData);

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (pageSize - ((size * sizeof(entry_t)) % pageSize));
    printf("sizeRoundedToPageBoundary = %ld\n", sizeRoundedToPageBoundary);

	if(wd->dst) {
		ERRNO_CHECK(
		munmap(wd->dst, sizeRoundedToPageBoundary);
		wd->dst = NULL;
	    , "munmap dst", true);
        printf("Freeing %s\n", getFieldName(MMAP_DST));
	}

	if(hugePages) {
		printf("Unlink file (huge pages)\n");
		ERRNO_CHECK(
		close(wd->fd);
		wd->fd = -1;
		, "close", true);
		ERRNO_CHECK(
		unlink("/mnt/hugetlbfs/allocationRewiring");
		, "unlinck /mnt/hugetlbfs/allocationRewiring", true);
		printf("Freeing %s\n", getFieldName(SHM_SRC_HUGE));
	}
	else {
		ERRNO_CHECK(
		close(wd->fd);
		wd->fd = -1;
		, "close", true);
		printf("Unlink file (small pages)\n");
		ERRNO_CHECK(
		shm_unlink("/allocationRewiring");
		, "shm_unlick /allocationRewiring", true);
		printf("Freeing %s\n", getFieldName(SHM_SRC));
	}
}

