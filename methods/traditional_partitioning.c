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
 *  traditional_partitioning.c
 *
 *  Created on: Feb 17, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "traditional_partitioning.h"

void traditionalPartitioningWithHistogram(wd_pt* const workingData,
											const size_t size,
											const size_t numPartitions,
											const bool initialize,
											measurement_t* const measurement) {
	// creating wdPartitioned_t
	wdPartitioned_t* newWorkingData = getWdPartitioned();
	*workingData = newWorkingData;

	assert(sizeof(entry_t) == 2 * sizeof(uint32_t));

	timeval_t start, end;

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
    	// just initialize key, the rest is just for space filling
        wrtPtr[i].cols[0] = urand64();
    }
    measure(&end);
    printTimeDifference(&start, &end, INIT_MALLOC_SRC, measurement);

	// build histogram
    measure(&start);
    __attribute__((aligned(64))) size_t buckets[numPartitions];
	memset(buckets, 0, numPartitions * sizeof(size_t));
	const size_t shift = 64 - log2partitions(numPartitions);

	for(size_t i = 0; i < size; ++i){
		++buckets[GET_BUCKET(src[i].cols[0], shift)];
	}
	size_t* histogram = (size_t*) malloc(sizeof(size_t) * numPartitions);
	memcpy(histogram, buckets, sizeof(size_t) * numPartitions);

	for(size_t i = 1; i < numPartitions; ++i){
		buckets[i] += buckets[i-1];
	}
    __attribute__((aligned(64))) size_t originalBuckets[numPartitions];
	memcpy(originalBuckets, buckets, sizeof(size_t) * numPartitions);

	for(size_t i = 0; i < numPartitions; ++i) {
		buckets[i] = buckets[i] - (buckets[i] % TUPLES_PER_CACHELINE);
	}
    measure(&end);
    printTimeDifference(&start, &end, BUILD_HISTO, measurement);

	// initialize buffers and set offsets
    measure(&start);
    __attribute__((aligned(64))) buffer_t buffers[numPartitions];
    for(size_t i = 0; i < numPartitions; ++i) {
    	buffers[i].slot = 0;
        buffers[i].target = buckets[i];
    }

    // allocation of dst array
    entry_t* dst = NULL;
    const size_t dstSize = sizeof(entry_t) * size + 64 * numPartitions;
	posix_memalign((void**) &dst, 64, dstSize);
	measure(&end);
    printTimeDifference(&start, &end, MALLOC_DST, measurement);

    if(initialize) {
    	// fault pages
    	uint8_t* tmp = (uint8_t*) dst;
    	for(size_t pageStart = 0; pageStart < dstSize; pageStart += SMALL_PAGE_SIZE) {
    		tmp[pageStart] = 1;
    	}
    }

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
			targetBackup -= TUPLES_PER_CACHELINE;
			store_nontemp_64B(dst + targetBackup, bufferStart);
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

    for(long i = numPartitions - 1; i >= 0; --i) {
    	if(i > 0 && buffers[i].target < originalBuckets[i-1]) {
    		// fix the wrongly written elements
    		size_t endPartition = originalBuckets[i] - 1;
    		for(size_t j = buffers[i].target; j < originalBuckets[i-1]; ++j) {
    			dst[endPartition--] = dst[j];
    		}
    		buffers[i].target = endPartition + 1;
    	}
    	for(uint32_t b = 0; b < buffers[i].slot; ++b) {
    		// rollback to end after completing the unpadded writes in the beginning
    		if(buffers[i].target <= (i > 0 ? originalBuckets[i-1] : 0)) {
    			buffers[i].target = originalBuckets[i];
    		}
    		dst[buffers[i].target - 1] = buffers[i].tuples[b];
    		--buffers[i].target;
    	}
    }

    measure(&end);
    printTimeDifference(&start, &end, MEMCPY, measurement);

    // sanity check
	#ifdef ERRORCHECK
		size_t lastBucket = 0;
		for(size_t i = 0; i < size; ++i) {
			entry_t entry = dst[i];
			size_t bucket = GET_BUCKET(entry, shift);
			if(bucket > lastBucket) {
				lastBucket = bucket;
			}
			else if(bucket < lastBucket) {
				DEBUG_PRINT("Error: jumped back. Backet = %ld, but lastBucket = %ld\n", bucket, lastBucket);
			}
		}
	#endif

    printf("\n");

    setWdPartitioned(newWorkingData, src, dst, -1, histogram, numPartitions, 0);
}

void traditionalPartitionedCleanup(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	// expecting wdPartitioned_t
	wdPartitioned_t* wd = (wdPartitioned_t*) getWdPartitionedPtr(workingData);

	if(SAFE_FREE(wd->src)) printf("Freeing %s\n", getFieldName(MALLOC_SRC));
	if(SAFE_FREE(wd->dst)) printf("Freeing %s\n", getFieldName(MALLOC_DST));
	if(SAFE_FREE(wd->histogram)) printf("Freeing %s\n", getFieldName(BUILD_HISTO));

    SAFE_FREE(wd);
}











