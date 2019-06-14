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
 * mapped_blocked_partitioning_adaptive.h
 *
 *  Created on: Mar 06, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef MAPPED_BLOCKED_PARTITIONING_ADAPTIVE_H_
#define MAPPED_BLOCKED_PARTITIONING_ADAPTIVE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <math.h>
#include "../timediff.h"
#include "../types.h"
#include "../fields.h"
#include "../error.h"
#include "../timefile.h"
#include "../useful_functions.h"
#include "../tests/sequential_read_test.h"

// requires sizeof(entry_t) == 2 * sizeof(uint32_t) == 4 * sizeof(uint_64_t)
typedef struct {
    entry_t tuples[TUPLES_PER_CACHELINE - 1];
    uint16_t slot;
    uint16_t blockSize;
    uint32_t target;
} bufferAdaptive_t;

void mappedPartitioningBlockedAdaptive(wd_pt* const workingData, const size_t size, const size_t numPartitions, measurement_t* const measurement);
void mappedPartitioningBlockedAdaptiveCleanup(wd_pt* const workingData);

#endif /* MAPPED_BLOCKED_PARTITIONING_ADAPTIVE_H_ */
