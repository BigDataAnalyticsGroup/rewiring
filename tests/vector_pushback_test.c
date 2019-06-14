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
 * vector_pushback_test.c
 *
 *  Created on: Mar 09, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "vector_pushback_test.h"

//#define SORT

void rewiredVectorPushbackTest(rewiredVector_t* const rv, const size_t size, measurement_t* const measurement, const bool detailed) {
	notNull(rv, "rv");
	notNull(measurement, "measurement");

	timeval_t start, end;

	entry_t* entries = malloc(sizeof(*entries) * size);
	for(size_t i = 0; i < size; ++i) entries[i] = urand64();
	shuffle_varsize(entries, sizeof(entry_t), size);

	if(detailed) {
		measure(&start);
		prepareDetailedTimeDifference(VECTOR_INSERT, measurement, size / DETAILED_GRANULARITY);
		timespec_t startDetailed, endDetailed;

		size_t elem = 0;
		for(size_t i = 0; i < size / DETAILED_GRANULARITY; ++i) {
			clock_gettime(CLOCK_REALTIME, &startDetailed);
			for(size_t g = 0; g < DETAILED_GRANULARITY; ++g) {
				rvPushBack(rv, entries + elem);
				++elem;
			}
			clock_gettime(CLOCK_REALTIME, &endDetailed);
			collectDetailedTimeDifference(&startDetailed, &endDetailed, i, VECTOR_INSERT, measurement);
		}
		measure(&end);
	}
	else {
		measure(&start);
		for(size_t i = 0; i < size; ++i) {
			rvPushBack(rv, entries + i);
		}
		measure(&end);
	}

	SAFE_FREE(entries);

	#ifdef ERRORCHECK
		entry_t total = 0;
		size_t numEntries = 0;
		entry_t* mem = rvGetMem(rv, &numEntries);
		for(size_t i = 0; i < numEntries; ++i) {
			total += mem[i];
			if(mem[i] != i) {
				printf("Error: mem[%ld] = %ld, but should be %ld", i, mem[i], i);
			}
		}
		printTimeDifferenceChecksum(&start, &end, VECTOR_INSERT, total, measurement);
	#else
		printTimeDifference(&start, &end, VECTOR_INSERT, measurement);
	#endif

	#ifdef SORT
		// sort the data
		measure(&start);
		hybridRadixsortInsert(rvGetMem(rv, NULL), size);
		measure(&end);
		printTimeDifference(&start, &end, RADIX_SORT, measurement);
	#endif
}

void mremapedVectorPushbackTest(rewiredVector_t* const rv, const size_t size, measurement_t* const measurement, const bool detailed) {
	notNull(rv, "rv");
	notNull(measurement, "measurement");

	timeval_t start, end;

	entry_t* entries = malloc(sizeof(*entries) * size);
	for(size_t i = 0; i < size; ++i) entries[i] = urand64();
	shuffle_varsize(entries, sizeof(entry_t), size);

	if(detailed) {
		measure(&start);
		prepareDetailedTimeDifference(VECTOR_INSERT, measurement, size / DETAILED_GRANULARITY);
		timespec_t startDetailed, endDetailed;

		size_t elem = 0;
		for(size_t i = 0; i < size / DETAILED_GRANULARITY; ++i) {
			clock_gettime(CLOCK_REALTIME, &startDetailed);
			for(size_t g = 0; g < DETAILED_GRANULARITY; ++g) {
				mremapedvPushBack(rv, entries + elem);
				++elem;
			}
			clock_gettime(CLOCK_REALTIME, &endDetailed);
			collectDetailedTimeDifference(&startDetailed, &endDetailed, i, VECTOR_INSERT, measurement);
		}
		measure(&end);
	}
	else {
		measure(&start);
		for(size_t i = 0; i < size; ++i) {
			mremapedvPushBack(rv, entries + i);
		}
		measure(&end);
	}

	SAFE_FREE(entries);

	#ifdef ERRORCHECK
		entry_t total = 0;
		size_t numEntries = 0;
		entry_t* mem = mremapedvGetMem(rv, &numEntries);
		for(size_t i = 0; i < numEntries; ++i) {
			total += mem[i];
			if(mem[i] != i) {
				printf("Error: mem[%ld] = %ld, but should be %ld", i, mem[i], i);
			}
		}
		printTimeDifferenceChecksum(&start, &end, VECTOR_INSERT, total, measurement);
	#else
		printTimeDifference(&start, &end, VECTOR_INSERT, measurement);
	#endif

	#ifdef SORT
		// sort the data
		measure(&start);
		hybridRadixsortInsert(mremapedvGetMem(rv, NULL), size);
		measure(&end);
		printTimeDifference(&start, &end, RADIX_SORT, measurement);
	#endif
}

