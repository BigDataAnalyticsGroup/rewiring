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
 * main.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include <stdio.h>
#include <stdlib.h>
#include "main.h"

void method(wd_pt* workingData, const copyMethod_t copyMethod, const parameters_t* parameters, const size_t size, measurement_t* const measurement) {
	// Methods
	printf("%s:\n", getMethodName(copyMethod));
	switch(copyMethod) {
		case TR_MEMCPY: {
			traditionalMemcpy(workingData, size, measurement);
		} break;
		case COW_MEMCPY: {
			cowMemcpy(workingData, size, false, measurement);
		} break;
		case COW_MEMCPY_POPULATE: {
			cowMemcpy(workingData, size, true, measurement);
		} break;
		case COW_MEMCPY_HUGE: {
			cowMemcpyHuge(workingData, size, false, measurement);
		} break;
		case COW_MEMCPY_HUGE_POPULATE: {
			cowMemcpyHuge(workingData, size, true, measurement);
		} break;
		case COW_MEMCPY_SHUFFLE: {
			cowMemcpyShuffle(workingData, size, parameters->pagesPerChunk, false, false, false, measurement);
		} break;
		case COW_MEMCPY_SHUFFLE_POPULATE: {
			cowMemcpyShuffle(workingData, size, parameters->pagesPerChunk, true, false, false, measurement);
		} break;
		case COW_MEMCPY_SHUFFLE_HUGE: {
			cowMemcpyShuffle(workingData, size, parameters->pagesPerChunk, false, true, false, measurement);
		} break;
		case COW_MEMCPY_SHUFFLE_HUGE_POPULATE: {
			cowMemcpyShuffle(workingData, size, parameters->pagesPerChunk, true, true, false, measurement);
		} break;
		case COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES: {
			cowMemcpyShuffle(workingData, size, parameters->pagesPerChunk, false, false, true, measurement);
		} break;
		case COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES_HUGE: {
			cowMemcpyShuffle(workingData, size, parameters->pagesPerChunk, false, true, true, measurement);
			//printf("COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES_HUGE not working (apparently not possible).\n");
			//exit(1);
		} break;
		case TR_MEMCPY_SHUFFLE: {
			traditionalMemcpyShuffle(workingData, size, parameters->pagesPerChunk, false, parameters->initializeFile, measurement);
		} break;
		case TR_MEMCPY_SHUFFLE_HUGE: {
			traditionalMemcpyShuffle(workingData, size, parameters->pagesPerChunk, true, parameters->initializeFile, measurement);
		} break;
		case TR_PARTITIONING_WITH_HISTO: {
			traditionalPartitioningWithHistogram(workingData, size, parameters->numPartitions, parameters->populate, measurement);
		} break;
		case MAPPED_PARTITIONING: {
			mappedPartitioning(workingData, size, parameters->numPartitions, false, measurement);
		} break;
		case MAPPED_PARTITIONING_HUGE: {
			mappedPartitioning(workingData, size, parameters->numPartitions, true, measurement);
		} break;
		case TR_PARTITIONING_BLOCK_CHAINS: {
			traditionalPartitioningBlocked(workingData, size, parameters->numPartitions, measurement);
		} break;
		case MAPPED_PARTITIONING_BLOCK_CHAINS: {
			mappedPartitioningBlocked(workingData, size, parameters->numPartitions, parameters->pagesPerBlock, false, measurement);
		} break;
		case MAPPED_PARTITIONING_BLOCK_CHAINS_HUGE: {
			mappedPartitioningBlocked(workingData, size, parameters->numPartitions, parameters->pagesPerBlock, true, measurement);
		} break;
		case COW_HUGE_SMALL_MIX: {
			cowHugeSmallMix(workingData, size, measurement);
		} break;
		case MAPPED_PARTITIONING_BLOCK_CHAINS_ADAPTIVE: {
			mappedPartitioningBlockedAdaptive(workingData, size, parameters->numPartitions, measurement);
		} break;
		case REWIRED_VECTOR: {
			rewiredVector(workingData, SMALL_PAGE_SIZE / sizeof(entry_t), size, false, true, false, measurement);
		} break;
		case REWIRED_VECTOR_HUGE: {
			rewiredVector(workingData, HUGE_PAGE_SIZE / sizeof(entry_t), size, true, true, false, measurement);
		} break;
		case REWIRED_VECTOR_PREPOPULATE: {
			rewiredVector(workingData, SMALL_PAGE_SIZE / sizeof(entry_t), size, false, true, true, measurement);
		} break;
		case REWIRED_VECTOR_HUGE_PREPOPULATE: {
			rewiredVector(workingData, HUGE_PAGE_SIZE / sizeof(entry_t), size, true, true, true, measurement);
		} break;
		case STL_VECTOR: {
			stlVector(workingData, HUGE_PAGE_SIZE / sizeof(entry_t), measurement);
		} break;
		case MAP_AND_POPULATE: {
			mapAndPopulate(workingData,
							size,
							parameters->hugePages,
							parameters->file,
							parameters->shared,
							parameters->populate,
							parameters->populateByOption,
							parameters->initializeFile,
							parameters->randomShuffle,
							measurement);
		} break;
		case MREMAPED_VECTOR: {
			mremapedVector(workingData, SMALL_PAGE_SIZE / sizeof(entry_t), false, true, measurement);
		} break;
		case MREMAPED_VECTOR_HUGE: {
			mremapedVector(workingData, HUGE_PAGE_SIZE / sizeof(entry_t), true, true, measurement);
		} break;
		case ALLOCATION_REWIRING: {
			allocationRewiring(workingData, size, parameters->hugePages, parameters->randomMapping, true, measurement);
		} break;
		case ALLOCATION_SOFTWARE_INDIRECTION: {
			allocationSoftwareIndirection(workingData, size, parameters->hugePages, parameters->randomMapping, measurement);
		} break;
		case ALLOCATION_MMAP: {
			allocationMmap(workingData, size, parameters->hugePages, measurement);
		} break;
		case SOFTWARE_IND_VECTOR_HUGE: {
			softwareIndVector(workingData, HUGE_PAGE_SIZE / sizeof(entry_t), size, true, true, measurement);
		} break;
		case MREMAPED_PARTITIONING: {
			mremapedPartitioningBlocked(workingData, size, parameters->numPartitions, measurement);
		} break;
		default: {
			printf("Error: Unknown copy method.\n");
			exit(1);
		}
    }
}

