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
 * random_read_test.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "random_read_test.h"

#define NUM_RANDOM_ACCESSES 5

void randomReadTest(const entry_t* const p,
					const size_t size,
					const double selectivity,
					const field_t f,
					measurement_t* const measurement) {
	notNull(p, "p");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    measure(&start);
    entry_t total = 0;
    for (size_t i = 0; i < num_reads; i++) {
        size_t index = rand() % size;
        total += p[index];
    }
    measure(&end);
    printTimeDifferenceSelectivityChecksum(&start, &end, f, selectivity, total, measurement);
}

void createDependency(entry_t* const p, const size_t size) {
    uint32_t* idxs = malloc(sizeof(*idxs) * size);
    for(uint32_t i = 0; i < size; ++i) {
    	idxs[i] = i;
    }
    // randomize the list order
    shuffle_varsize(idxs, sizeof(*idxs), size);
    for(size_t i = 0; i < size - 1; ++i) {
    	p[idxs[i]] = idxs[i+1];
    }
    p[idxs[size-1]] = idxs[0];
    SAFE_FREE(idxs);
}

void createDependencySoftwareIndirection(entry_t** dir,
											const size_t entriesPerPageLog2,
											const size_t entriesPerPage,
											const size_t size) {
    uint32_t* idxs = malloc(sizeof(*idxs) * size);
    for(uint32_t i = 0; i < size; ++i) {
    	idxs[i] = i;
    }
    // randomize the list order
    shuffle_varsize(idxs, sizeof(*idxs), size);
    for(size_t i = 0; i < size - 1; ++i) {
    	dir[idxs[i] >> entriesPerPageLog2][idxs[i] & (entriesPerPage - 1)] = idxs[i+1];
    }
    dir[idxs[size-1] >> entriesPerPageLog2][idxs[size-1] & (entriesPerPage - 1)] = idxs[0];
    SAFE_FREE(idxs);
}

void randomReadTestDependent(entry_t* const p,
								const size_t size,
								const double selectivity,
								const field_t f,
								measurement_t* const measurement) {
	notNull(p, "p");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    createDependency(p, size);

    measure(&start);
    entry_t total = 0;
    uint32_t nextPos = 0;
    for (size_t i = 0; i < num_reads; i++) {
		total += (nextPos = p[nextPos]);
    }

    measure(&end);
    printTimeDifferenceSelectivityChecksum(&start, &end, f, selectivity, total, measurement);
}

void randomReadTestDependentMixed(entry_t* const p,
									const size_t size,
									const double selectivity,
									const size_t numRandomAccesses,
									const field_t f,
									measurement_t* const measurement) {
	notNull(p, "p");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    createDependency(p, size);

    measure(&start);
    entry_t total = 0;
    uint32_t nextPos = 0;
    for (size_t i = 0; i < num_reads; i++) {
		nextPos = rand() & (size - 1);
		for(size_t j = 0; j < NUM_RANDOM_ACCESSES; ++j) {
			total += (nextPos = p[nextPos]);
		}
    }

    measure(&end);
    printTimeDifferenceSelectivityChecksum(&start, &end, f, selectivity, total, measurement);
}

void randomReadSoftwareIndirectionTestHugePages(entry_t** dir, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(dir, "dir");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    measure(&start);
    entry_t total = 0;
    for(size_t i = 0; i < num_reads; i++) {
        size_t index = rand() % size;
        total += dir[index >> ENTRIES_PER_HUGE_PAGE_LOG2][index & (ENTRIES_PER_HUGE_PAGE - 1)];
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, num_reads, selectivity, total, measurement);
}

void randomReadSoftwareIndirectionTestDependentHugePages(entry_t** dir,
															const size_t size,
															const double selectivity,
															const field_t f,
															measurement_t* const measurement) {
	notNull(dir, "dir");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    createDependencySoftwareIndirection(dir, ENTRIES_PER_HUGE_PAGE_LOG2, ENTRIES_PER_HUGE_PAGE, size);

    measure(&start);
    entry_t total = 0;
    uint32_t nextPos = 0;
    for(size_t i = 0; i < num_reads; i++) {
		total += (nextPos = dir[nextPos >> ENTRIES_PER_HUGE_PAGE_LOG2][nextPos & (ENTRIES_PER_HUGE_PAGE - 1)]);
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, num_reads, selectivity, total, measurement);
}

