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
 *  blocked_partitioning.c
 *
 *  Created on: Feb 17, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "blocked_partitioning.h"

void traditionalPartitioningBlocked(wd_pt* const workingData, const size_t size, const size_t numPartitions, measurement_t* const measurement) {
	// creating wdChunkedPartitioned_t
	wdChunkedPartitioned_t* newWorkingData = getWdChunkedPartitioned();
	*workingData = newWorkingData;

	const size_t pageSize = HUGE_PAGE_SIZE;			// TODO: support small pages
	assert(pageSize % CACHELINE_SIZE == 0);
	const size_t entriesPerPage = pageSize / sizeof(entry_t);

	const size_t shift = 64 - log2partitions(numPartitions);

	timeval_t start, end;

	// allocate pool
	entry_t* pool = NULL;
	const size_t poolSize = (sizeof(*pool) * size) + (numPartitions * pageSize);	// one page more per partition
	const size_t entriesInPool = poolSize / sizeof(entry_t);
	posix_memalign((void**) &pool, pageSize, poolSize);
	newWorkingData->pool = pool;
	entry_t* firstFreePage = pool;
	// prefault pool
	for(size_t i = 0; i < entriesInPool; i += entriesPerPage) {
		*(pool + i) = 1;
	}

    // allocation of src array
    measure(&start);
    row_t* src = NULL;
	posix_memalign((void**) &src, 64, sizeof(*src) * size);
    measure(&end);
    printTimeDifference(&start, &end, MALLOC_SRC, measurement);

    // initialization of src array
    measure(&start);
    srand48(SEED);
    row_t* wrtPtr = src;
    for(size_t i = 0; i < size; i++) {
        wrtPtr[i].cols[0] = urand64();
    }
    measure(&end);
    printTimeDifference(&start, &end, INIT_MALLOC_SRC, measurement);

    measure(&start);
    // allocate and initialize the first block of each partition
    size_t pageCount = 0;
    partitionBlock_t** partitionStarts = (partitionBlock_t**) malloc(sizeof(partitionBlock_t*) * numPartitions);
	for(size_t i = 0; i < numPartitions; ++i) {
		partitionStarts[i] = (partitionBlock_t*) malloc(sizeof(partitionBlock_t));

		partitionStarts[i]->tuples = firstFreePage;
		firstFreePage += entriesPerPage;
		++pageCount;
		partitionStarts[i]->next = NULL;
		partitionStarts[i]->fillState = 0;
	}

	// initialize the points that will always point to the partitionBlock to be filled now
	partitionBlock_t* currentBlocks[numPartitions];
	memcpy(currentBlocks, partitionStarts, sizeof(partitionBlock_t*) * numPartitions);

	// initialize the pointers that will always point to the page to be filled now, to have access without indirection
	entry_t* currentPages[numPartitions];
	for(size_t i = 0; i < numPartitions; ++i) {
		currentPages[i] = partitionStarts[i]->tuples;
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

    row_t* readStream = src;
    uint32_t targetBackup;
    size_t bucketNum;
    for(size_t i = 0; i < size; ++i) {
    	bucketNum = GET_BUCKET(readStream->cols[0], shift);
    	entry_t* bufferStart = (entry_t*) (buffers + bucketNum);
    	if(buffers[bucketNum].slot == TUPLES_PER_CACHELINE - 1) {
    		targetBackup = buffers[bucketNum].target;
    		bufferStart[TUPLES_PER_CACHELINE - 1] = readStream->cols[0];

			store_nontemp_64B(currentPages[bucketNum] + targetBackup, bufferStart);
			targetBackup += TUPLES_PER_CACHELINE;
			if(targetBackup == entriesPerPage) {
				partitionBlock_t* newPartitionBlock = (partitionBlock_t*) malloc(sizeof(partitionBlock_t));
				posix_memalign((void**) &(newPartitionBlock->tuples), pageSize, pageSize);
				newPartitionBlock->tuples = firstFreePage;
				firstFreePage += entriesPerPage;
				++pageCount;
				newPartitionBlock->next = NULL;
				newPartitionBlock->fillState = 0;

				currentBlocks[bucketNum]->next = newPartitionBlock;
				currentBlocks[bucketNum]->fillState = entriesPerPage;

				currentBlocks[bucketNum] = newPartitionBlock;
				currentPages[bucketNum] = newPartitionBlock->tuples;
				targetBackup = 0;
			}

			// restore
			buffers[bucketNum].slot = 0;
			buffers[bucketNum].target = targetBackup;
    	}
    	else {
    		bufferStart[buffers[bucketNum].slot] = readStream->cols[0];
    		++buffers[bucketNum].slot;
    	}
    	++readStream;
    }

    printf("pageCount = %ld\n", pageCount);

    for(size_t i = 0; i < numPartitions; ++i) {
		for(uint32_t b = 0; b < buffers[i].slot; ++b) {
			currentPages[i][buffers[i].target] = buffers[i].tuples[b];
			++buffers[i].target;
		}
		currentBlocks[i]->fillState = buffers[i].target;
    }

    measure(&end);
    printTimeDifference(&start, &end, MEMCPY, measurement);

    entry_t** chunks = (entry_t**) malloc(sizeof(entry_t*) * pageCount);
    size_t* chunkSizes = (size_t*) malloc(sizeof(size_t) * pageCount);
    size_t* histogram = malloc(sizeof(*histogram) * numPartitions);
    memset(histogram, 0, sizeof(*histogram) * numPartitions);
    size_t j = 0;
    for(size_t i = 0; i < numPartitions; ++i) {
    	partitionBlock_t* block = partitionStarts[i];
    	do {
    		chunks[j] = block->tuples;
    		chunkSizes[j] = block->fillState;
    		histogram[i] += block->fillState;
			++j;
    	} while((block = block->next));
    }

	#ifdef ERRORCHECK
		size_t lastBucket = 0;
		for(size_t i = 0; i < numPartitions; ++i) {
			partitionBlock_t* block = partitionStarts[i];
			do {
				for(size_t j = 0; j < block->fillState; ++j) {
					entry_t entry = block->tuples[j];
					size_t bucket = GET_BUCKET(entry, shift);
					if(bucket > lastBucket) {
						lastBucket = bucket;
					}
					else if(bucket < lastBucket) {
						DEBUG_PRINT("Error: jumped back. Backet = %ld, but lastBucket = %ld\n", bucket, lastBucket);
					}
				}
			} while((block = block->next));
		}
		DEBUG_PRINT("Everything OK!\n");
	#endif

    setWdChunkedPartitioned(newWorkingData, src, chunks, chunkSizes, pageCount, (void**) partitionStarts, histogram, numPartitions);
}

void traditionalPartitioningBlockedCleanup(wd_pt* const workingData, const size_t numPartitions) {
	notNull(workingData, "workingData");
	// expecting wdChunkedPartitioned_t
	wdChunkedPartitioned_t* wd = (wdChunkedPartitioned_t*) getWdChunkedPartitionedPtr(workingData);

	SAFE_FREE(wd->src);

//	for(size_t i = 0; i < numPartitions; ++i) {
//		partitionBlock_t* block = wd->partitionStarts[i];
//		do {
//			SAFE_FREE(block->tuples);
//		} while((block = block->next));
//	}
	SAFE_FREE(wd->pool);
	SAFE_FREE(wd->chunks);
	SAFE_FREE(wd->sizes);
	SAFE_FREE(wd->partitionStarts);
	SAFE_FREE(wd->histogram);
}


entry_t* mergeChunks(entry_t** chunks, const size_t* const sizes, const size_t numChunks, size_t* const totalSize, const field_t f, measurement_t* const measurement) {
	notNull(chunks, "chunks");
	notNull(sizes, "sizes");
	notNull(measurement, "measurement");

	timeval_t start, end;
	measure(&start);

	printf("Calculating sizes.\n");
	size_t ts = 0;
	for(size_t i = 0; i < numChunks; ++i) {
		ts += sizes[i];
	}
	if(totalSize) *totalSize = ts;

	printf("Allocating merge space sizes.\n");
	// copy into consecutive memory area
	entry_t* p = (entry_t*) malloc(ts * sizeof(entry_t));
	entry_t* fillPtr = p;

	printf("Merging.\n");
	for(size_t c = 0; c < numChunks; ++c) {
		memcpy(fillPtr, chunks[c], sizes[c] * sizeof(entry_t));
		fillPtr += sizes[c];
	}
	printf("Merging done.\n");

	measure(&end);
    printTimeDifference(&start, &end, f, measurement);

	return p;
}

