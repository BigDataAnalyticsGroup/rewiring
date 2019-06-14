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
 *  mremaped_partitioning.c
 *
 *  Created on: Dec 10, 2015
 *      Author: Felix Martin Schuhknecht
 */

#include "mremaped_partitioning.h"

void mremapedPartitioningBlocked(wd_pt* const workingData,
									const size_t size,
									const size_t numPartitions,
									measurement_t* const measurement) {

	// creating wdChunkedPartitioned_t
	wdChunkedPartitioned_t* newWorkingData = getWdChunkedPartitioned();
	*workingData = newWorkingData;

	const size_t pageSize = HUGE_PAGE_SIZE;
	const size_t entriesPerPage = pageSize / sizeof(entry_t);
	const size_t shift = 64 - log2partitions(numPartitions);

	timeval_t start, end;

    // allocation of src array
    measure(&start);
    row_t* src = NULL;
	posix_memalign((void**) &src, CACHELINE_SIZE, sizeof(*src) * size);
    measure(&end);
    printTimeDifference(&start, &end, MALLOC_SRC, measurement);

    // initialization of src array
    measure(&start);
    srand48(SEED);
    for(size_t i = 0; i < size; i++) {
        src[i].cols[0] = urand64();
    }
    measure(&end);
    printTimeDifference(&start, &end, INIT_MALLOC_SRC, measurement);

    measure(&start);
    // allocate and initialize the first block of each partition
    mremapedPartitionBlock_t** partitions = (mremapedPartitionBlock_t**) malloc(sizeof(*partitions) * numPartitions);
	for(size_t i = 0; i < numPartitions; ++i) {
		partitions[i] = (mremapedPartitionBlock_t*) malloc(sizeof(**partitions));

	    ERRNO_CHECK(
	    partitions[i]->tuples = (entry_t*) mmap(NULL,
	    								pageSize,
										PROT_READ | PROT_WRITE,
										MAP_ANONYMOUS | MAP_PRIVATE,
										-1, 0);
	    VALID_PTR(partitions[i]->tuples, -1, "mmap partitions[i]->tuples", false);
	    , "mmap partitions[i]->tuples", true);

		partitions[i]->numPages = 1;
		partitions[i]->startOfCurrentBlock = partitions[i]->tuples;
	}

	// initialize buffers and set offsets
    __attribute__((aligned(64))) buffer_t buffers[numPartitions];
    for(size_t i = 0; i < numPartitions; ++i) {
    	buffers[i].slot = 0;
    	buffers[i].target = 0;   // store the offset into the page that is currently written (see currentPages)
    }
    measure(&end);
    printTimeDifference(&start, &end, MALLOC_DST, measurement);

    // partition
    measure(&start);
    uint32_t targetBackup;
    size_t bucketNum;
    for(size_t i = 0; i < size; ++i) {
    	bucketNum = GET_BUCKET(src[i].cols[0], shift);
    	entry_t* bufferStart = (entry_t*) (buffers + bucketNum);
    	if(buffers[bucketNum].slot == TUPLES_PER_CACHELINE - 1) {
    		targetBackup = buffers[bucketNum].target;
    		bufferStart[TUPLES_PER_CACHELINE - 1] = src[i].cols[0];

			store_nontemp_64B(partitions[bucketNum]->startOfCurrentBlock + targetBackup, bufferStart);
			targetBackup += TUPLES_PER_CACHELINE;
			if(targetBackup == entriesPerPage) {
				partitions[bucketNum]->tuples = (entry_t*) mremap(partitions[bucketNum]->tuples,
																	partitions[bucketNum]->numPages * pageSize,
																	(partitions[bucketNum]->numPages + 1) * pageSize,
																	MREMAP_MAYMOVE);

				partitions[bucketNum]->startOfCurrentBlock = partitions[bucketNum]->tuples + (partitions[bucketNum]->numPages * entriesPerPage);
				partitions[bucketNum]->numPages++;
				targetBackup = 0;
			}

			// restore
			buffers[bucketNum].slot = 0;
			buffers[bucketNum].target = targetBackup;
    	}
    	else {
    		bufferStart[buffers[bucketNum].slot] = src[i].cols[0];
    		++buffers[bucketNum].slot;
    	}
    }

    for(size_t i = 0; i < numPartitions; ++i) {
		for(uint32_t b = 0; b < buffers[i].slot; ++b) {
			partitions[i]->startOfCurrentBlock[buffers[i].target] = buffers[i].tuples[b];
			++buffers[i].target;
		}
		partitions[i]->fillState = buffers[i].target;
    }

    measure(&end);
    printTimeDifference(&start, &end, MEMCPY, measurement);

    entry_t** chunks = (entry_t**) malloc(sizeof(entry_t*) * numPartitions);
    size_t* chunkSizes = (size_t*) malloc(sizeof(size_t) * numPartitions);
    size_t* histogram = malloc(sizeof(*histogram) * numPartitions);
    memset(histogram, 0, sizeof(*histogram) * numPartitions);

    for(size_t i = 0; i < numPartitions; ++i) {
    	chunks[i] = partitions[i]->tuples;
    	chunkSizes[i] = partitions[i]->fillState;
    	histogram[i] += partitions[i]->fillState;
    }

    setWdChunkedPartitioned(newWorkingData, src, chunks, chunkSizes, numPartitions, (void**) partitions, histogram, numPartitions);
}

void mremapedPartitioningBlockedCleanup(wd_pt* const workingData, const size_t numPartitions) {
	notNull(workingData, "workingData");
	// expecting wdChunkedPartitioned_t
	wdChunkedPartitioned_t* wd = (wdChunkedPartitioned_t*) getWdChunkedPartitionedPtr(workingData);
	const size_t pageSize = HUGE_PAGE_SIZE;

	SAFE_FREE(wd->src);

	for(size_t i = 0; i < numPartitions; ++i) {
		mremapedPartitionBlock_t* block = (mremapedPartitionBlock_t*) wd->partitionStarts[i];

		ERRNO_CHECK(
		munmap(block->tuples, block->numPages * pageSize);
		block->tuples = NULL;
		, "munmap block->tuples", true);
	}

	SAFE_FREE(wd->chunks);
	SAFE_FREE(wd->sizes);
	SAFE_FREE(wd->partitionStarts);
	SAFE_FREE(wd->histogram);
}

