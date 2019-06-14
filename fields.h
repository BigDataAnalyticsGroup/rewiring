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
 * fields.h
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef FIELDS_H_
#define FIELDS_H_

#include <stdio.h>
#include <stdlib.h>

// must be enumerated sequentially, starting from 0
typedef enum {
    MALLOC_SRC = 0,
    INIT_MALLOC_SRC = 1,
    MALLOC_DST = 2,
    MEMCPY = 3,
    SHM_SRC = 4,
	MMAP_SRC = 5,
    INIT_SHM_SRC = 6,
    MMAP_DST = 7,
	REMAP_DST = 8,
    SHM_SRC_HUGE = 9,
    READ_SEQ_SRC = 10,
    READ_SEQ_DST = 11,
	READ2_SEQ_DST = 12,
    READ_RAND_SRC = 13,
    READ_RAND_DST = 14,
    WRITE_SEQ_SRC = 15,
    WRITE_SEQ_DST = 16,
    WRITE_RAND_SRC = 17,
    WRITE_RAND_DST = 18,
    WRITE_SEQ_DST_PER_ELEM = 19,
	BUILD_HISTO = 20,
	SQUEEZE_AREA_COPY = 21,
	SQUEEZE_AREA_MMAP = 22,
	SHM_DST = 23,
	SHM_DST_HUGE = 24,
	MERGE = 25,
	RADIX_SORT = 26,
	INDIRECTION = 27,
	VECTOR_INSERT = 28,
	CRACK_IN_PLACE = 29,
	INDEX_INSERT = 30,
	INDEX_ACCESS = 31,
	WRITE2_SEQ_DST = 32
} field_t;
#define num_fields 33

const char* getFieldName(field_t field);

#endif /* FIELDS_H_ */
