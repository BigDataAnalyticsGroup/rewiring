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
 * mremaped_partitioning.h
 *
 *  Created on: Dec 10, 2015
 *      Author: Felix Martin Schuhknecht
 */

#ifndef MREMAPED_PARTITIONING_H_
#define MREMAPED_PARTITIONING_H_

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
#include "../timediff.h"
#include "../types.h"
#include "../fields.h"
#include "../error.h"
#include "../timefile.h"
#include "../useful_functions.h"
#include "../tests/sequential_read_test.h"

#define TUPLES_PER_BLOCK (HUGE_PAGE_SIZE / sizeof(entry_t))

void mremapedPartitioningBlocked(wd_pt* const workingData, const size_t size, const size_t numPartitions, measurement_t* const measurement);
void mremapedPartitioningBlockedCleanup(wd_pt* const workingData, const size_t numPartitions);

#endif /* MREMAPED_PARTITIONING_H_ */