void stlVectorPushbackTest(stlVec_pt const stlv, const size_t size, measurement_t* const measurement, const bool detailed) {
	notNull(stlv, "stlv");
	notNull(measurement, "measurement");

	timeval_t start, end;

	entry_t* entries = malloc(sizeof(*entries) * size);
	for(size_t i = 0; i < size; ++i) entries[i] = urand64();
	shuffle_varsize(entries, sizeof(entry_t), size);

	if(detailed) {
		measure(&start);
		prepareDetailedTimeDifference(VECTOR_INSERT, measurement, size / DETAILED_GRANULARITY);
		double* diffs = NULL;
		stlvPushBackArray(stlv, entries, size, &diffs);
		for(size_t i = 0; i < size / DETAILED_GRANULARITY; ++i) {
			collectDetailedTime(diffs[i], i, VECTOR_INSERT, measurement);
		}
		measure(&end);
	}
	else {
		measure(&start);
		stlvPushBackArray(stlv, entries, size, NULL);
		measure(&end);
	}

	SAFE_FREE(entries);

	#ifdef ERRORCHECK
		entry_t total = 0;
		size_t numEntries = 0;
		entry_t* mem = stlvGetMem(stlv, &numEntries);
		for(size_t i = 0; i < numEntries; ++i) {
			total += mem[i];
			if(mem[i] != i) {
				printf("Error: mem[%ld] = %ld, but should be %ld", i, mem[i], i);
			}
		}
		printTimeDifferenceChecksum(&start, &end, VECTOR_INSERT, total, measurement);
	#else
		printTimeDifference(&start, &end, VECTOR_INSERT, measurement);
	#endif

	#ifdef SORT
		// sort the data
		measure(&start);
		hybridRadixsortInsert(stlvGetMem(stlv, NULL), size);
		measure(&end);
		printTimeDifference(&start, &end, RADIX_SORT, measurement);
	#endif
}

void softwareIndVectorPushbackTest(softwareIndVector_t* const siv, const size_t size, measurement_t* const measurement, const bool detailed) {
	notNull(siv, "siv");
	notNull(measurement, "measurement");

	timeval_t start, end;

	entry_t* entries = malloc(sizeof(*entries) * size);
	for(size_t i = 0; i < size; ++i) entries[i] = urand64();
	shuffle_varsize(entries, sizeof(entry_t), size);

	if(detailed) {
		measure(&start);
		prepareDetailedTimeDifference(VECTOR_INSERT, measurement, size / DETAILED_GRANULARITY);
		timespec_t startDetailed, endDetailed;

		size_t elem = 0;
		for(size_t i = 0; i < size / DETAILED_GRANULARITY; ++i) {
			clock_gettime(CLOCK_REALTIME, &startDetailed);
			for(size_t g = 0; g < DETAILED_GRANULARITY; ++g) {
				softwareIndPushBack(siv, entries + elem);
				++elem;
			}
			clock_gettime(CLOCK_REALTIME, &endDetailed);
			collectDetailedTimeDifference(&startDetailed, &endDetailed, i, VECTOR_INSERT, measurement);
		}
		measure(&end);
	}
	else {
		measure(&start);
		for(size_t i = 0; i < size; ++i) {
			softwareIndPushBack(siv, entries + i);
		}
		measure(&end);
	}

	SAFE_FREE(entries);

	#ifdef ERRORCHECK
		// TODO: error check
		printTimeDifference(&start, &end, VECTOR_INSERT, measurement);
	#else
		printTimeDifference(&start, &end, VECTOR_INSERT, measurement);
	#endif

	#ifdef SORT
		measure(&start);
		softwareIndDoHybridRadixsortInsert(softwareIndGetMem(siv, NULL, NULL), 0, ENTRIES_PER_HUGE_PAGE, ENTRIES_PER_HUGE_PAGE_LOG2, size, 56);
		measure(&end);
		printTimeDifference(&start, &end, RADIX_SORT, measurement);
	#endif
}
