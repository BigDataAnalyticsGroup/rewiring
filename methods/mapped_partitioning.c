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
 *  mapped_partitioning.c
 *
 *  Created on: Feb 17, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "mapped_partitioning.h"


void mappedPartitioning(wd_pt* const workingData, const size_t size, const size_t numPartitions, const bool hugePages, measurement_t* const measurement) {
	// creating wdPartitioned_t
//	wdPartitioned_t* newWorkingData = getWdPartitioned();
//	*workingData = newWorkingData;
//
//	assert(sizeof(entry_t) == 2 * sizeof(uint32_t));
//
//	timeval_t start, end;
//
//	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
//    const size_t entriesPerPage = pageSize / sizeof(entry_t);
//
//    // allocation of src array
//    measure(&start);
//    row_t* src = NULL;
//	posix_memalign((void**) &src, 64, sizeof(*src) * size);
//    measure(&end);
//    printTimeDifference(&start, &end, MALLOC_SRC, measurement);
//
//    // initialization of src array
//    measure(&start);
//    srand48(SEED);
//    row_t* wrtPtr = src;
//    for(size_t i = 0; i < size; i++) {
//        wrtPtr[i].cols[0] = urand64();
//    }
//    measure(&end);
//    printTimeDifference(&start, &end, INIT_MALLOC_SRC, measurement);
//
//	// initialize buffers and set offsets
//    __attribute__((aligned(64))) buffer_t buffers[numPartitions];
//	for(size_t i = 0; i < numPartitions; ++i) buffers[i].slot = 0;
//
//	// create a file that will contain the partition data
//    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (pageSize - ((size * sizeof(entry_t)) % pageSize));
//    const size_t entriesRoundedToPageBoundary = sizeRoundedToPageBoundary / sizeof(entry_t);
//
//	filedescriptor_t fd;
//	if(hugePages) {
//	    ERRNO_CHECK(
//	    fd = open("/mnt/hugetlbfs/partitions", O_RDWR | O_CREAT, 0666);
//	    , "open /mnt/hugetlbfs/partitions", true);
//	}
//	else {
//	    ERRNO_CHECK(
//	    fd = shm_open("/partitions", O_RDWR | O_CREAT, 0666);
//	    , "shm_open /partitions", true);
//	    ERRNO_CHECK(
//	    ftruncate(fd, sizeRoundedToPageBoundary * numPartitions);
//	    , "ftruncate src", true);
//	}
//
//    ERRNO_CHECK(
//    entry_t* fillMapping = (entry_t*) mmap(NULL, sizeRoundedToPageBoundary * numPartitions, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, fd, 0);
//    VALID_PTR(fillMapping, -1, "mmap fillMapping", false);
//    , "mmap fillMapping", true);
//
//    // assume uniform distribution and prefault the first pages of each partition
//    size_t numFaultedPages = 0;
//    for(size_t p = 0; p < numPartitions; ++p) {
//		char* partitionStart = ((char*) fillMapping) + sizeRoundedToPageBoundary * p;
//    	for(size_t i = 0; i < sizeRoundedToPageBoundary / numPartitions; i += pageSize) {
//    		*(partitionStart + pageSize) = 0;
//    		++numFaultedPages;
//		}
//    }
//    printf("numFaultedPages = %ld\n", numFaultedPages);
//
//    // point to the first entry of each partition
//    for(size_t i = 0; i < numPartitions; ++i) {
//    	buffers[i].target = 0;
//    }
//
//    // partition
//	const size_t shift = 64 - log2partitions(numPartitions);
//    measure(&start);
//
//    DEBUG_PRINT("before partitioning\n");
//
//    row_t* readStream = src;
//    uint32_t targetBackup;
//    size_t bucketNum;
//
//    for(size_t i = 0; i < size; ++i) {
//    	bucketNum = GET_BUCKET(readStream->cols[0], shift);
//    	entry_t* bufferStart = (entry_t*) (buffers + bucketNum);
//    	if(buffers[bucketNum].slot == TUPLES_PER_CACHELINE - 1) {
//    		targetBackup = buffers[bucketNum].target;
//    		bufferStart[TUPLES_PER_CACHELINE - 1] = readStream->cols[0];
//			store_nontemp_64B(fillMapping + (entriesRoundedToPageBoundary * bucketNum) + targetBackup, bufferStart);
//			targetBackup += TUPLES_PER_CACHELINE;
//			// restore
//			buffers[bucketNum].slot = 0;
//			buffers[bucketNum].target = targetBackup;
//    	}
//    	else {
//    		bufferStart[buffers[bucketNum].slot] = readStream->cols[0];
//    		++buffers[bucketNum].slot;
//    	}
//    	++readStream;
//    }
//
//    // flush non-empty buffers
//    for(size_t i = 0; i < numPartitions; ++i) {
//    	for(uint32_t s = 0; s < buffers[i].slot; ++s) {
//    		*(fillMapping + (entriesRoundedToPageBoundary * i) + buffers[i].target) = buffers[i].tuples[s];
//    		++buffers[i].target;
//    	}
//    }
//
//    // keep the histogram information
//    size_t* histogram = malloc(sizeof(*histogram) * numPartitions);
//    for(size_t p = 0; p < numPartitions; ++p) {
//    	histogram[p] = buffers[p].target;
//    }
//
//    DEBUG_PRINT("after partitioning\n");
//
//	measure(&end);
//	printTimeDifference(&start, &end, MEMCPY, measurement);
//
//	measure(&start);
//
//    // squeeze memory to get rid of the empty pages between partitions
//
//	#ifdef ERRORCHECK
//		// sanity check before we start
//		size_t histo[numPartitions];
//		memset(histo, 0, sizeof(size_t) * numPartitions);
//		for(size_t p = 0; p < numPartitions; ++p) {
//			// check whether the data belongs to the partitions
//			entry_t* beginOfPartition = fillMapping + (entriesRoundedToPageBoundary * p);
//			for(size_t i = 0; i < buffers[p].target; ++i) {
//				size_t bucket = GET_BUCKET(beginOfPartition[i], shift);
//				++histo[bucket];
//				if(p != bucket) {
//					DEBUG_PRINT("Error: GET_BUCKET(beginOfPartition[%ld], shift) = %ld, but we are in partition %ld!\n", i, bucket, p);
//				}
//			}
//		}
//	#endif
//
//
//	measure(&start);
//
//    // step 1: fill uncompleted pages
//    squeezeInfo_t s[numPartitions];
//    for(size_t p = 0; p < numPartitions; ++p) {
//    	// how many entries are missing on the last populated page?
//    	s[p].entriesMissingInPage = entriesPerPage - (buffers[p].target % entriesPerPage);
//    	s[p].entriesMissingInPage %= entriesPerPage;
//    	// collect the partition start and the fill pointer for fast access later on
//    	s[p].startPtr = fillMapping + (entriesRoundedToPageBoundary * p);
//    	s[p].fillPtr = s[p].startPtr + buffers[p].target;
//    	// store total fillstate as well again
//    	s[p].entriesInTotal = buffers[p].target;
//    }
//
//    // go to first uncompleted partition
//    size_t partitionToFill = 0;
//    for(; partitionToFill < numPartitions - 1; ++partitionToFill) {
//    	if(s[partitionToFill].entriesMissingInPage > 0) {
//    		break;
//    	}
//    }
//
//    // as long as there are partitions uncompleted ...
//    while(partitionToFill < numPartitions - 1) {
//    	// .. and there are partitions left that can fill them ...
//    	size_t partitionToCollectFrom = partitionToFill + 1;
//    	for(; partitionToCollectFrom < numPartitions; ++partitionToCollectFrom) {
//    		if(s[partitionToCollectFrom].entriesInTotal > 0) {
//    			break;
//    		}
//    	}
//    	if(partitionToCollectFrom >= numPartitions) {
//    		break;
//    	}
//    	// ... perform the filling
//    	const size_t entriesToMove = GET_MIN(s[partitionToFill].entriesMissingInPage, s[partitionToCollectFrom].entriesInTotal);
//    	memcpy(s[partitionToFill].fillPtr, s[partitionToCollectFrom].fillPtr - entriesToMove, entriesToMove * sizeof(entry_t));
//
//    	// update statistics
//    	s[partitionToFill].entriesMissingInPage -= entriesToMove;
//    	s[partitionToFill].entriesInTotal += entriesToMove;
//    	s[partitionToFill].fillPtr += entriesToMove;
//    	s[partitionToCollectFrom].entriesMissingInPage += entriesToMove;
//    	s[partitionToCollectFrom].entriesMissingInPage %= entriesPerPage;
//    	s[partitionToCollectFrom].entriesInTotal -= entriesToMove;
//    	s[partitionToCollectFrom].fillPtr -= entriesToMove;
//
//    	if(s[partitionToFill].entriesMissingInPage == 0) {
//    		// search next uncompleted one
//    		for(; partitionToFill < numPartitions - 1; ++partitionToFill) {
//    			if(s[partitionToFill].entriesMissingInPage > 0) {
//    				break;
//    			}
//    		}
//    	}
//    }
//
//	measure(&end);
//	printTimeDifference(&start, &end, SQUEEZE_AREA_COPY, measurement);
//
//    // sanity checks after squeezing
//	#ifdef ERRORCHECK
//		size_t histo2[numPartitions];
//		size_t lastBucket = 0;
//		bool endReached = false;
//		memset(histo2, 0, sizeof(size_t) * numPartitions);
//		for(size_t p = 0; p < numPartitions; ++p) {
//			entry_t* inclusiveStart = fillMapping + (p * entriesRoundedToPageBoundary);
//			entry_t* exclusiveEnd = s[p].fillPtr;
//			if(p < numPartitions - 1) {
//				if((size_t)(s[p].fillPtr) % pageSize != 0) {
//					if(!endReached) {
//						endReached = true;
//					}
//					else {
//						DEBUG_PRINT("Error: pointer at s[%ld].fillPtr = %p, entries in total = %ld\n", p, (void*) s[p].fillPtr, s[p].entriesInTotal);
//					}
//				}
//			}
//			while(inclusiveStart < exclusiveEnd) {
//				const size_t bucket = GET_BUCKET(*inclusiveStart, shift);
//				if(bucket > lastBucket) {
//					lastBucket = bucket;
//				}
//				else if(bucket < lastBucket) {
//					DEBUG_PRINT("Error: Jumped back.\n");
//				}
//				++histo2[bucket];
//				++inclusiveStart;
//			}
//		}
//		for(size_t p = 0; p < numPartitions; ++p) {
//			assert(histo[p] == histo2[p]);
//		}
//	#endif
//
//	measure(&end);
//
//	measure(&start);
//
//    // remap to get rid of the holes
//    // find start to remap
//    entry_t* remapPtr = s[0].fillPtr;
//    size_t partitionToStart = 1;
//    for(; partitionToStart < numPartitions; ++partitionToStart) {
//    	if(remapPtr < s[partitionToStart].startPtr) {
//    		break;
//    	}
//    	remapPtr = s[partitionToStart].fillPtr;
//    }
//    DEBUG_PRINT("remapPtr = %p\n", (void*) remapPtr);
//    DEBUG_PRINT("partitionToStart = %ld\n", partitionToStart);
//
//    size_t currentPartition = partitionToStart;
//    for(; currentPartition < numPartitions; ++currentPartition) {
//    	const size_t bytesToMap = (s[currentPartition].fillPtr - s[currentPartition].startPtr) * sizeof(entry_t);
//    	if(bytesToMap == 0) continue;
//    	ERRNO_CHECK(
//    	mmap(remapPtr, bytesToMap, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, (s[currentPartition].startPtr - fillMapping) * sizeof(entry_t));
//    	VALID_PTR(fillMapping, -1, "mmap remapping", false);
//    	, "mmap remapping", true);
//    	remapPtr += bytesToMap / sizeof(entry_t);
//    }
//
//	measure(&end);
//	printTimeDifference(&start, &end, SQUEEZE_AREA_MMAP, measurement);
//
//	#ifdef ERRORCHECK
//		// final sanity check
//		size_t histo3[numPartitions];
//		lastBucket = 0;
//		memset(histo3, 0, sizeof(size_t) * numPartitions);
//		for(size_t i = 0; i < size; ++i) {
//			const size_t bucket = GET_BUCKET(fillMapping[i], shift);
//			if(bucket > lastBucket) {
//				lastBucket = bucket;
//			}
//			else if(bucket < lastBucket) {
//				DEBUG_PRINT("Error: Jumped back.\n");
//			}
//			++histo3[bucket];
//		}
//		for(size_t p = 0; p < numPartitions; ++p) {
//			assert(histo[p] == histo3[p]);
//		}
//	#endif
//
//	printf("\n");
//
//	setWdPartitioned(newWorkingData, src, fillMapping, fd, histogram, numPartitions, 0);
}