void tests(wd_pt* workingData, const testType_t testType, const parameters_t* parameters, const size_t size, measurement_t* const measurement, const bool detailed) {
	notNull(workingData, "workingData");
	printf("Starting tests:\n");

	// Tests
	const wdKey_t wdKey = getWdKey(workingData);
	switch(testType) {
		case SEQ_READ_SRC_TEST: {					// read sequentially the entire src array
			switch(wdKey) {
				case WD_STANDARD: {
													const entry_t* src = getWdStandardPtr(workingData)->src;
													sequentialReadTest(src, size, parameters->selectivity, READ_SEQ_SRC, measurement);
				} break;
				case WD_PARTITIONED: {
													const row_t* src = getWdPartitionedPtr(workingData)->src;
													sequentialReadTableTest(src, size, parameters->selectivity, READ_SEQ_SRC, measurement);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case SEQ_READ_DST_TEST: {					// read sequentially the entire dst array
			const entry_t* dst = NULL;
			switch(wdKey) {
				case WD_STANDARD: {
													dst = getWdStandardPtr(workingData)->dst;
													sequentialReadTest(dst, size, parameters->selectivity, READ_SEQ_DST, measurement);
				} break;
				case WD_PARTITIONED: {
													dst = getWdPartitionedPtr(workingData)->dst;
													sequentialReadTest(dst, size, parameters->selectivity, READ_SEQ_DST, measurement);
				} break;
				case WD_MIXED: {
													dst = getWdMixedPtr(workingData)->dst;
													sequentialReadTest(dst, size, parameters->selectivity, READ_SEQ_DST, measurement);
				} break;
				case WD_SOFTWARE_INDIRECTION: {
													entry_t** dir = getWdSoftwareIndirectionPtr(workingData)->dir;
													if(parameters->hugePages) {
														sequentialReadSoftwareIndirectionTestHugePages(dir, size, parameters->selectivity, READ_SEQ_DST, measurement);
													}
													else {
														sequentialReadSoftwareIndirectionTestSmallPages(dir, size, parameters->selectivity, READ_SEQ_DST, measurement);
													}
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case RAND_READ_SRC_TEST: {					// read x% randomly of the src aray
			switch(wdKey) {
				case WD_STANDARD: {
													const entry_t* src = getWdStandardPtr(workingData)->src;
													randomReadTest(src, size, parameters->selectivity, READ_RAND_SRC, measurement);
				} break;
				case WD_PARTITIONED: {
													const row_t* src = getWdPartitionedPtr(workingData)->src;
													randomReadTableTest(src, size, parameters->selectivity, READ_RAND_SRC, measurement);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case RAND_READ_DST_TEST: {					// read x% randomly of the dst array
			const entry_t* dst = NULL;
			switch(wdKey) {
				case WD_STANDARD: 					dst = getWdStandardPtr(workingData)->dst;
													randomReadTest(dst, size, parameters->selectivity, READ_RAND_DST, measurement); break;
				case WD_PARTITIONED:				dst = getWdPartitionedPtr(workingData)->dst;
													randomReadTest(dst, size, parameters->selectivity, READ_RAND_DST, measurement); break;
				case WD_CHUNKED_PARTITIONED: {
													size_t totalSize = 0;
													wdChunkedPartitioned_t* wd = getWdChunkedPartitionedPtr(workingData);
													dst = mergeChunks(wd->chunks, wd->sizes, wd->numChunks, &totalSize, MERGE, measurement);
													randomReadTest(dst, totalSize, parameters->selectivity, READ_RAND_DST, measurement);
													SAFE_FREE(dst);
													break;
				} break;
				case WD_SOFTWARE_INDIRECTION: {
													entry_t** dir = getWdSoftwareIndirectionPtr(workingData)->dir;
													if(parameters->hugePages) {
														randomReadSoftwareIndirectionTestHugePages(dir, size, parameters->selectivity, READ_RAND_DST, measurement);
													}
													else {
														randomReadSoftwareIndirectionTestSmallPages(dir, size, parameters->selectivity, READ_RAND_DST, measurement);
													}
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case RANDOM_READ_DST_DEPENDENT_TEST: {		// read x% randomly of the dst array
			entry_t* dst = NULL;
			switch(wdKey) {
				case WD_STANDARD: {					dst = getWdStandardPtr(workingData)->dst;
													randomReadTestDependent(dst, size, parameters->selectivity, READ_RAND_DST, measurement);
				} break;
				case WD_SOFTWARE_INDIRECTION: {
													entry_t** dir = getWdSoftwareIndirectionPtr(workingData)->dir;
													if(parameters->hugePages) {
														randomReadSoftwareIndirectionTestDependentHugePages(dir, size, parameters->selectivity, READ_RAND_DST, measurement);
													}
													else {
														randomReadSoftwareIndirectionTestDependentSmallPages(dir, size, parameters->selectivity, READ_RAND_DST, measurement);
													}
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case RANDOM_READ_DST_DEPENDENT_MIXED_TEST: {	// read x% randomly of the dst array (mixed)
			entry_t* dst = NULL;
			switch(wdKey) {
				case WD_STANDARD: {					dst = getWdStandardPtr(workingData)->dst;
													randomReadTestDependentMixed(dst,
																					size,
																					parameters->selectivity,
																					parameters->numRandomAccesses,
																					READ_RAND_DST,
																					measurement);
				} break;
				case WD_SOFTWARE_INDIRECTION: {
													entry_t** dir = getWdSoftwareIndirectionPtr(workingData)->dir;
													if(parameters->hugePages) {
														randomReadSoftwareIndirectionTestDependentMixedHugePages(dir,
																													size,
																													parameters->selectivity,
																													parameters->numRandomAccesses,
																													READ_RAND_DST,
																													measurement);
													}
													else {
														randomReadSoftwareIndirectionTestDependentMixedSmallPages(dir,
																													size,
																													parameters->selectivity,
																													parameters->numRandomAccesses,
																													READ_RAND_DST,
																													measurement);
													}
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case SEQ_WRITE_SRC_TEST: {					// write x% sequentially of the src array
			switch(wdKey) {
				case WD_STANDARD: {
													entry_t* src = getWdStandardPtr(workingData)->src;
													sequentialWriteTest(src, size, parameters->selectivity, WRITE_SEQ_SRC, measurement);
				} break;
				case WD_PARTITIONED: {
													row_t* src = getWdPartitionedPtr(workingData)->src;
													sequentialWriteTableTest(src, size, parameters->selectivity, WRITE_SEQ_SRC, measurement);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case SEQ_WRITE_DST_TEST: {					// write x% sequentially of the dst array
			entry_t* dst = NULL;
			switch(wdKey) {
				case WD_STANDARD:					dst = getWdStandardPtr(workingData)->dst; break;
				case WD_PARTITIONED:				dst = getWdPartitionedPtr(workingData)->dst; break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
			sequentialWriteTest(dst, size, parameters->selectivity, WRITE_SEQ_DST, measurement);
		} break;
		case SEQ_READ_DST_DEPENDENT_TEST: {		// read x% sequentially of the dst array
			entry_t* dst = NULL;
			switch(wdKey) {
				case WD_STANDARD: {					dst = getWdStandardPtr(workingData)->dst;
													sequentialReadTestDependent(dst, size, parameters->selectivity, READ_SEQ_DST, measurement);
				} break;
				case WD_SOFTWARE_INDIRECTION: {
													entry_t** dir = getWdSoftwareIndirectionPtr(workingData)->dir;
													if(parameters->hugePages) {
														sequentialReadSoftwareIndirectionTestDependentHugePages(dir, size, parameters->selectivity, READ_SEQ_DST, measurement);
													}
													else {
														printf("Not implemented.\n"); exit(1);
													}
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case RAND_WRITE_SRC_TEST: {					// write x% randomly of the src array
			switch(wdKey) {
				case WD_STANDARD: {
													entry_t* src = getWdStandardPtr(workingData)->src;
													randomWriteTest(src, size, parameters->selectivity, WRITE_RAND_SRC, measurement);
				} break;
				case WD_PARTITIONED: {
													row_t* src = getWdPartitionedPtr(workingData)->src;
													randomWriteTableTest(src, size, parameters->selectivity, WRITE_RAND_SRC, measurement);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case RAND_WRITE_DST_TEST: {					// write x% randomly of the dst array
			entry_t* dst = NULL;
			switch(wdKey) {
				case WD_STANDARD:					dst = getWdStandardPtr(workingData)->dst; break;
				case WD_PARTITIONED:				dst = getWdPartitionedPtr(workingData)->dst; break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
			randomWriteTest(dst, size, parameters->selectivity, WRITE_RAND_DST, measurement);
		} break;
		case SEQ_READ_DST_TWICE_TEST: {				// read sequentially the entire dst array (two times)
			const entry_t* dst = NULL;
			switch(wdKey) {
				case WD_STANDARD:					dst = getWdStandardPtr(workingData)->dst; break;
				case WD_PARTITIONED:				dst = getWdPartitionedPtr(workingData)->dst; break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
			sequentialReadTest(dst, size, 100.0, READ_SEQ_DST, measurement);
			sequentialReadTest(dst, size, 100.0, READ2_SEQ_DST, measurement);
		} break;
		case SEQ_WRITE_DST_TWICE_TEST: {				// read sequentially the entire dst array (two times)
			switch(wdKey) {
				case WD_STANDARD: {
													entry_t* dst = getWdStandardPtr(workingData)->dst;
													sequentialWriteTest(dst, size, 100.0, WRITE_SEQ_DST, measurement);
													sequentialWriteTest(dst, size, 100.0, WRITE2_SEQ_DST, measurement);
				} break;
				case WD_SOFTWARE_INDIRECTION: {
													entry_t** dir = getWdSoftwareIndirectionPtr(workingData)->dir;
													sequentialWriteSoftwareIndirectionTest(dir, size, parameters->hugePages, 100.0, WRITE_SEQ_DST, measurement);
													sequentialWriteSoftwareIndirectionTest(dir, size, parameters->hugePages, 100.0, WRITE2_SEQ_DST, measurement);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case SEQ_READ_DST_FROM_SEVERAL_CHUNKS: {	// read sequentially from several chunks
			entry_t** chunks = NULL;
			const size_t* sizes = NULL;
			size_t numChunks = 0;
			switch(wdKey) {
				case WD_CHUNKED: {
													chunks = getWdChunkedPtr(workingData)->chunks;
													sizes = getWdChunkedPtr(workingData)->sizes;
													numChunks = getWdChunkedPtr(workingData)->numChunks;
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
			sequentialReadTestFromSeveralChunks(chunks, sizes, numChunks, READ_SEQ_DST, measurement);
		} break;

		case SEQ_READ_DST_MERGE_FROM_CHUNKS: {		// merge several chunks and read the consecutive result sequentially
			entry_t* dst = NULL;
			size_t totalSize = 0;
			switch(wdKey) {
				case WD_CHUNKED: {
													const wdChunked_t* const wd = getWdChunkedPtr(workingData);
													dst = mergeChunks(wd->chunks, wd->sizes, wd->numChunks, &totalSize, MERGE, measurement);
				} break;
				case WD_CHUNKED_PARTITIONED: {
													const wdChunkedPartitioned_t* const wd = getWdChunkedPartitionedPtr(workingData);
													dst = mergeChunks(wd->chunks, wd->sizes, wd->numChunks, &totalSize, MERGE, measurement);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
			sequentialReadTest(dst, totalSize, 100.0, READ_SEQ_DST, measurement);
			SAFE_FREE(dst);
		} break;
		case RADIX_SORT_DST_TEST: {					// perform radix sort on the entire dst array
			entry_t* dst = NULL;
			switch(wdKey) {
				case WD_STANDARD:					dst = getWdStandardPtr(workingData)->dst; break;
				case WD_PARTITIONED:				dst = getWdPartitionedPtr(workingData)->dst; break;
				case WD_CHUNKED_PARTITIONED: {
													const wdChunkedPartitioned_t* const wd = getWdChunkedPartitionedPtr(workingData);
													dst = mergeChunks(wd->chunks, wd->sizes, wd->numChunks, NULL, MERGE, measurement);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
			radixSortTest(dst, size, RADIX_SORT, measurement);
			SAFE_FREE(dst);
		} break;
		case RADIX_SORT_PER_PARTITION_CONSECUTIVE:	{
			entry_t** partitions = NULL;
			size_t* histogram = NULL;
			size_t numPartitions = 0;
			switch(wdKey) {
				case WD_PARTITIONED: {
													wdPartitioned_t* wd = getWdPartitionedPtr(workingData);
													numPartitions = wd->numPartitions;
													histogram = wd->histogram;
													partitions = malloc(sizeof(*partitions) * numPartitions);
													size_t offset = 0;
													for(size_t p = 0; p < wd->numPartitions; ++p) {
														partitions[p] = wd->dst + offset;
														offset += wd->histogram[p];
													}
													radixSortTestFromSeveralChunks(partitions, histogram, numPartitions, RADIX_SORT, measurement);
													SAFE_FREE(partitions);
				} break;
//				case WD_CHUNKED_PARTITIONED: {
//													wdChunkedPartitioned_t* wd = getWdChunkedPartitionedPtr(workingData);
//													entry_t* dst = mergeChunks(wd->chunks, wd->sizes, wd->numChunks, NULL, MERGE, measurement);
//													histogram = wd->histogram;
//													numPartitions = wd->numPartitions;
//													partitions = malloc(sizeof(*partitions) * numPartitions);
//													size_t offset = 0;
//													for(size_t p = 0; p < wd->numPartitions; ++p) {
//														partitions[p] = dst + offset;
//														offset += histogram[p];
//													}
//													radixSortTestFromSeveralChunks(partitions, histogram, numPartitions, RADIX_SORT, measurement);
//													SAFE_FREE(partitions);
//				} break;
				case WD_CHUNKED_PARTITIONED: {
													wdChunkedPartitioned_t* wd = getWdChunkedPartitionedPtr(workingData);
													radixSortTestFromSeveralChunks(wd->chunks, wd->sizes, wd->numPartitions, RADIX_SORT, measurement);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case RADIX_SORT_PER_PARTITION_INDIRECTION: 	{
			switch(wdKey) {
//				case WD_CHUNKED_PARTITIONED: {
//													wdChunkedPartitioned_t* wd = getWdChunkedPartitionedPtr(workingData);
//													entry_t** indirection = createIndirectionForChunks(wd->chunks, wd->sizes, wd->numChunks, NULL, INDIRECTION, measurement);
//													histogram = wd->histogram;
//													numPartitions = wd->numPartitions;
//													partitions = malloc(sizeof(*partitions) * numPartitions);
//													size_t offset = 0;
//													for(size_t p = 0; p < wd->numPartitions; ++p) {
//														partitions[p] = indirection + offset;
//														offset += histogram[p];
//													}
//													radixSortTestFromSeveralChunksIndirection(partitions, histogram, numPartitions, RADIX_SORT, measurement);
//													SAFE_FREE(partitions);
//				} break;
				case WD_CHUNKED_PARTITIONED: {
													wdChunkedPartitioned_t* wd = getWdChunkedPartitionedPtr(workingData);
													softwareIndRadixSortTestFromSeveralChunks(wd->chunks, wd->numChunks, wd->sizes, wd->numPartitions, wd->histogram, RADIX_SORT, measurement);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case RAND_READ_DST_INDIRECTION_TEST: {
			switch(wdKey) {
				case WD_CHUNKED_PARTITIONED: {
													wdChunkedPartitioned_t* wd = getWdChunkedPartitionedPtr(workingData);
													entry_t** indirection = createIndirectionForChunks(wd->chunks, wd->sizes, wd->numChunks, NULL, INDIRECTION, measurement);

													randomReadIndirectionTest(indirection, size, parameters->selectivity, READ_RAND_DST, measurement);
													SAFE_FREE(indirection);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case VECTOR_INSERT_TEST: {
			switch(wdKey) {
				case WD_REWIRED_VECTOR: {
													wdRewiredVector_t* wd = getWdRewiredVectorPtr(workingData);
													if(wd->rv->fd == -1) {
														// anonymous version
														mremapedVectorPushbackTest(wd->rv, size, measurement, detailed);
													}
													else {
														// file backed version
														rewiredVectorPushbackTest(wd->rv, size, measurement, detailed);
													}
				} break;
				case WD_STL_VECTOR: {
													wdStlVector_t* wd = getWdStlVectorPtr(workingData);
													stlVectorPushbackTest(wd->stlv, size, measurement, detailed);
				} break;
				case WD_SOFTWARE_INDIRECTION_VECTOR: {
													wdSoftwareIndVector_t* wd = getWdSoftwareIndVectorPtr(workingData);
													softwareIndVectorPushbackTest(wd->siv, size, measurement, detailed);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		case PAGE_FAULT_TEST: {						// write first byte of each page of the src array
			switch(wdKey) {
				case WD_STANDARD: {
													entry_t* src = getWdStandardPtr(workingData)->src;
													pageFaultTest(src, size, parameters->hugePages, WRITE_SEQ_SRC, measurement);
				} break;
				default:							printf("Error: wdKey = %d\n", wdKey); exit(1);
			}
		} break;
		default: {
			printf("Error: Unknown test type.\n");
			exit(1);
		}
	}
}

void cleanup(wd_pt* workingData, const copyMethod_t copyMethod, const parameters_t* parameters, const size_t size) {
	notNull(workingData, "workingData");
	printf("Cleanup:\n");

	// Cleanup
	switch(copyMethod) {
		case TR_MEMCPY:
		case TR_MEMCPY_SHUFFLE:
		case TR_MEMCPY_SHUFFLE_HUGE: {
			 traditionalMemcpyCleanup(workingData);
		} break;
		case TR_PARTITIONING_WITH_HISTO: {
			 traditionalPartitionedCleanup(workingData);
		} break;
		case TR_PARTITIONING_BLOCK_CHAINS: {
			traditionalPartitioningBlockedCleanup(workingData, parameters->numPartitions);
		} break;
		case COW_MEMCPY:
		case COW_MEMCPY_POPULATE:
		case COW_MEMCPY_SHUFFLE:
		case COW_MEMCPY_SHUFFLE_POPULATE:
		case COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES:
		case COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES_HUGE: {
			cowMemcpyCleanup(workingData, size);
		} break;
		case COW_MEMCPY_HUGE:
		case COW_MEMCPY_HUGE_POPULATE:
		case COW_MEMCPY_SHUFFLE_HUGE:
		case COW_MEMCPY_SHUFFLE_HUGE_POPULATE: {
			cowMemcpyHugeCleanup(workingData, size);
		} break;
		case MAPPED_PARTITIONING: {
			mappedPartitioningCleanup(workingData, size, parameters->numPartitions, false);
		} break;
		case MAPPED_PARTITIONING_HUGE: {
			mappedPartitioningCleanup(workingData, size, parameters->numPartitions, true);
		} break;
		case MAPPED_PARTITIONING_BLOCK_CHAINS: {
			mappedPartitioningBlockedCleanup(workingData, parameters->pagesPerBlock, false);
		} break;
		case MAPPED_PARTITIONING_BLOCK_CHAINS_HUGE: {
			mappedPartitioningBlockedCleanup(workingData, parameters->pagesPerBlock, true);
		} break;
		case COW_HUGE_SMALL_MIX: {
			cowHugeSmallMixCleanup(workingData, size);
		} break;
		case MAPPED_PARTITIONING_BLOCK_CHAINS_ADAPTIVE: {
			mappedPartitioningBlockedAdaptiveCleanup(workingData);
		} break;
		case REWIRED_VECTOR:
		case REWIRED_VECTOR_HUGE:
		case REWIRED_VECTOR_PREPOPULATE:
		case REWIRED_VECTOR_HUGE_PREPOPULATE: {
			freeRewiredVector(workingData);
		} break;
		case MREMAPED_VECTOR:
		case MREMAPED_VECTOR_HUGE: {
			freeMremapedVector(workingData);
		} break;
		case STL_VECTOR: {
			freeStlVector(workingData);
		} break;
		case MAP_AND_POPULATE: {
			mapAndPopulateCleanup(workingData, size, parameters->hugePages, parameters->file);
		} break;
		case ALLOCATION_REWIRING: {
			allocationRewiringCleanup(workingData, size, parameters->hugePages);
		} break;
		case ALLOCATION_SOFTWARE_INDIRECTION: {
			allocationSoftwareIndirectionCleanup(workingData);
		} break;
		case ALLOCATION_MMAP: {
			allocationMmapCleanup(workingData, size, parameters->hugePages);
		} break;
		case SOFTWARE_IND_VECTOR_HUGE: {
			freeSoftwareIndVector(workingData);
		} break;
		case MREMAPED_PARTITIONING: {
			mremapedPartitioningBlockedCleanup(workingData, parameters->numPartitions);
		} break;
		default: {
	    	printf("Error: Unknown copy method.\n");
	    	exit(1);
		}
	}
}

void run(const copyMethod_t copyMethod, const testType_t testType, const parameters_t* parameters, const size_t size, measurement_t* const measurement, const bool detailed) {
	wd_pt workingData = NULL;

	method(&workingData, copyMethod, parameters, size, measurement);
    tests(&workingData, testType, parameters, size, measurement, detailed);
    cleanup(&workingData, copyMethod, parameters, size);
}

int main(int argc, const char **argv) {

	if (argc < 6) {
	    printf("Usage: ./a.out <method> <test_type>[(test_parameter)] <number_of_entries> <number_of_repetitions> <detailed (true|false)> [<output_file>]\n");

	    printf(	"Methods\n"
	    		"1. Memcpy\n"
	    		"2. Cow memcpy\n"
	    		"3. Cow memcpy using populate\n"
	    		"4. Cow memcpy using huge pages\n"
	    		"5. Cow memcpy using huge pages and populate\n"
	    		"6. Cow memcpy with shuffling\n"
	    		"7. Cow memcpy with shuffling using populate\n"
	    		"8. Cow memcpy with shuffling using huge pages\n"
	    		"9. Cow memcpy with shuffling using huge pages and populate\n"
	    		"10. Memcpy with shuffling\n"
	    		"11. Memcpy with shuffling using huge pages\n"
	    		"12. Traditional partitioning with histogram\n"
	    		"13. Mapped partitioning without histogram\n"
	    		"14. Mapped partitioning without histogram using huge pages\n"
	    		"15. Cow memcpy with shuffling using remap_file_pages\n"
	    		"16. Cow memcpy with shuffling using remap_file_pages and huge pages\n"
	    		"17. Traditional partitioning with block-chains\n"
	    		"18. Mapped partitioning with block-chains\n"
	    		"19. Mapped partitioning with block-chains using huge pages\n"
				"20. Mix huge and small pages\n"
	    		"21. Mapped partitioning with block-chains using adaptive block sizes\n"
	    		"22. Rewired vector\n"
	    		"23. Rewired vector using huge pages\n"
	    		"24. STL vector\n"
	    		"30. Rewired vector using prepopulation\n"
	    		"31. Rewired vector using huge pages and prepopulation\n"
	    		"32. Map and Populate\n"
	    		"36. Mremaped vector\n"
	    		"37. Mremaped vector using huge pages\n"
	    		"40. Allocation Rewiring (Pool)\n"
	    		"41. Allocation Software Indirection (Pool)\n"
	    		"42. Allocation Mmap\n"
	    		"43. Software Indirection Vector (hugepages)\n"
	    		"44. Mremaped partitioning using huge pages\n");

	    printf(	"Test Types\n"
	    		"1. Read sequential x%% src (x is test_parameter)\n"
	    		"2. Read sequential x%% dst (x is test_parameter)\n"
	    		"3. Read x%% random,uniform src (x is test_parameter).\n"
	    		"4. Read x%% random,uniform dst (x is test_parameter).\n"
	    		"5. Write sequential x%% src (x is test_parameter).\n"
        		"6. Write sequential x%% dst (x is test_parameter).\n"
        		"7. Write x%% random,uniform src (x is test_parameter).\n"
        		"8. Write x%% random,uniform dst (x is test_parameter).\n"
        		"9. Read sequential all dst (two times)\n"
	    		"10. Read sequential all dst from several chunks\n"
	    		"11. Read sequential all dst and merge several chunks\n"
	    		"12. Radix-sort all dst\n"
	    		"13. Radix-sort all dst from several partitions (from consecutive memory region)\n"
	    		"14. Radix-sort all dst from several partitions (using an indirection)\n"
	    		"15. Vector insertion test\n"
	    		"18. Page fault test\n"
	    		"19. Read x%% random,uniform dst (x is test_parameter) through an indirection.\n"
        		"20. Write sequential all dst (two times)\n"
	    		"21. Read x%% random,uniform dst (x is test_parameter) with dependent access.\n"
	    		"22. Read x%% random,uniform dst (x is test_parameter) with dependent access (mixed).\n"
        		"23. Read sequential x%% dst (x is test_parameter) with dependent access.\n");
	    exit(1);
	}

	parameters_t parameters;

	// copy method
	copyMethod_t copyMethod;
	parseArgument(argv[1], &copyMethod, &parameters, true);

	// test type and parameter
	testType_t testType;
	parseArgument(argv[2], &testType, &parameters, false);

	// run variables
	const size_t size = atoi(argv[3]);
	const size_t runs = atoi(argv[4]);
	const bool detailed = strcmp(argv[5], "true") == 0;

	// handling optional output file argument (if no argument is provided, it prints to stdout)
	FILE* outputFile = NULL;
	if(argc == 7) {
		outputFile = fopen(argv[6], "w");
		if(!outputFile) {
			printf("Error: Can not open file handle.\n");
			exit(1);
		}
	}

	if (copyMethod > numCopyMethod || copyMethod < 1 || testType < 1 || testType > numTestType) {
	    printf("Invalid choices. Method (1 - %d). Test Type (1 - %d).\n", numCopyMethod, numTestType);
	    exit(1);
	}

	experiment_t experiment = getFreshExperiment(getMethodName(copyMethod), runs);

	printf("Number of elements = %ld\n", size);
	printf("Number of runs = %ld\n", runs);
	for (size_t r = 0; r < runs; r++) {
		printf("\n=============== Run %ld ===============\n", r + 1);
		measurement_t measurement = getFreshMeasurement();
		run(copyMethod, testType, &parameters, size, &measurement, detailed);
		experiment.measurements[r] = measurement;
	}
	printf("\n");

	if(detailed) {
		printDetailedExperimentalData(&experiment, outputFile);
	}
	else {
		printExperimentalData(&experiment, outputFile, X_FIELDS_Y_RUNS);
	}

	freeExperimentalData(&experiment);

	printf("\n");
	return 0;
}





