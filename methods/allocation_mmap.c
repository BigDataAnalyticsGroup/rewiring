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
 * allocation_mmap.c
 *
 *  Created on: Oct 19, 2015
 *      Author: Felix Martin Schuhknecht
 */

#include "allocation_mmap.h"

void allocationMmap(wd_pt* const workingData,
						const size_t size,
						const bool hugePages,
						measurement_t* const measurement) {

	// creating wdStandard_t
	wdStandard_t* newWorkingData = getWdStandard();
	*workingData = newWorkingData;

	timeval_t start, end;

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (pageSize - ((size * sizeof(entry_t)) % pageSize));

    measure(&start);
    ERRNO_CHECK(
    entry_t* dst = (entry_t*) mmap(NULL,
    								sizeRoundedToPageBoundary,
									PROT_READ | PROT_WRITE,
									MAP_ANONYMOUS | MAP_PRIVATE | (hugePages ? MAP_HUGETLB : 0) | MAP_NORESERVE,
									-1, 0);
    VALID_PTR(dst, -1, "mmap dst", false);
    , "mmap dst", true);
    measure(&end);
    printTimeDifference(&start, &end, MMAP_DST, measurement);

    printf("\n");

    setWdStandard(newWorkingData, NULL, dst, -1);
}

void allocationMmapCleanup(wd_pt* const workingData,
								const size_t size,
								const bool hugePages) {
	notNull(workingData, "workingData");
	// expecting wdStandard_t
	wdStandard_t* wd = (wdStandard_t*) getWdStandardPtr(workingData);

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (pageSize - ((size * sizeof(entry_t)) % pageSize));
	if(wd->dst) {
		ERRNO_CHECK(
		munmap(wd->dst, sizeRoundedToPageBoundary);
		wd->dst = NULL;
	    , "munmap dst", true);
        printf("Freeing %s\n", getFieldName(MMAP_DST));
	}
}

