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
 *  mapped_blocked_partitioning_adaptive.c
 *
 *  Created on: Feb 17, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "mapped_blocked_partitioning_adaptive.h"

void mappedPartitioningBlockedAdaptive(wd_pt* const workingData, const size_t size, const size_t numPartitions, measurement_t* const measurement) {
	notNull(workingData, "workingData");
	notNull(measurement, "measurement");

	// creating wdPartitioned_t
	wdPartitioned_t* newWorkingData = getWdPartitioned();
	*workingData = newWorkingData;

	assert(HUGE_PAGE_SIZE % CACHELINE_SIZE == 0);
	assert(SMALL_PAGE_SIZE % CACHELINE_SIZE == 0);

	const size_t shift = 64 - log2partitions(numPartitions);

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
        wrtPtr[i].cols[0] = urand64();
    }
    measure(&end);
    printTimeDifference(&start, &end, INIT_MALLOC_SRC, measurement);

    // setup file to represent the physical pages both for huge and small pages, as we will probably need both
    // as we do not know yet how many we need, the file sizes remain zero here
    // initialize huge page file
    size_t fdHugeSize = 0;
	ERRNO_CHECK(
	filedescriptor_t fdHuge = open("/mnt/hugetlbfs/partitionsHuge", O_RDWR | O_CREAT, ACCESS_PERMISSION);
	, "open /mnt/hugetlbfs/partitionsHuge", true);
	// initialize small page file
	size_t fdSmallSize = 0;
	ERRNO_CHECK(
	filedescriptor_t fdSmall = shm_open("/partitionsSmall", O_RDWR | O_CREAT, ACCESS_PERMISSION);
	, "shm_open", true);

    measure(&start);
	// the initial block size should be the size of a huge page
	// this goes down in logarithmic steps to the size of a small page
	size_t currentBlockSize = HUGE_PAGE_SIZE;
	while(size * sizeof(entry_t) < currentBlockSize * numPartitions && currentBlockSize > SMALL_PAGE_SIZE) {
		currentBlockSize /= 2;
	}

    // allocate and initialize the first block of each partition
    mappedPartitionBlockAdaptive_t** partitionStarts = malloc(sizeof(*partitionStarts) * numPartitions);
	for(size_t i = 0; i < numPartitions; ++i) {
		partitionStarts[i] = malloc(sizeof(*(partitionStarts[i])));
		partitionStarts[i]->offset = i * currentBlockSize;
		partitionStarts[i]->next = NULL;
		partitionStarts[i]->fillState = 0;
		partitionStarts[i]->sizeInByte = currentBlockSize;
	}

	if(currentBlockSize == HUGE_PAGE_SIZE) {
		fdHugeSize = currentBlockSize * numPartitions;
	}
	else {
		fdSmallSize = currentBlockSize * numPartitions;
		ftruncate(fdSmall, fdSmallSize);
	}


	// initialize the pointers that will always point to the partitionBlock to be filled now
	mappedPartitionBlockAdaptive_t* currentPartitionBlocks[numPartitions];
	memcpy(currentPartitionBlocks, partitionStarts, sizeof(*currentPartitionBlocks) * numPartitions);

	// create fill mappings
	// for each possible block size, we will have a dense mapping to minimize number of necessary virtual pages and thus pressure on the TLB
	// it holds currentBlock[i] stores the mapping for a block size of 2^i bytes (slot 0 and 1 are unused, as 2^2=4KB is the smallest size)
	const size_t numBlockSizes = 12;
	entry_t* currentBlocks[numBlockSizes];
	memset(currentBlocks, 0, sizeof(*currentBlocks) * numBlockSizes);
	// initialize the mapping for the current block size
    ERRNO_CHECK(
    currentBlocks[(size_t) log2(currentBlockSize / 1024)] = (entry_t*) mmap(NULL,
    																 currentBlockSize * numPartitions,
																	 PROT_READ | PROT_WRITE,
																	 MAP_SHARED | MAP_NORESERVE,
																	 currentBlockSize == HUGE_PAGE_SIZE ? fdHuge : fdSmall, 0);
    VALID_PTR(currentBlocks[(size_t) log2(currentBlockSize / 1024)], -1, "mmap currentPages", false);
    , "mmap currentPages", true);

    // create also anonymous mapping for smaller block sizes to have them available when needed
    for(size_t bs = currentBlockSize / 2; bs >= SMALL_PAGE_SIZE; bs /= 2) {
        ERRNO_CHECK(
        currentBlocks[(size_t) log2(bs / 1024)] = (entry_t*) mmap(NULL,
        									bs * numPartitions,
    										PROT_READ | PROT_WRITE,
											MAP_SHARED | MAP_ANONYMOUS,
    										-1, 0);
        VALID_PTR(currentBlocks[(size_t) log2(currentBlockSize / 1024)], -1, "mmap currentPages", false);
        , "mmap currentPages", true);
    }

	// initialize buffers and set offsets
    __attribute__((aligned(64))) bufferAdaptive_t buffers[numPartitions];
    assert(sizeof(bufferAdaptive_t) == CACHELINE_SIZE);

    for(size_t i = 0; i < numPartitions; ++i) {
    	buffers[i].slot = 0;
    	buffers[i].blockSize = (uint16_t) log2(currentBlockSize / 1024);	// store the block size to which we will flush next
    	buffers[i].target = 0;   // store the offset into the block that is currently written (see currentBlocks)
    }
    measure(&end);
    printTimeDifference(&start, &end, MALLOC_DST, measurement);

    // partition
    measure(&start);

    row_t* readStream = src;
    uint16_t blockSizeBackup;
    uint32_t targetBackup;
    size_t bucketNum;
    size_t powerOfTwo = 1;
    for(size_t i = 0; i < size; ++i) {
    	bucketNum = GET_BUCKET(readStream->cols[0], shift);
    	entry_t* bufferStart = (entry_t*) (buffers + bucketNum);
    	if(buffers[bucketNum].slot == TUPLES_PER_CACHELINE - 1) {
    		blockSizeBackup = buffers[bucketNum].blockSize;
    		targetBackup = buffers[bucketNum].target;
    		bufferStart[TUPLES_PER_CACHELINE - 1] = readStream->cols[0];

    		store_nontemp_64B(currentBlocks[blockSizeBackup] + (bucketNum * ((powerOfTwo << blockSizeBackup) * 1024 / sizeof(entry_t))) + targetBackup, bufferStart);
			targetBackup += TUPLES_PER_CACHELINE;

			// check whether the block has just been filled
			if(targetBackup == (currentPartitionBlocks[bucketNum]->sizeInByte / sizeof(entry_t))) {
				// it has been filled, so allocate a new block
				mappedPartitionBlockAdaptive_t* newPartitionBlock = malloc(sizeof(*newPartitionBlock));
				// check whether the block size should be halved based on the number of elements left to be processed
				while((size - i) * sizeof(entry_t) < currentBlockSize * numPartitions && currentBlockSize > SMALL_PAGE_SIZE) {
					currentBlockSize /= 2;
					printf("Changed block size to %d\n", blockSizeBackup);
				}

				// update the information in the buffer accordingly
				blockSizeBackup = (uint16_t) log2(currentBlockSize / 1024);

				if(currentBlockSize == HUGE_PAGE_SIZE) {
					// new huge page
					newPartitionBlock->offset = fdHugeSize;
					fdHugeSize += currentBlockSize;
				}
				else {
					// new small page(s)
					newPartitionBlock->offset = fdSmallSize;
					fdSmallSize += currentBlockSize;
					ftruncate(fdSmall, fdSmallSize);
				}

				newPartitionBlock->next = NULL;
				newPartitionBlock->fillState = 0;
				newPartitionBlock->sizeInByte = currentBlockSize;

				// link the new block into the list
				currentPartitionBlocks[bucketNum]->next = newPartitionBlock;
				currentPartitionBlocks[bucketNum]->fillState = currentPartitionBlocks[bucketNum]->sizeInByte / sizeof(entry_t);

				// rewire the fill mapping of the corresponding block size to point to the new block
				currentPartitionBlocks[bucketNum] = newPartitionBlock;
		    	ERRNO_CHECK(
		    	mmap(currentBlocks[blockSizeBackup] + (bucketNum * ((powerOfTwo << blockSizeBackup) * 1024 / sizeof(entry_t))),
		    		 currentBlockSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
					 currentBlockSize == HUGE_PAGE_SIZE ? fdHuge : fdSmall, newPartitionBlock->offset);
		    	VALID_PTR(currentBlocks[blockSizeBackup] + (bucketNum * ((powerOfTwo << blockSizeBackup) * 1024 / sizeof(entry_t))), -1, "mmap mapping to next page", false);
		    	, "mmap mapping to next page", true);

		    	// reset target
				targetBackup = 0;
			}

			// restore buffer working variables
			buffers[bucketNum].blockSize = blockSizeBackup;
			buffers[bucketNum].slot = 0;
			buffers[bucketNum].target = targetBackup;
    	}
    	else {
    		bufferStart[buffers[bucketNum].slot] = readStream->cols[0];
    		++buffers[bucketNum].slot;
    	}
    	++readStream;
    }

    // flush entries remaining in the buffers
    for(size_t i = 0; i < numPartitions; ++i) {
		for(uint32_t b = 0; b < buffers[i].slot; ++b) {
			(currentBlocks[buffers[i].blockSize] + (i * ((powerOfTwo << buffers[i].blockSize) * 1024 / sizeof(entry_t))))[buffers[i].target] = buffers[i].tuples[b];
			++buffers[i].target;
		}
		currentPartitionBlocks[i]->fillState = buffers[i].target;
    }

    measure(&end);
    printTimeDifference(&start, &end, MEMCPY, measurement);

    // sanity check
	//#ifdef ERRORCHECK
		entry_t total = 0;
		for(size_t i = 0; i < numPartitions; ++i) {
			mappedPartitionBlockAdaptive_t* block = partitionStarts[i];
			do {
				ERRNO_CHECK(
				entry_t* fp = mmap(NULL,
						  	  	   block->sizeInByte,
								   PROT_READ | PROT_WRITE,
								   MAP_SHARED | MAP_NORESERVE,
								   block->sizeInByte == HUGE_PAGE_SIZE ? fdHuge : fdSmall,
								   block->offset);
				VALID_PTR(fp, -1, "mmap debug", false);
				, "mmap debug", true);
				for(size_t j = 0; j < block->fillState; ++j) {
					total += fp[j];
				}
				if(fp) {
					ERRNO_CHECK(
					munmap(fp, block->sizeInByte);
					fp = NULL;
					, "munmap debug", true);
				}
			} while((block = block->next));
		}
		printf("total =%ld\n", total);
	//#endif