void mappedPartitioningCleanup(wd_pt* const workingData, const size_t size, const size_t numPartitions, const bool hugePages) {
	notNull(workingData, "workingData");
	// expecting wdPartitioned_t
	wdPartitioned_t* wd = (wdPartitioned_t*) getWdPartitionedPtr(workingData);

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
    const size_t sizeRoundedToPageBoundary = size * sizeof(entry_t) + (pageSize - ((size * sizeof(entry_t)) % pageSize));

    if(SAFE_FREE(wd->src)) {
        printf("Freeing %s\n", getFieldName(MALLOC_SRC));
    }

    if(wd->dst) {
		ERRNO_CHECK(
		munmap(wd->dst, sizeRoundedToPageBoundary * numPartitions);
		wd->dst = NULL;
	    , "munmap dst", true);
        printf("Freeing %s\n", getFieldName(MMAP_DST));

        if(hugePages) {
        	ERRNO_CHECK(
        	close(wd->fd);
        	wd->fd = -1;
        	, "close", true);
        	ERRNO_CHECK(
        	unlink("/mnt/hugetlbfs/partitions");
        	, "unlinck /mnt/hugetlbfs/partitions", true);
        	printf("Freeing %s\n", getFieldName(SHM_DST_HUGE));
        }
        else {
            ERRNO_CHECK(
            shm_unlink("/partitions");
            , "shm_unlick /partitions", true);
            printf("Freeing %s\n", getFieldName(SHM_DST));
        }
    }
    SAFE_FREE(wd->histogram);
}












