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
 *  mapped_blocked_partitioning.c
 *
 *  Created on: Feb 17, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "mapped_blocked_partitioning.h"

void mappedPartitioningBlockedCore(row_t* const input,
											entry_pair_t** output,
											filedescriptor_t fdOutput,
											const offset_t fdOffset,
											const size_t size,
											const size_t numPartitions,
											offset_t* histogram,
											offset_t** virtualToPhysicalOffsets,
											const bool hugePages,
											const size_t shiftOffset) {

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
	const size_t sizeInByte = size * sizeof(entry_pair_t);
	assert((sizeInByte % pageSize) == 0);
	const size_t pagesPerBlock = 1;

	assert(pageSize % CACHELINE_SIZE == 0);
	const size_t entriesPerPage = pageSize / sizeof(entry_pair_t);

	const size_t blockSize = pageSize * pagesPerBlock;
	const size_t entriesPerBlock = entriesPerPage * pagesPerBlock;

	const size_t shift = 64 - log2partitions(numPartitions);

	// input from outside
    row_t* src = input;

    // allocate and initialize the first block of each partition
    size_t blockCount = 0;
    mappedPartitionBlock_t** partitionStarts = malloc(sizeof(*partitionStarts) * numPartitions);
	for(size_t i = 0; i < numPartitions; ++i) {
		partitionStarts[i] = malloc(sizeof(*(partitionStarts[i])));
		partitionStarts[i]->offset = i * blockSize;
		partitionStarts[i]->next = NULL;
		partitionStarts[i]->fillState = 0;
		++blockCount;
	}

	// initialize the pointers that will always point to the partitionBlock to be filled now
	mappedPartitionBlock_t* currentPartitionBlocks[numPartitions];
	memcpy(currentPartitionBlocks, partitionStarts, sizeof(*currentPartitionBlocks) * numPartitions);

	// create the fill mapping, through which the data is written into the partition blocks
    ERRNO_CHECK(
    entry_pair_t* currentBlocks = (entry_pair_t*) mmap(NULL, blockSize * numPartitions, PROT_READ | PROT_WRITE, MAP_SHARED, fdOutput, fdOffset);
    VALID_PTR(currentBlocks, -1, "mmap currentPages", false);
    , "mmap currentPages", true);

	// initialize buffers and set offsets
    __attribute__((aligned(64))) bufferIndex_t buffers[numPartitions];
    for(size_t i = 0; i < numPartitions; ++i) {
    	buffers[i].slot = 0;
    	buffers[i].target = 0;   // store the offset into the block that is currently written (see currentBlocks)
    }

    // partitioning
    row_t* readStream = src;
    uint32_t targetBackup;
    size_t bucketNum;
    for(size_t i = 0; i < size; ++i) {
    	bucketNum = GET_BUCKET(readStream->cols[0], shift);
    	entry_pair_t* bufferStart = (entry_pair_t*) (buffers + bucketNum);
    	if(buffers[bucketNum].slot == INDEX_ENTRIES_PER_CACHELINE - 1) {
    		targetBackup = buffers[bucketNum].target;
    		bufferStart[INDEX_ENTRIES_PER_CACHELINE - 1].first = readStream->cols[0];
    		bufferStart[INDEX_ENTRIES_PER_CACHELINE - 1].second = 1;

			store_nontemp_64B(currentBlocks + (bucketNum * entriesPerBlock) + targetBackup, bufferStart);
			targetBackup += INDEX_ENTRIES_PER_CACHELINE;

			// check whether the block has just been filled
			if(targetBackup == entriesPerBlock) {
				// it has been filled, so allocate a new block
				mappedPartitionBlock_t* newPartitionBlock = malloc(sizeof(*newPartitionBlock));
				// set new offset based on old ones
				newPartitionBlock->offset = currentPartitionBlocks[bucketNum]->offset + (blockSize * numPartitions);
				newPartitionBlock->next = NULL;
				newPartitionBlock->fillState = 0;
				++blockCount;

				// link the new block into the list
				currentPartitionBlocks[bucketNum]->next = newPartitionBlock;
				currentPartitionBlocks[bucketNum]->fillState = entriesPerBlock;
				// rewire the fill mapping to point to the new block
				currentPartitionBlocks[bucketNum] = newPartitionBlock;
		    	ERRNO_CHECK(
		    	mmap(currentBlocks + (bucketNum * entriesPerBlock), blockSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fdOutput, newPartitionBlock->offset + fdOffset);
		    	VALID_PTR(currentBlocks + (bucketNum * entriesPerBlock), -1, "mmap mapping to next page", false);
		    	, "mmap mapping to next page", true);

		    	// reset target
				targetBackup = 0;
			}

			// restore buffer working variables
			buffers[bucketNum].slot = 0;
			buffers[bucketNum].target = targetBackup;
    	}
    	else {
    		bufferStart[buffers[bucketNum].slot].first = readStream->cols[0];
    		bufferStart[buffers[bucketNum].slot].second = 1;
    		++buffers[bucketNum].slot;
    	}
    	++readStream;
    }

    // flush entries remaining in the buffers
    for(size_t i = 0; i < numPartitions; ++i) {
		for(uint32_t b = 0; b < buffers[i].slot; ++b) {
			(currentBlocks + (i * entriesPerBlock))[buffers[i].target] = buffers[i].tuples[b];
			++buffers[i].target;
		}
		currentPartitionBlocks[i]->fillState = buffers[i].target;
    }

    // sanity check
//	#ifdef ERRORCHECK
//		entry_t total = 0;
//		for(size_t i = 0; i < numPartitions; ++i) {
//			mappedPartitionBlock_t* block = partitionStarts[i];
//			do {
//				ERRNO_CHECK(
//				entry_t* fp = mmap(NULL, blockSize, PROT_READ | PROT_WRITE, MAP_SHARED, fdOutput, block->offset + fdOffset);
//				VALID_PTR(fp, -1, "mmap debug", false);
//				, "mmap debug", true);
//				for(size_t j = 0; j < block->fillState; ++j) {
//					total += fp[j];
//				}
//				if(fp) {
//					ERRNO_CHECK(
//					munmap(fp, blockSize);
//					fp = NULL;
//					, "munmap debug", true);
//				}
//			} while((block = block->next));
//		}
//		printf("total =%ld\n", total);
//	#endif


    // map the blocks into a consecutive virtual memory area
	// first, get a mapping into anonymous memory with enough space
	*virtualToPhysicalOffsets = malloc(sizeof(**virtualToPhysicalOffsets) * blockCount);

    ERRNO_CHECK(
    entry_pair_t* fillMapping = (entry_pair_t*) mmap(NULL, blockCount * blockSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE | MAP_ANONYMOUS | (hugePages ? MAP_HUGETLB : 0), -1, 0);
    VALID_PTR(fillMapping, -1, "mmap fillMapping", false);
    , "mmap fillMapping", true);

    size_t j = 0;
    size_t blocksPerPartition[numPartitions];
    memset(blocksPerPartition, 0, sizeof(*blocksPerPartition) * numPartitions);
    for(size_t i = 0; i < numPartitions; ++i) {
    	mappedPartitionBlock_t* block = partitionStarts[i];
    	// map block per block for each partition
    	do {
    		ERRNO_CHECK(
	    	mmap(fillMapping + (j * entriesPerBlock), blockSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fdOutput, block->offset + fdOffset);
	    	VALID_PTR(fillMapping + (j * entriesPerBlock), -1, "mmap fillMapping pagewise", false);
	    	, "mmap fillMapping pagewise", true);
    		// update the mapping info
    		(*virtualToPhysicalOffsets)[j] = block->offset + fdOffset;
			++j;
			++blocksPerPartition[i];
    	} while((block = block->next));
    }

    // complete the last page of each partition by backwards memcpying
    // step 1: fill uncompleted pages
    squeezeInfo_t s[numPartitions];
    size_t partitionOffset = 0;
    for(size_t p = 0; p < numPartitions; ++p) {
    	// how many entries are missing on the last populated page?
    	s[p].entriesMissingInPage = entriesPerBlock - buffers[p].target;
    	s[p].entriesMissingInPage %= entriesPerBlock;
    	s[p].entriesInTotal = ((blocksPerPartition[p] - 1) * entriesPerBlock) + buffers[p].target;
    	// use for histogram generation
    	histogram[p] = s[p].entriesInTotal;
    	// collect the partition start and the fill pointer for fast access later on
    	s[p].startPtr = fillMapping + partitionOffset;
    	partitionOffset += blocksPerPartition[p] * entriesPerBlock;
    	s[p].fillPtr = s[p].startPtr + s[p].entriesInTotal;
    }

    for(size_t p = 1; p < numPartitions; ++p) {
    	histogram[p] += histogram[p-1];
    }

//	#ifdef ERRORCHECK
//	    total = 0;
//		for(size_t p = 0; p < numPartitions; ++p) {
//			entry_t* current = s[p].startPtr;
//			while(current < s[p].fillPtr) {
//				total += *current;
//				++current;
//			}
//		}
//		printf("total = %ld\n", total);
//	#endif

    // go to first uncompleted partition
    size_t partitionToFill = 0;
    for(; partitionToFill < numPartitions - 1; ++partitionToFill) {
    	if(s[partitionToFill].entriesMissingInPage > 0) {
    		break;
    	}
    }

    // as long as there are partitions uncompleted ...
    while(partitionToFill < numPartitions - 1) {
    	// .. and there are partitions left that can fill them ...
    	size_t partitionToCollectFrom = partitionToFill + 1;
    	for(; partitionToCollectFrom < numPartitions; ++partitionToCollectFrom) {
    		if(s[partitionToCollectFrom].entriesInTotal > 0) {
    			break;
    		}
    	}
    	if(partitionToCollectFrom >= numPartitions) {
    		break;
    	}
    	// ... perform the filling
    	const size_t entriesToMove = GET_MIN(s[partitionToFill].entriesMissingInPage, s[partitionToCollectFrom].entriesInTotal);
    	memcpy(s[partitionToFill].fillPtr, s[partitionToCollectFrom].fillPtr - entriesToMove, entriesToMove * sizeof(entry_pair_t));

    	// update statistics
    	s[partitionToFill].entriesMissingInPage -= entriesToMove;
    	s[partitionToFill].entriesInTotal += entriesToMove;
    	s[partitionToFill].fillPtr += entriesToMove;
    	s[partitionToCollectFrom].entriesMissingInPage += entriesToMove;
    	s[partitionToCollectFrom].entriesMissingInPage %= entriesPerBlock;
    	s[partitionToCollectFrom].entriesInTotal -= entriesToMove;
    	s[partitionToCollectFrom].fillPtr -= entriesToMove;

    	if(s[partitionToFill].entriesMissingInPage == 0) {
    		// search next uncompleted one
    		for(; partitionToFill < numPartitions - 1; ++partitionToFill) {
    			if(s[partitionToFill].entriesMissingInPage > 0) {
    				break;
    			}
    		}
    	}
    }

//	#ifdef ERRORCHECK
//		total = 0;
//		for(size_t p = 0; p < numPartitions; ++p) {
//			entry_t* current = s[p].startPtr;
//			while(current < s[p].fillPtr) {
//				total += *current;
//				++current;
//			}
//		}
//		printf("total = %ld\n", total);
//	#endif

    // remap to get rid of the holes
    // find start to remap
	entry_pair_t* remapPtr = s[0].fillPtr;
    size_t partitionToStart = 1;
    for(; partitionToStart < numPartitions; ++partitionToStart) {
    	if(remapPtr < s[partitionToStart].startPtr) {
    		break;
    	}
    	remapPtr = s[partitionToStart].fillPtr;
    }
    DEBUG_PRINT("partitionToStart = %ld\n", partitionToStart);

    size_t currentPartition = partitionToStart;
    for(; currentPartition < numPartitions; ++currentPartition) {
    	const size_t bytesToMap = (s[currentPartition].fillPtr - s[currentPartition].startPtr) * sizeof(entry_pair_t);
    	if(bytesToMap == 0) continue;

    	mappedPartitionBlock_t* block = partitionStarts[currentPartition];
    	size_t entriesProcessed = 0;
    	do {
    		// the list of blocks might have becomes shorter, i.e. there might be blocks
    		// who becomes empty after backwards memcpying
    		if(entriesProcessed >= s[currentPartition].entriesInTotal) break;

    		ERRNO_CHECK(
			mmap(remapPtr, blockSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fdOutput, block->offset + fdOffset);
			VALID_PTR(remapPtr, -1, "mmap remapping", false);
			, "mmap remapping", true);
    		// update mapping info
    		const size_t remappedPage = (remapPtr - fillMapping) * sizeof(*fillMapping) / pageSize;
    		(*virtualToPhysicalOffsets)[remappedPage] = block->offset + fdOffset;

        	remapPtr += entriesPerBlock;
        	entriesProcessed += entriesPerBlock;
    	} while((block = block->next));
    }

	// cleanup currentPages mapping
	if(currentBlocks) {
		ERRNO_CHECK(
		munmap(currentBlocks, blockSize * numPartitions);
		currentBlocks = NULL;
	    , "munmap currentPages", true);
	}

	*output = fillMapping;
}




