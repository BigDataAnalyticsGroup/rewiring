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
 * stl_vector.c
 *
 *  Created on: Mar 10, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "stl_vector.h"

stlVec_pt stlvGetNewVector(const size_t initCapacity) {
	std::vector<entry_t>* vec = new std::vector<entry_t>;
	vec->reserve(initCapacity);

	return (stlVec_pt) vec;
}

void stlvPushBack(stlVec_pt const v,
				  const entry_t* const entry) {
	std::vector<entry_t>* vec = (std::vector<entry_t>*) v;
	vec->push_back(*entry);
}

void stlvPushBackArray(stlVec_pt const v,
				      const entry_t* const entries,
					  const size_t size,
					  double** diffs) {
	std::vector<entry_t>* vec = (std::vector<entry_t>*) v;

	if(diffs) {
		// detailed time measurement
		timespec_t start, end;
		*diffs = (double*) malloc(sizeof(**diffs) * size);

		size_t elem = 0;
		for(size_t i = 0; i < size / DETAILED_GRANULARITY; ++i) {
	        clock_gettime(CLOCK_REALTIME, &start);
			for(size_t g = 0; g < DETAILED_GRANULARITY; ++g) {
				vec->push_back(entries[elem]);
				++elem;
			}
	        clock_gettime(CLOCK_REALTIME, &end);
			(*diffs)[i] = ((end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec));
			(*diffs)[i] /= 1000;
		}
	}
	else {
		// standard time measurement
		for(size_t i = 0; i < size; ++i) {
			vec->push_back(entries[i]);
		}
	}
}

void stlvInsertPage(stlVec_pt const v,
					const entry_t* const entries,
					const size_t size,
					const size_t insertPosition) {
	std::vector<entry_t>* vec = (std::vector<entry_t>*) v;

	assert(size % HUGE_PAGE_SIZE == 0);
	assert(insertPosition % HUGE_PAGE_SIZE == 0);

	vec->insert(vec->begin() + insertPosition, entries, entries + size);
}

void stlvClear(stlVec_pt const v) {
	std::vector<entry_t>* vec = (std::vector<entry_t>*) v;

	vec->clear();
}

void stlvReserve(stlVec_pt const v, const size_t size) {
	std::vector<entry_t>* vec = (std::vector<entry_t>*) v;

	vec->reserve(size);
}

entry_t* stlvGetMem(const stlVec_pt v,
		     	    size_t* numEntries) {
	std::vector<entry_t>* vec = (std::vector<entry_t>*) v;
	if(numEntries) *numEntries = vec->size();
	return vec->data();
}

void stlvFreeVector(stlVec_pt const v) {
	std::vector<entry_t>* vec = (std::vector<entry_t>*) v;
	if(vec) {
		delete vec;
		vec = NULL;
	}
}
