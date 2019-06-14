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
 * cow_huge_small_mix.c
 *
 *  Created on: Mar 05, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "cow_huge_small_mix.h"

void cowHugeSmallMix(wd_pt* const workingData, const size_t size, measurement_t* const measurement) {
	// creating wdMixed_t
	wdMixed_t* newWorkingData = getWdMixed();
	*workingData = newWorkingData;

	timeval_t start, end;
    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (HUGE_PAGE_SIZE - ((size * sizeof(entry_t)) % HUGE_PAGE_SIZE));

	// allocation of src array, both using huge pages and small pages
    measure(&start);
    ERRNO_CHECK(
    // get file backed with small pages
    filedescriptor_t fdSmall = shm_open("/smallPages", O_RDWR | O_CREAT, 0666);
    , "shm_open", true);
    ERRNO_CHECK(
    ftruncate(fdSmall, sizeRoundedToPageBoundary);
    , "ftruncate src", true);

    // get file backed with huge pages
    ERRNO_CHECK(
    filedescriptor_t fdHuge = open("/mnt/hugetlbfs/hugePages", O_RDWR | O_CREAT, 0666);
    , "open", true);

    measure(&end);
    printTimeDifference(&start, &end, SHM_SRC, measurement);

    measure(&start);
    ERRNO_CHECK(
    entry_t* srcSmall = (entry_t*) mmap(NULL, sizeRoundedToPageBoundary, PROT_READ | PROT_WRITE, MAP_SHARED, fdSmall, 0);
    VALID_PTR(srcSmall, -1, "mmap src small", false);
    , "mmap src small", true);

    ERRNO_CHECK(
    entry_t* srcHuge = (entry_t*) mmap(NULL, sizeRoundedToPageBoundary, PROT_READ | PROT_WRITE, MAP_SHARED, fdHuge, 0);
    VALID_PTR(srcHuge, -1, "mmap src huge", false);
    , "mmap src huge", true);
    measure(&end);
    printTimeDifference(&start, &end, MMAP_SRC, measurement);

    // initialization of both src arrays
    measure(&start);
    for(size_t i = 0; i < size; i++) {
        srcSmall[i] = i;
        srcHuge[i] = i;
    }
    measure(&end);
    printTimeDifference(&start, &end, INIT_SHM_SRC, measurement);

    // copy from src to dst array lazily (COW)
    measure(&start);

    ERRNO_CHECK(
    entry_t* dst = (entry_t*) mmap(NULL, sizeRoundedToPageBoundary, PROT_READ | PROT_WRITE, MAP_SHARED, fdHuge, 0);
    VALID_PTR(dst, -1, "mmap dst", false);
    , "mmap dst", true);

    ERRNO_CHECK(
    mmap(dst + ((sizeRoundedToPageBoundary - HUGE_PAGE_SIZE) / sizeof(entry_t)), HUGE_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fdSmall, sizeRoundedToPageBoundary - HUGE_PAGE_SIZE);
    VALID_PTR(dst, -1, "mmap dst", false);
    , "mmap dst", true);

    measure(&end);
    printTimeDifference(&start, &end, MMAP_DST, measurement);

    printf("\n");

    setWdMixed(newWorkingData, srcSmall, srcHuge, dst, fdSmall, fdHuge);
}

void cowHugeSmallMixCleanup(wd_pt* const workingData, const size_t size) {
	notNull(workingData, "workingData");
	// expecting wdMixed_t
	wdMixed_t* wd = (wdMixed_t*) getWdMixedPtr(workingData);

    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (HUGE_PAGE_SIZE - ((size * sizeof(entry_t)) % HUGE_PAGE_SIZE));
	if(wd->srcSmall) {
		ERRNO_CHECK(
	    munmap(wd->srcSmall, sizeRoundedToPageBoundary);
	    wd->srcSmall = NULL;
	    , "munmap srcSmall", true);
        printf("Freeing %s\n", getFieldName(MMAP_SRC));
	}
	if(wd->srcHuge) {
		ERRNO_CHECK(
	    munmap(wd->srcHuge, sizeRoundedToPageBoundary);
	    wd->srcHuge = NULL;
	    , "munmap srcHuge", true);
        printf("Freeing %s\n", getFieldName(MMAP_SRC));
	}
	if(wd->dst) {
		ERRNO_CHECK(
		munmap(wd->dst, sizeRoundedToPageBoundary);
		wd->dst = NULL;
	    , "munmap dst", true);
        printf("Freeing %s\n", getFieldName(MMAP_DST));
	}

    ERRNO_CHECK(
    shm_unlink("/smallPages");
    , "shm_unlick /smallPages", true);
    printf("Freeing %s\n", getFieldName(SHM_SRC));

	ERRNO_CHECK(
	close(wd->fdHuge);
	wd->fdHuge = -1;
	, "close", true);
	ERRNO_CHECK(
	unlink("/mnt/hugetlbfs/hugePages");
	, "unlinck /mnt/hugetlbfs/hugePages", true);
}

