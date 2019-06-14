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
 * tests.h
 *
 *  Created on: Mar 06, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef TESTS_H_
#define TESTS_H_

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    SEQ_READ_SRC_TEST = 1,
    SEQ_READ_DST_TEST = 2,

	RAND_READ_SRC_TEST = 3,
    RAND_READ_DST_TEST = 4,

	SEQ_WRITE_SRC_TEST = 5,
    SEQ_WRITE_DST_TEST = 6,

	RAND_WRITE_SRC_TEST = 7,
    RAND_WRITE_DST_TEST = 8,

    SEQ_READ_DST_TWICE_TEST = 9,

	SEQ_READ_DST_FROM_SEVERAL_CHUNKS = 10,
	SEQ_READ_DST_MERGE_FROM_CHUNKS = 11,

	RADIX_SORT_DST_TEST = 12,
	RADIX_SORT_PER_PARTITION_CONSECUTIVE = 13,
	RADIX_SORT_PER_PARTITION_INDIRECTION = 14,

	VECTOR_INSERT_TEST = 15,

	PAGE_FAULT_TEST = 18,

	RAND_READ_DST_INDIRECTION_TEST = 19,

	SEQ_WRITE_DST_TWICE_TEST = 20,

	RANDOM_READ_DST_DEPENDENT_TEST = 21,
	RANDOM_READ_DST_DEPENDENT_MIXED_TEST = 22,

	SEQ_READ_DST_DEPENDENT_TEST = 23

} testType_t;
#define numTestType 23

const char* getTestName(const testType_t test_type);

#endif /* TESTS_H_ */
