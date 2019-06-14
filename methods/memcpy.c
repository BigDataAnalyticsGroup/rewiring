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
 * memcpy.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "memcpy.h"

void traditionalMemcpy(wd_pt* const workingData, const size_t size, measurement_t* const measurement) {
	notNull(workingData, "workingData");
	// creating wdStandard_t
	wdStandard_t* newWorkingData = getWdStandard();
	*workingData = newWorkingData;

	timeval_t start, end;

    // allocation of src array
    measure(&start);
    entry_t* src = (entry_t*) malloc(sizeof(entry_t) * size);
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

    // allocation of dst array
    measure(&start);
    entry_t* dst = (entry_t*) malloc(sizeof(entry_t) * size);
    measure(&end);
    printTimeDifference(&start, &end, MALLOC_DST, measurement);

    // copying src to dst array
    measure(&start);
    memcpy(dst, src, sizeof(entry_t) * size);
    measure(&end);
    printTimeDifference(&start, &end, MEMCPY, measurement);

    printf("\n");

    setWdStandard(newWorkingData, src, dst, -1);
}

void traditionalMemcpyCleanup(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	// expecting wdStandard_t
	wdStandard_t* wd = (wdStandard_t*) getWdStandardPtr(workingData);

	if(SAFE_FREE(wd->src)) {
        printf("Freeing %s\n", getFieldName(MALLOC_SRC));
	}
	if(SAFE_FREE(wd->dst)) {
        printf("Freeing %s\n", getFieldName(MALLOC_DST));
	}

    SAFE_FREE(wd);
}



