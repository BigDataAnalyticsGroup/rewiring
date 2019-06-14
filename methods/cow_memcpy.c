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
 * cow_memcpy.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "cow_memcpy.h"

void cowMemcpy(wd_pt* const workingData, const size_t size, const bool populate, measurement_t* const measurement) {
	// creating wdStandard_t
	wdStandard_t* newWorkingData = getWdStandard();
	*workingData = newWorkingData;

	timeval_t start, end;
    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (SMALL_PAGE_SIZE - ((size * sizeof(entry_t)) % SMALL_PAGE_SIZE));

	// allocation of src array
    measure(&start);
    ERRNO_CHECK(
    filedescriptor_t fd = shm_open("/intcpy", O_RDWR | O_CREAT, 0666);
    , "shm_open", true);
    ERRNO_CHECK(
    ftruncate(fd, sizeRoundedToPageBoundary);
    , "ftruncate src", true);
    measure(&end);
    printTimeDifference(&start, &end, SHM_SRC, measurement);

    measure(&start);
    ERRNO_CHECK(
    entry_t* src = (entry_t*) mmap(NULL, sizeRoundedToPageBoundary, PROT_READ | PROT_WRITE, MAP_SHARED | (populate ? MAP_POPULATE : 0), fd, 0);
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

    // copy from src to dst array lazily (COW)
    measure(&start);
    ERRNO_CHECK(
    entry_t* dst = (entry_t*) mmap(NULL, sizeRoundedToPageBoundary, PROT_READ | PROT_WRITE, MAP_PRIVATE | (populate ? MAP_POPULATE : 0), fd, 0);
    VALID_PTR(dst, -1, "mmap dst", false);
    , "mmap dst", true);
    measure(&end);
    printTimeDifference(&start, &end, MMAP_DST, measurement);

    printf("\n");

    setWdStandard(newWorkingData, src, dst, fd);
}

void cowMemcpyCleanup(wd_pt* const workingData, const size_t size) {
	notNull(workingData, "workingData");
	// expecting wdStandard_t
	wdStandard_t* wd = (wdStandard_t*) getWdStandardPtr(workingData);

    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (SMALL_PAGE_SIZE - ((size * sizeof(entry_t)) % SMALL_PAGE_SIZE));
	if(wd->src) {
		ERRNO_CHECK(
		munmap(wd->src, sizeRoundedToPageBoundary);
	    wd->src = NULL;
	    , "munmap src", true);
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
	close(wd->fd);
	, "close fd", true);

    ERRNO_CHECK(
    printf("%d\n", shm_unlink("/intcpy"));
    , "shm_unlick /intcpy", true);
    printf("Freeing %s\n", getFieldName(SHM_SRC));

}