void mappedPartitioningBlocked(wd_pt* const workingData,
								const size_t size,
								const size_t numPartitions,
								const size_t pagesPerBlock,
								const bool hugePages,
								measurement_t* const measurement) {
	notNull(workingData, "workingData");
	notNull(measurement, "measurement");

	assert(pagesPerBlock >= 1);

//	// creating wdPartitioned_t
//	wdPartitioned_t* newWorkingData = getWdPartitioned();
//	*workingData = newWorkingData;
//
//	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
//	assert(pageSize % CACHELINE_SIZE == 0);
//	const size_t entriesPerPage = pageSize / sizeof(entry_t);
//
//	const size_t blockSize = pageSize * pagesPerBlock;
//	const size_t entriesPerBlock = entriesPerPage * pagesPerBlock;
//
//	const size_t shift = 64 - log2partitions(numPartitions);
//
//	timeval_t start, end;
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
//    // setup file to represent the physical pages
//    filedescriptor_t fd;
//    size_t fileSizeInPages = (size * sizeof(entry_t) / pageSize) + ((size * sizeof(entry_t)) % pageSize ? 1 : 0);
//    if(hugePages) {
//        ERRNO_CHECK(
//        fd = open("/mnt/hugetlbfs/partitions", O_RDWR | O_CREAT, 0666);
//        , "open /mnt/hugetlbfs/partitions", true);
//    }
//    else {
//        ERRNO_CHECK(
//        fd = shm_open("/partitions", O_RDWR | O_CREAT, 0666);
//        , "shm_open", true);
//        ERRNO_CHECK(
//        ftruncate(fd, fileSizeInPages * pageSize);
//        , "ftruncate partitions", true);
//    }
//
//    // fault pages
//	ERRNO_CHECK(
//	uint8_t* tmp = mmap(NULL,
//						fileSizeInPages * pageSize,
//						PROT_READ | PROT_WRITE,
//						MAP_SHARED, fd, 0);
//	VALID_PTR(tmp, -1, "mmap tmp", false);
//	, "mmap tmp", true);
//	for(size_t pageStart = 0; pageStart < fileSizeInPages * pageSize; pageStart += pageSize) {
//		tmp[pageStart] = 1;
//	}
//
//    measure(&start);
//    // allocate and initialize the first block of each partition
//    size_t blockCount = 0;
//    mappedPartitionBlock_t** partitionStarts = malloc(sizeof(*partitionStarts) * numPartitions);
//	for(size_t i = 0; i < numPartitions; ++i) {
//		partitionStarts[i] = malloc(sizeof(*(partitionStarts[i])));
//		partitionStarts[i]->offset = i * blockSize;
//		partitionStarts[i]->next = NULL;
//		partitionStarts[i]->fillState = 0;
//		++blockCount;
//	}
//
//	// initialize the pointers that will always point to the partitionBlock to be filled now
//	mappedPartitionBlock_t* currentPartitionBlocks[numPartitions];
//	memcpy(currentPartitionBlocks, partitionStarts, sizeof(*currentPartitionBlocks) * numPartitions);
//
//	// create the fill mapping, through which the data is written into the partition blocks
//    ERRNO_CHECK(
//    entry_t* currentBlocks = (entry_t*) mmap(NULL, blockSize * numPartitions, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//    VALID_PTR(currentBlocks, -1, "mmap currentPages", false);
//    , "mmap currentPages", true);
//
//	// initialize buffers and set offsets
//    __attribute__((aligned(64))) buffer_t buffers[numPartitions];
//    for(size_t i = 0; i < numPartitions; ++i) {
//    	buffers[i].slot = 0;
//    	buffers[i].target = 0;   // store the offset into the block that is currently written (see currentBlocks)
//    }
//    measure(&end);
//    printTimeDifference(&start, &end, MALLOC_DST, measurement);
//
//    // partition
//    measure(&start);
//
//    row_t* readStream = src;
//    uint32_t targetBackup;
//    size_t bucketNum;
//    for(size_t i = 0; i < size; ++i) {
//    	bucketNum = GET_BUCKET(readStream->cols[0], shift);
//    	entry_t* bufferStart = (entry_t*) (buffers + bucketNum);
//    	if(buffers[bucketNum].slot == TUPLES_PER_CACHELINE - 1) {
//    		targetBackup = buffers[bucketNum].target;
//    		bufferStart[TUPLES_PER_CACHELINE - 1] = readStream->cols[0];
//
//			store_nontemp_64B(currentBlocks + (bucketNum * entriesPerBlock) + targetBackup, bufferStart);
//			targetBackup += TUPLES_PER_CACHELINE;
//
//			// check whether the block has just been filled
//			if(targetBackup == entriesPerBlock) {
//				// it has been filled, so allocate a new block
//				mappedPartitionBlock_t* newPartitionBlock = malloc(sizeof(*newPartitionBlock));
//				// set new offset based on old ones
//				newPartitionBlock->offset = currentPartitionBlocks[bucketNum]->offset + (blockSize * numPartitions);
//				newPartitionBlock->next = NULL;
//				newPartitionBlock->fillState = 0;
//				++blockCount;
//
//				// link the new block into the list
//				currentPartitionBlocks[bucketNum]->next = newPartitionBlock;
//				currentPartitionBlocks[bucketNum]->fillState = entriesPerBlock;
//				// rewire the fill mapping to point to the new block
//				currentPartitionBlocks[bucketNum] = newPartitionBlock;
//		    	ERRNO_CHECK(
//		    	mmap(currentBlocks + (bucketNum * entriesPerBlock), blockSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, newPartitionBlock->offset);
//		    	VALID_PTR(currentBlocks + (bucketNum * entriesPerBlock), -1, "mmap mapping to next page", false);
//		    	, "mmap mapping to next page", true);
//
//		    	// reset target
//				targetBackup = 0;
//			}
//
//			// restore buffer working variables
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
//    // flush entries remaining in the buffers
//    for(size_t i = 0; i < numPartitions; ++i) {
//		for(uint32_t b = 0; b < buffers[i].slot; ++b) {
//			(currentBlocks + (i * entriesPerBlock))[buffers[i].target] = buffers[i].tuples[b];
//			++buffers[i].target;
//		}
//		currentPartitionBlocks[i]->fillState = buffers[i].target;
//    }
//
//    measure(&end);
//    printTimeDifference(&start, &end, MEMCPY, measurement);
//
//    // sanity check
//	#ifdef ERRORCHECK
//		entry_t total = 0;
//		for(size_t i = 0; i < numPartitions; ++i) {
//			mappedPartitionBlock_t* block = partitionStarts[i];
//			do {
//				ERRNO_CHECK(
//				entry_t* fp = mmap(NULL, blockSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, block->offset);
//				VALID_PTR(fp, -1, "mmap debug", false);
//				, "mmap debug", true);
//				for(size_t j = 0; j < block->fillState; ++j) {
//					total += fp[j];
//				}
//				if(fp) {
//					ERRNO_CHECK(
//					munmap(fp, blockSize);
//					fp = NULL;
//					, "munmap debug", true);
//				}
//			} while((block = block->next));
//		}
//		printf("total =%ld\n", total);
//	#endif
//
//
//    // map the blocks into a consecutive virtual memory area
//	// first, get a mapping into anonymous memory with enough space
//    ERRNO_CHECK(
//    entry_t* fillMapping = (entry_t*) mmap(NULL, blockCount * blockSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE | MAP_ANONYMOUS | (hugePages ? MAP_HUGETLB : 0), -1, 0);
//    VALID_PTR(fillMapping, -1, "mmap fillMapping", false);
//    , "mmap fillMapping", true);
//
//    size_t j = 0;
//    size_t blocksPerPartition[numPartitions];
//    memset(blocksPerPartition, 0, sizeof(*blocksPerPartition) * numPartitions);
//    for(size_t i = 0; i < numPartitions; ++i) {
//    	mappedPartitionBlock_t* block = partitionStarts[i];
//    	// map block per block for each partition
//    	do {
//    		ERRNO_CHECK(
//	    	mmap(fillMapping + (j * entriesPerBlock), blockSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, block->offset);
//	    	VALID_PTR(currentBlocks + (j * entriesPerBlock), -1, "mmap fillMapping pagewise", false);
//	    	, "mmap fillMapping pagewise", true);
//			++j;
//			++blocksPerPartition[i];
//    	} while((block = block->next));
//    }
//
//    // complete the last page of each partition by backwards memcpying
//	measure(&start);
//
//    // step 1: fill uncompleted pages
//    squeezeInfo_t s[numPartitions];
//    size_t partitionOffset = 0;
//    size_t* histogram = malloc(sizeof(*histogram) * numPartitions);
//    for(size_t p = 0; p < numPartitions; ++p) {
//    	// how many entries are missing on the last populated page?
//    	s[p].entriesMissingInPage = entriesPerBlock - buffers[p].target;
//    	s[p].entriesMissingInPage %= entriesPerBlock;
//    	s[p].entriesInTotal = ((blocksPerPartition[p] - 1) * entriesPerBlock) + buffers[p].target;
//    	// use for histogram generation
//    	histogram[p] = s[p].entriesInTotal;
//    	// collect the partition start and the fill pointer for fast access later on
//    	s[p].startPtr = fillMapping + partitionOffset;
//    	partitionOffset += blocksPerPartition[p] * entriesPerBlock;
//    	s[p].fillPtr = s[p].startPtr + s[p].entriesInTotal;
//    }
//
//	#ifdef ERRORCHECK
//	    total = 0;
//		for(size_t p = 0; p < numPartitions; ++p) {
//			entry_t* current = s[p].startPtr;
//			while(current < s[p].fillPtr) {
//				total += *current;
//				++current;
//			}
//		}
//		printf("total = %ld\n", total);
//	#endif
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
//    	s[partitionToCollectFrom].entriesMissingInPage %= entriesPerBlock;
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
//	#ifdef ERRORCHECK
//		total = 0;
//		for(size_t p = 0; p < numPartitions; ++p) {
//			entry_t* current = s[p].startPtr;
//			while(current < s[p].fillPtr) {
//				total += *current;
//				++current;
//			}
//		}
//		printf("total = %ld\n", total);
//	#endif
//
//	measure(&end);
//	printTimeDifference(&start, &end, SQUEEZE_AREA_COPY, measurement);
//
//    // remap to get rid of the holes
//    // find start to remap
//	measure(&start);
//
//    entry_t* remapPtr = s[0].fillPtr;
//    size_t partitionToStart = 1;
//    for(; partitionToStart < numPartitions; ++partitionToStart) {
//    	if(remapPtr < s[partitionToStart].startPtr) {
//    		break;
//    	}
//    	remapPtr = s[partitionToStart].fillPtr;
//    }
//    DEBUG_PRINT("partitionToStart = %ld\n", partitionToStart);
//
//    size_t currentPartition = partitionToStart;
//    for(; currentPartition < numPartitions; ++currentPartition) {
//    	const size_t bytesToMap = (s[currentPartition].fillPtr - s[currentPartition].startPtr) * sizeof(entry_t);
//    	if(bytesToMap == 0) continue;
//
//    	mappedPartitionBlock_t* block = partitionStarts[currentPartition];
//    	size_t entriesProcessed = 0;
//    	do {
//    		// the list of blocks might have becomes shorter, i.e. there might be blocks
//    		// who becomes empty after backwards memcpying
//    		if(entriesProcessed >= s[currentPartition].entriesInTotal) break;
//
//    		ERRNO_CHECK(
//			mmap(remapPtr, blockSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, block->offset);
//			VALID_PTR(remapPtr, -1, "mmap remapping", false);
//			, "mmap remapping", true);
//
//        	remapPtr += entriesPerBlock;
//        	entriesProcessed += entriesPerBlock;
//    	} while((block = block->next));
//    }
//
//	measure(&end);
//	printTimeDifference(&start, &end, SQUEEZE_AREA_MMAP, measurement);
//
//	// cleanup currentPages mapping
//	if(currentBlocks) {
//		ERRNO_CHECK(
//		munmap(currentBlocks, blockSize * numPartitions);
//		currentBlocks = NULL;
//	    , "munmap currentPages", true);
//	}
//
//	if(tmp) {
//		ERRNO_CHECK(
//		munmap(tmp, fileSizeInPages * pageSize);
//		tmp = NULL;
//		, "munmap tmp", true);
//	}
//
//	entry_t* dst = fillMapping;
//  setWdPartitioned(newWorkingData, src, dst, fd, histogram, numPartitions, blockCount);
}

void mappedPartitioningBlockedCleanup(wd_pt* const workingData, const size_t pagesPerBlock, const bool hugePages) {
	notNull(workingData, "workingData");

	// expecting wdPartitioned_t
	wdPartitioned_t* wd = (wdPartitioned_t*) getWdPartitionedPtr(workingData);

	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
	assert(pagesPerBlock >= 1);
	const size_t blockSize = pageSize * pagesPerBlock;

    if(SAFE_FREE(wd->src)) {
        printf("Freeing %s\n", getFieldName(MALLOC_SRC));
    }

    if(wd->dst) {
		ERRNO_CHECK(
		munmap(wd->dst, blockSize * wd->blockCount);
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