//    // map the blocks into a consecutive virtual memory area
//	// first, get a mappping into anonymous memory with enough space
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
//    	const size_t entriesToMove = MIN(s[partitionToFill].entriesMissingInPage, s[partitionToCollectFrom].entriesInTotal);
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
//	print_time_difference(&start, &end, SQUEEZE_AREA_COPY, measurement);
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
//	print_time_difference(&start, &end, SQUEEZE_AREA_MMAP, measurement);
//
//	// cleanup currentPages mapping
//	if(currentBlocks) {
//		ERRNO_CHECK(
//		munmap(currentBlocks, blockSize * numPartitions);
//		currentBlocks = NULL;
//	    , "munmap currentPages", true);
//	}

	entry_t* dst = NULL;
    setWdPartitioned(newWorkingData, src, dst, fdSmall, NULL, numPartitions, 0);
}

void mappedPartitioningBlockedAdaptiveCleanup(wd_pt* const workingData) {
//	notNull(workingData, "workingData");
//
//	// expecting wdPartitioned_t
//	wdPartitioned_t* wd = (wdPartitioned_t*) getWdPartitionedPtr(workingData);
//
//	const size_t pageSize = hugePages ? HUGE_PAGE_SIZE : SMALL_PAGE_SIZE;
//	assert(pagesPerBlock >= 1);
//	const size_t blockSize = pageSize * pagesPerBlock;
//
//    if(SAFE_FREE(wd->src)) {
//        printf("Freeing %s\n", getFieldName(MALLOC_SRC));
//    }
//
//    if(wd->dst) {
//		ERRNO_CHECK(
//		munmap(wd->dst, blockSize * wd->blockCount);
//		wd->dst = NULL;
//	    , "munmap dst", true);
//        printf("Freeing %s\n", getFieldName(MMAP_DST));
//
//        if(hugePages) {
//			ERRNO_CHECK(
//			close(wd->fd);
//			wd->fd = -1;
//			, "close", true);
//			ERRNO_CHECK(
//			unlink("/mnt/hugetlbfs/partitions");
//			, "unlinck /mnt/hugetlbfs/partitions", true);
//			printf("Freeing %s\n", getFieldName(SHM_DST_HUGE));
//        }
//        else {
//        	ERRNO_CHECK(
//            shm_unlink("/partitions");
//            , "shm_unlick /partitions", true);
//            printf("Freeing %s\n", getFieldName(SHM_DST));
//        }
//    }
//    SAFE_FREE(wd->histogram);
}
