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
 * radix_sort_test.h
 *
 *  Created on: Feb 26, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef RADIX_SORT_TEST_H_
#define RADIX_SORT_TEST_H_

#include <string.h>
#include "../types.h"
#include "../timefile.h"
#include "../timediff.h"
#include "../useful_functions.h"

#define INSERT_SORT_LEVEL 64

void insertionSort(entry_t* const c, const size_t n);
void insertionSortIndirect(entry_t** const c, const size_t n);

void doHybridRadixsortInsert(entry_t* const c, const size_t n, size_t shift);
void doHybridRadixsortInsertIndirect(entry_t** const c, const size_t n, size_t shift);

void hybridRadixsortInsert(entry_t* const c, const size_t n);
void hybridRadixsortInsertIndirect(entry_t** const c, const size_t n);

void radixSortTest(entry_t* const p, const size_t size, const field_t f, measurement_t* const measurement);
void radixSortTestFromSeveralChunks(entry_t** const p, const size_t* size, const size_t numChunks, const field_t f, measurement_t* const measurement);
void radixSortTestFromSeveralChunksIndirection(entry_t*** const p, const size_t* size, const size_t numChunks, const field_t f, measurement_t* const measurement);
void softwareIndRadixSortTestFromSeveralChunks(entry_t** chunks,
												const size_t numChunks,
												const size_t* size,
												const size_t numPartitions,
												const size_t* histogram,
												const field_t f,
												measurement_t* const measurement);

entry_t** createIndirectionForChunks(entry_t** chunks, const size_t* const sizes, const size_t numChunks, size_t* const totalSize, const field_t f, measurement_t* const measurement);

void softwareIndInsertionSort(entry_t** const dir,
							const size_t offset,
							const size_t gran,
							const size_t granLog2,
							const size_t n);

void softwareIndDoHybridRadixsortInsert(entry_t** const dir,
										const size_t offset,
										const size_t gran,
										const size_t granLog2,
										const size_t n,
										size_t shift);

#endif /* RADIX_SORT_TEST_H_ */
