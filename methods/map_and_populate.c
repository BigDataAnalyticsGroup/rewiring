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
 * map_and_populate.c
 *
 *  Created on: Jul 17, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "map_and_populate.h"

void mapAndPopulate(wd_pt* const workingData,
					const size_t size,
					const bool hugePages,					// otherwise small pages
					const bool file,						// otherwise anonymous
					const bool shared,						// otherwise private mapping
					const bool populate,					// populate at all?
					const bool populateByOption,			// otherwise asynchronous prefaulting
					const bool initializeFile,				// otherwise fresh file
					const bool randomShuffle,				// otherwise one sequential mapping
					measurement_t* const measurement) {
	// creating wdStandard_t
	wdStandard_t* newWorkingData = getWdStandard();
	*workingData = newWorkingData;

	printf("Settings: hugePages = %d, file = %d, shared = %d, populate = %d, populateByOption = %d, initializeFile = %d, randomShuffle = %d\n",
			hugePages, file, shared, populate, populateByOption, initializeFile, randomShuffle);
	// support random shuffle only if file is used
	assert(!(randomShuffle && !file));

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;

	timeval_t start, end;
    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (pageSize - ((size * sizeof(entry_t)) % pageSize));

	// create file if file mapping requested
    filedescriptor_t fd = -1;
    if(file) {
		measure(&start);
		ERRNO_CHECK(
		fd = hugePages ? open("/mnt/hugetlbfs/mapAndPopulate", O_RDWR | O_CREAT, 0666) : shm_open("/mapAndPopulate", O_RDWR | O_CREAT, 0666);
		, "shm_open", true);
		if(!hugePages) {
			ERRNO_CHECK(
			ftruncate(fd, sizeRoundedToPageBoundary);
			, "ftruncate src", true);
		}
		measure(&end);
		printTimeDifference(&start, &end, SHM_SRC, measurement);
    }

    if(initializeFile) {
    	assert(file);
        ERRNO_CHECK(
        char* tmp = (char*) mmap(NULL,
        							sizeRoundedToPageBoundary,
    								PROT_READ | PROT_WRITE,
    								MAP_SHARED,
    								fd,
    								0);
        VALID_PTR(tmp, -1, "mmap tmp", false);
        , "mmap tmp", true);

        memset(tmp, 0, sizeRoundedToPageBoundary);

		ERRNO_CHECK(
	    munmap(tmp, sizeRoundedToPageBoundary);
	    tmp = NULL;
	    , "munmap tmp", true);
    }

    // create mapping
    entry_t* src = NULL;
    if(randomShuffle) {
    	// create shuffling
    	const size_t numPages = sizeRoundedToPageBoundary / pageSize;
    	const size_t entriesPerPage = pageSize / sizeof(entry_t);
    	size_t* shuffling = malloc(sizeof(*shuffling) * numPages);
    	for(size_t i = 0; i < numPages; ++i) {
    		shuffling[i] = i;
    	}
    	shuffle(shuffling, numPages);

        measure(&start);
    	// reserve virtual memory area
        ERRNO_CHECK(
        src = (entry_t*) mmap(NULL,
        						sizeRoundedToPageBoundary,
    							PROT_READ | PROT_WRITE,
								  (shared ? MAP_SHARED : MAP_PRIVATE),
    							fd,
    							0);
        VALID_PTR(src, -1, "mmap src", false);
        , "mmap src", true);

        // remap
        for(size_t i = 0; i < numPages; ++i) {
        	entry_t* pageToRemap = src + (i * entriesPerPage);
            ERRNO_CHECK(
            mmap(pageToRemap,
            		pageSize,
        			PROT_READ | PROT_WRITE,
					  (shared ? MAP_SHARED : MAP_PRIVATE) | MAP_FIXED,
        			fd,
        			shuffling[i] * pageSize);
            VALID_PTR(pageToRemap, -1, "mmap pageToRemap", false);
            , "mmap pageToRemap", true);
        }

        SAFE_FREE(shuffling);
    }
    else {
        measure(&start);
        ERRNO_CHECK(
        src = (entry_t*) mmap(NULL,
        					  sizeRoundedToPageBoundary,
							  PROT_READ | PROT_WRITE,
							  (shared ? MAP_SHARED : MAP_PRIVATE)
							  | (file ? 0 : MAP_ANONYMOUS)
							  | (hugePages && !file ? MAP_HUGETLB : 0)
							  | (populate && populateByOption ? MAP_POPULATE : 0),
							  file ? fd : -1,
							  0);
        VALID_PTR(src, -1, "mmap src", false);
        , "mmap src", true);
    }

    if(!file) {
    	//madvise(src, sizeRoundedToPageBoundary, hugePages ? MADV_HUGEPAGE : MADV_NOHUGEPAGE);
    }

    // asynchronous prefaulting if requested
    if(populate && !populateByOption) {
    	pthread_mutex_t remappedMutex;
    	pthread_mutex_init(&remappedMutex, NULL);
    	// launch asynchronous prefaulting
		pthread_t* prefaulter = malloc(sizeof(*prefaulter)); // TODO: memory leak here, cleanup threads..
		prefaultPagesData_t* pD = malloc(sizeof(*pD));
		pD->mem = (char*) src;
		pD->numBytes = sizeRoundedToPageBoundary;
		pD->pageSize = pageSize;
		pD->currentMem = (char**) &src;
		pD->remappedMutex = &remappedMutex;
		pthread_create(prefaulter, NULL, prefaultPagesAsync, pD);
    }
    measure(&end);
    printTimeDifference(&start, &end, MMAP_SRC, measurement);

    printf("\n");

    setWdStandard(newWorkingData, src, NULL, fd);
}

void mapAndPopulateCleanup(wd_pt* const workingData,
							const size_t size,
							const bool hugePages,
							const bool file) {
	notNull(workingData, "workingData");
	// expecting wdStandard_t
	wdStandard_t* wd = (wdStandard_t*) getWdStandardPtr(workingData);

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;

    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (pageSize - ((size * sizeof(entry_t)) % pageSize));
	if(wd->src) {
		ERRNO_CHECK(
	    munmap(wd->src, sizeRoundedToPageBoundary);
	    wd->src = NULL;
	    , "munmap src", true);
        printf("Freeing %s\n", getFieldName(MMAP_SRC));
	}

	if(file) {
		if(hugePages) {
			ERRNO_CHECK(
			close(wd->fd);
			wd->fd = -1;
			, "close", true);
			ERRNO_CHECK(
			unlink("/mnt/hugetlbfs/mapAndPopulate");
			, "unlinck /mnt/hugetlbfs/mapAndPopulate", true);
			printf("Freeing %s\n", getFieldName(SHM_SRC_HUGE));
		}
		else {
			ERRNO_CHECK(
			shm_unlink("/mapAndPopulate");
			, "shm_unlick /intcpy", true);
			printf("Freeing %s\n", getFieldName(SHM_SRC));
		}
	}
}

