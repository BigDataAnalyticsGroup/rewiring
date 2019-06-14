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
 * sequential_write_test.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "sequential_write_test.h"

void sequentialWriteTest(entry_t* const p, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");
	notNull(measurement, "measurement");

	timeval_t start, end;

    measure(&start);
    entry_t total = 0;
    const size_t effective_size = ((double) size / 100.0) * selectivity;
    for(size_t i = 0; i < effective_size; i++) {
        p[i] = size - 1 - i;
        total += p[i];
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, effective_size, selectivity, total, measurement);
}

void sequentialWriteSoftwareIndirectionTest(entry_t** dir, const size_t size, const size_t hugePages, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(dir, "dir");
	notNull(measurement, "measurement");

	timeval_t start, end;

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
	const size_t entriesPerPage = pageSize / sizeof(entry_t);
	const size_t entriesPerPageLog2 = log2(entriesPerPage);

    measure(&start);
    entry_t total = 0;
    const size_t effective_size = ((double) size / 100.0) * selectivity;
    for(size_t i = 0; i < effective_size; i++) {
    	entry_t* entry = dir[i >> entriesPerPageLog2] + (i & (entriesPerPage - 1));
    	*entry = size - 1 - i;
        total += *entry;
    }

    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, effective_size, selectivity, total, measurement);
}

//void sequentialWriteSoftwareIndirectionTest(entry_t** dir, const size_t size, const size_t hugePages, const double selectivity, const field_t f, measurement_t* const measurement) {
//	notNull(dir, "dir");
//	notNull(measurement, "measurement");
//
//	timeval_t start, end;
//
//	// simulate an iterator-style processing
//
//	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
//	const size_t entriesPerPage = pageSize / sizeof(entry_t);
//    const size_t effectiveSize = ((double) size / 100.0) * selectivity;
//
//	iteratorState state;
//	state.dir = dir;
//	state.entriesPerPage = entriesPerPage;
//	state.currentDirSlot = 0;
//	state.currentInPage = 0;
//
//    measure(&start);
//    entry_t total = 0;
//    for(size_t i = 0; i < effectiveSize; i++) {
//    	entry_t* entry = next(&state);
//    	*entry = size - 1 - i;
//        total += *entry;
//    }
//
//    measure(&end);
//    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, effectiveSize, selectivity, total, measurement);
//}

void sequentialWriteTableTest(row_t* const p, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");
	notNull(measurement, "measurement");

	timeval_t start, end;

    measure(&start);
    entry_t total = 0;
    const size_t effective_size = ((double) size / 100.0) * selectivity;
    for(size_t i = 0; i < effective_size; i++) {
        p[i].cols[0] = size - 1 - i;
        total += p[i].cols[0];
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, effective_size, selectivity, total, measurement);
}
