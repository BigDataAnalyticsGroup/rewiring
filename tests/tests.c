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
 * tests.c
 *
 *  Created on: Mar 06, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "tests.h"

const char* getTestName(const testType_t test_type) {
	switch(test_type) {
		case SEQ_READ_SRC_TEST: return "Sequential read all src test.";
		case SEQ_READ_DST_TEST: return "Sequential read all dst test.";
		case RAND_READ_SRC_TEST: return "Uniformly at random read x%% of src data.";
		case RAND_READ_DST_TEST: return "Uniformly at random read x%% of dst data.";
		case SEQ_WRITE_SRC_TEST: return "Sequential write all src test.";
		case SEQ_WRITE_DST_TEST: return "Sequential write all dst test.";
		case RAND_WRITE_SRC_TEST: return "Uniformly at random write 1%% of src data.";
		case RAND_WRITE_DST_TEST: return "Uniformly at random write 1%% of dst data.";
		case SEQ_READ_DST_TWICE_TEST: return "Sequential read (two times) all dst test.";
		case SEQ_READ_DST_FROM_SEVERAL_CHUNKS: return "Sequential read all dst from several chunks.";
		case SEQ_READ_DST_MERGE_FROM_CHUNKS: return "Sequential read all dst after merging from several chunks.";
		case RADIX_SORT_DST_TEST: return "Radix sort all dst.";
		case RADIX_SORT_PER_PARTITION_CONSECUTIVE: return "Radix sort all dst from several partitions (in a consecutive memory region)";
		case RADIX_SORT_PER_PARTITION_INDIRECTION: return "Radix sort all dst from several partitions (using an indirection).";
		case VECTOR_INSERT_TEST: return "Vector insertion test.";
		case PAGE_FAULT_TEST: return "Page fault test.";
		case RAND_READ_DST_INDIRECTION_TEST: return "Uniformly at random read x%% of dst data through indirection.";
		case SEQ_WRITE_DST_TWICE_TEST: return "Sequential write (two times) all dst test.";
		case RANDOM_READ_DST_DEPENDENT_TEST: return "Uniformly random read x%% of dst data, with access dependency.";
		case RANDOM_READ_DST_DEPENDENT_MIXED_TEST: return "Uniformly random read x%% of dst data, with access dependency (mixed).";
		case SEQ_READ_DST_DEPENDENT_TEST: return "SEQUENTIAL_READ_DST_DEPENDENT_TEST";
		default: printf("%s\n", "Error: test type unknown."); exit(1);
	}
}
