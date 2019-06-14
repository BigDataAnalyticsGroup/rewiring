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
 * vector_insert_test.c
 *
 *  Created on: Mar 13, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "vector_insert_test.h"

void rewiredVectorInsertTest(rewiredVector_t* const rv, const size_t size, measurement_t* const measurement, const bool detailed) {
//	notNull(rv, "rv");
//	notNull(measurement, "measurement");
//
//	timeval_t start, end;
//
//	entry_t* entries = malloc(sizeof(*entries) * size);
//	for(size_t i = 0; i < size; ++i) entries[i] = i;
//
//	if(detailed) {
//		measure(&start);
//		prepareDetailedTimeDifference(VECTOR_INSERT, measurement, size / DETAILED_GRANULARITY);
//		timespec_t startDetailed, endDetailed;
//
//		size_t elem = 0;
//		for(size_t i = 0; i < size / DETAILED_GRANULARITY; ++i) {
//			measureHighRes(&startDetailed);
//			for(size_t g = 0; g < DETAILED_GRANULARITY; ++g) {
//				rvPushBack(rv, entries + elem);
//				++elem;
//			}
//			measureHighRes(&endDetailed);
//			collectDetailedTimeDifference(&startDetailed, &endDetailed, i, VECTOR_INSERT, measurement);
//		}
//		measure(&end);
//	}
//	else {
//		measure(&start);
//		for(size_t i = 0; i < size; ++i) {
//			rvPushBack(rv, entries + i);
//		}
//		measure(&end);
//	}
//
//	SAFE_FREE(entries);
//
//	entry_t total = 0;
//	size_t numEntries = 0;
//	entry_t* mem = rvGetMem(rv, &numEntries);
//	for(size_t i = 0; i < numEntries; ++i) {
//		total += mem[i];
//		if(mem[i] != i) {
//			printf("Error: mem[%ld] = %ld, but should be %ld", i, mem[i], i);
//		}
//	}
//
//	printTimeDifferenceChecksum(&start, &end, VECTOR_INSERT, total, measurement);
}


void stlVectorInsertTest(stlVec_pt const stlv, const size_t size, measurement_t* const measurement, const bool detailed, const bool ensureCapacity) {
	notNull(stlv, "stlv");
	notNull(measurement, "measurement");

	timeval_t start, end;

	entry_t* entries = malloc(sizeof(*entries) * size);
	for(size_t i = 0; i < size; ++i) entries[i] = i;

	entry_t* page = malloc(HUGE_PAGE_SIZE);
	const size_t entriesPerPage = HUGE_PAGE_SIZE / sizeof(entry_t);
	for(size_t i = 0; i < entriesPerPage; ++i) page[i] = i;


	if(detailed) prepareDetailedTimeDifference(VECTOR_INSERT, measurement, size / entriesPerPage);
	timespec_t startDetailed, endDetailed;

	measure(&start);
	size_t insertPosition = 0;
	size_t round = 0;
	while(insertPosition + entriesPerPage <= size) {
		// setup test
		stlvClear(stlv);
		stlvReserve(stlv, size + (ensureCapacity ? HUGE_PAGE_SIZE : 0));
		stlvPushBackArray(stlv, entries, size, NULL);

		if(detailed) {
			measureHighRes(&startDetailed);
		}
		stlvInsertPage(stlv, entries, entriesPerPage, insertPosition);
		if(detailed) {
			measureHighRes(&endDetailed);
			collectDetailedTimeDifference(&startDetailed, &endDetailed, round, VECTOR_INSERT, measurement);
		}
		insertPosition += entriesPerPage;
		++round;
	}
	measure(&end);


	SAFE_FREE(entries);
	SAFE_FREE(page);

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
}