void randomReadSoftwareIndirectionTestDependentMixedHugePages(entry_t** dir,
															const size_t size,
															const double selectivity,
															const size_t numRandomAccesses,
															const field_t f,
															measurement_t* const measurement) {
	notNull(dir, "dir");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    createDependencySoftwareIndirection(dir, ENTRIES_PER_HUGE_PAGE_LOG2, ENTRIES_PER_HUGE_PAGE, size);

    measure(&start);
    entry_t total = 0;
    uint32_t nextPos = 0;
    for(size_t i = 0; i < num_reads; i++) {
		nextPos = rand() & (size - 1);
		for(size_t j = 0; j < NUM_RANDOM_ACCESSES; ++j) {
			total += (nextPos = dir[nextPos >> ENTRIES_PER_HUGE_PAGE_LOG2][nextPos & (ENTRIES_PER_HUGE_PAGE - 1)]);
		}
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, num_reads, selectivity, total, measurement);
}

void randomReadSoftwareIndirectionTestSmallPages(entry_t** dir,
													const size_t size,
													const double selectivity,
													const field_t f,
													measurement_t* const measurement) {
	notNull(dir, "dir");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    measure(&start);
    entry_t total = 0;
    for(size_t i = 0; i < num_reads; i++) {
        size_t index = ( ((double)rand()) / (double)RAND_MAX) * (num_reads - 1);
        total += dir[index >> ENTRIES_PER_SMALL_PAGE_LOG2][index & (ENTRIES_PER_SMALL_PAGE - 1)];
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, num_reads, selectivity, total, measurement);
}

void randomReadSoftwareIndirectionTestDependentSmallPages(entry_t** dir,
															const size_t size,
															const double selectivity,
															const field_t f,
															measurement_t* const measurement) {
	notNull(dir, "dir");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    createDependencySoftwareIndirection(dir, ENTRIES_PER_SMALL_PAGE_LOG2, ENTRIES_PER_SMALL_PAGE, size);

    measure(&start);
    entry_t total = 0;
    uint32_t nextPos = 0;
    for(size_t i = 0; i < num_reads; i++) {
        total += (nextPos = dir[nextPos >> ENTRIES_PER_SMALL_PAGE_LOG2][nextPos & (ENTRIES_PER_SMALL_PAGE - 1)]);
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, num_reads, selectivity, total, measurement);
}

void randomReadSoftwareIndirectionTestDependentMixedSmallPages(entry_t** dir,
															const size_t size,
															const double selectivity,
															const size_t numRandomAccesses,
															const field_t f,
															measurement_t* const measurement) {
	notNull(dir, "dir");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    createDependencySoftwareIndirection(dir, ENTRIES_PER_SMALL_PAGE_LOG2, ENTRIES_PER_SMALL_PAGE, size);

    measure(&start);
    entry_t total = 0;
    uint32_t nextPos = 0;
    for(size_t i = 0; i < num_reads; i++) {
		nextPos = rand() & (size - 1);
		for(size_t j = 0; j < numRandomAccesses; ++j) {
			total += (nextPos = dir[nextPos >> ENTRIES_PER_SMALL_PAGE_LOG2][nextPos & (ENTRIES_PER_SMALL_PAGE - 1)]);
		}
    }
    measure(&end);
    printTimeDifferenceSelectivityPercentChecksum(&start, &end, f, num_reads, selectivity, total, measurement);
}

void randomReadIndirectionTest(entry_t** indirection,
								const size_t size,
								const double selectivity,
								const field_t f,
								measurement_t* const measurement) {
	notNull(indirection, "indirection");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    measure(&start);
    entry_t total = 0;
    for (size_t i = 0; i < num_reads; i++) {
        size_t index = ( ((double)rand()) / (double)RAND_MAX) * (num_reads - 1);
        total = total + *(indirection[index]);
    }
    measure(&end);
    printTimeDifferenceSelectivityChecksum(&start, &end, f, selectivity, total, measurement);
}

void randomReadTableTest(const row_t* const p,
							const size_t size,
							const double selectivity,
							const field_t f,
							measurement_t* const measurement) {
	notNull(p, "p");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * (selectivity / 100.0);

    srand(SEED);
    measure(&start);
    entry_t total = 0;
    for (size_t i = 0; i < num_reads; i++) {
        size_t index = ( ((double)rand()) / (double)RAND_MAX) * (num_reads - 1);
        total = total + p[index].cols[0];
    }
    measure(&end);
    printTimeDifferenceSelectivityChecksum(&start, &end, f, selectivity, total, measurement);
}
