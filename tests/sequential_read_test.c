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
 * sequential_read_test.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "sequential_read_test.h"

void sequentialReadTest(const entry_t* const p, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");

	timeval_t start, end;

    measure(&start);
    entry_t total = 0;
    const size_t effective_size = ((double) size / 100.0) * selectivity;
    for(size_t i = 0; i < effective_size; i++) {
        total += p[i];
    }
    measure(&end);
    printTimeDifferenceChecksum(&start, &end, f, total, measurement);
}

void sequentialReadTestDependent(entry_t* const p, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");

	timeval_t start, end;

	for(size_t i = 0; i < size - 1; ++i) {
		p[i] = i + 1;
	}
	p[size - 1] = 0;

    measure(&start);
    entry_t total = 0;

    uint32_t nextPos = 0;
    const size_t effective_size = ((double) size / 100.0) * selectivity;
    for(size_t i = 0; i < effective_size; i++) {
        total += (nextPos = p[nextPos]);
    }
    measure(&end);
    printTimeDifferenceChecksum(&start, &end, f, total, measurement);
}

void sequentialReadTableTest(const row_t* const p, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");

	timeval_t start, end;

    measure(&start);
    entry_t total = 0;
    const size_t effective_size = ((double) size / 100.0) * selectivity;
    for(size_t i = 0; i < effective_size; i++) {
        total += p[i].cols[0];
    }
    measure(&end);
    printTimeDifferenceChecksum(&start, &end, f, total, measurement);
}

void sequentialReadSoftwareIndirectionTestHugePages(entry_t** dir, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(dir, "dir");
	notNull(measurement, "measurement");

	timeval_t start, end;

    measure(&start);
    entry_t total = 0;
    const size_t effective_size = ((double) size / 100.0) * selectivity;
    for(size_t i = 0; i < effective_size; i++) {
    	total += dir[i >> ENTRIES_PER_HUGE_PAGE_LOG2][i & (ENTRIES_PER_HUGE_PAGE - 1)];
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, effective_size, selectivity, total, measurement);
}

void sequentialReadSoftwareIndirectionTestSmallPages(entry_t** dir, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(dir, "dir");
	notNull(measurement, "measurement");

	timeval_t start, end;

    measure(&start);
    entry_t total = 0;
    const size_t effective_size = ((double) size / 100.0) * selectivity;
    for(size_t i = 0; i < effective_size; i++) {
    	total += dir[i >> ENTRIES_PER_SMALL_PAGE_LOG2][i & (ENTRIES_PER_SMALL_PAGE - 1)];
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, effective_size, selectivity, total, measurement);
}

void createDependencySoftwareIndirectionSequential(entry_t** dir,
											const size_t entriesPerPageLog2,
											const size_t entriesPerPage,
											const size_t size) {
    for(size_t i = 0; i < size - 1; ++i) {
    	dir[i >> entriesPerPageLog2][i & (entriesPerPage - 1)] = i+1;
    }
    dir[(size-1) >> entriesPerPageLog2][(size-1) & (entriesPerPage - 1)] = 0;
}

void sequentialReadSoftwareIndirectionTestDependentHugePages(entry_t** dir, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(dir, "dir");
	notNull(measurement, "measurement");

	createDependencySoftwareIndirectionSequential(dir, ENTRIES_PER_HUGE_PAGE_LOG2, ENTRIES_PER_HUGE_PAGE, size);

	timeval_t start, end;

    measure(&start);
    uint32_t nextPos = 0;
    entry_t total = 0;
    const size_t effective_size = ((double) size / 100.0) * selectivity;
    for(size_t i = 0; i < effective_size; i++) {
		total += (nextPos = dir[nextPos >> ENTRIES_PER_HUGE_PAGE_LOG2][nextPos & (ENTRIES_PER_HUGE_PAGE - 1)]);
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, effective_size, selectivity, total, measurement);
}

void sequentialReadTestFromSeveralChunks(entry_t** p, const size_t* const size, const size_t numChunks, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");
	notNull(size, "size");

	timeval_t start, end;

    measure(&start);
    entry_t total = 0;
    for(size_t c = 0; c < numChunks; ++c) {
    	for(size_t i = 0; i < size[c]; ++i) {
    		total += p[c][i];
    	}
    }
    measure(&end);
    printTimeDifferenceChecksum(&start, &end, f, total, measurement);
}

