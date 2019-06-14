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
 * types.h
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include "error.h"
#include "macros.h"
#include "bitset.h"

typedef uint64_t entry_t;
typedef struct {
	entry_t cols[NUM_COLUMNS];
} row_t;

typedef struct {
	uint32_t key;
	uint32_t value;
} index_t;

typedef int filedescriptor_t;

typedef struct timeval timeval_t;
typedef struct timespec timespec_t;

typedef void* linearHashTable_pt;

typedef int64_t offset_t;

typedef enum {
	RANDOM,
	SEQUENTIAL,
	SKEWED
} workload_t;

typedef struct {
	size_t pagesPerChunk;
	double selectivity;
	size_t numPartitions;
	size_t pagesPerBlock;
	size_t numInsertions;
	size_t numQueries;
	double probeLengthLimit;
	size_t numRandomAccesses;
	bool hugePages;
	bool file;
	bool shared;
	bool populate;
	bool populateByOption;
	bool initializeFile;
	bool randomMapping;
	bool randomShuffle;
	size_t numPartitionsOutOfTLB;
	size_t numPartitionsInTLB;
	workload_t workload;
} parameters_t;

typedef struct {
	size_t first;
	size_t second;
} pair_t;

typedef struct {
	entry_t first;
	entry_t second;
} entry_pair_t;

typedef struct {
	uint32_t inclusiveLower;
	uint32_t inclusiveUpper;
} range_query_t;

// requires sizeof(entry_t) == 2 * sizeof(uint32_t)
typedef struct {
    entry_t tuples[TUPLES_PER_CACHELINE - 1];
    uint32_t slot;
    uint32_t target;
} buffer_t;

typedef struct {
    entry_pair_t tuples[INDEX_ENTRIES_PER_CACHELINE - 1];
    uint64_t slot;
    uint64_t target;
} bufferIndex_t;

// requires sizeof(row_t) == 2 * sizeof(uint32_t)
typedef struct {
    row_t tuples[TUPLES_PER_CACHELINE - 1];
    uint32_t slot;
    uint32_t target;
} bufferRow_t;

typedef struct partitionBlock_t partitionBlock_t;
struct partitionBlock_t {
	entry_t* tuples;
	size_t fillState;				// this fillState is updated only when a page is completed and when the partitioning ends. the live fillState is encoded in the buffers
	partitionBlock_t* next;
};

typedef struct {
	entry_t* tuples;
	entry_t* startOfCurrentBlock;
	size_t fillState;
	size_t numPages;
} mremapedPartitionBlock_t;


typedef struct mappedPartitionBlock_t mappedPartitionBlock_t;
struct mappedPartitionBlock_t {
	off_t offset;					// the offset to the page that contains the data of this block for file fd
	size_t fillState;				// this fillState is updated only when a page is completed and when the partitioning ends. the live fillState is encoded in the buffers
	mappedPartitionBlock_t* next;
};

typedef struct mappedPartitionBlockAdaptive_t mappedPartitionBlockAdaptive_t;
struct mappedPartitionBlockAdaptive_t {
	off_t offset;					// the offset to the page that contains the data of this block for file fd
	size_t fillState;				// this fillState is updated only when a page is completed and when the partitioning ends. the live fillState is encoded in the buffers
	size_t sizeInByte;
	mappedPartitionBlockAdaptive_t* next;
};

typedef struct {
	size_t entriesMissingInPage;
	size_t entriesInTotal;
	entry_pair_t* startPtr;				// points to the first slot of the first page
	entry_pair_t* fillPtr;				// points to the first empty slot
} squeezeInfo_t;

typedef struct {
	entry_t* vmem;
	size_t numEntries;
	size_t capacity;
	filedescriptor_t fd;
	size_t pageSize;
	size_t entriesPerPage;
	bool doubleWhenFull;
	char* path;
	char* remapped;
} rewiredVector_t;

typedef struct {
	entry_t** dir;
	uint8_t* pool;
	entry_t* writePtr;
	size_t currentDirSlot;
	size_t currentPageSlot;
	size_t entriesPerPage;
	size_t numEntries;
	size_t capacity;
	size_t pageSize;
	bool doubleWhenFull;
} softwareIndVector_t;

// working data types
typedef void* wd_pt;
typedef void* stlVec_pt;
typedef void* stlList_pt;
typedef void* btree_pt;
typedef size_t list_t;

#define SLOTS_PER_PAGE 512
typedef uint64_t block_t;
#define BLOCK_SIZE_IN_BITS (sizeof(block_t) * 8)

typedef struct {
	uint32_t physicalPageIdentifier;
	uint32_t localDepth;
} bucketMetaData_t;


typedef enum {
	WD_STANDARD,
	WD_CHUNKED,
	WD_PARTITIONED,
	WD_CHUNKED_PARTITIONED,
	WD_MIXED,
	WD_REWIRED_VECTOR,
	WD_STL_VECTOR,
	WD_SOFTWARE_INDIRECTION,
	WD_SOFTWARE_INDIRECTION_VECTOR
} wdKey_t;

typedef struct {
	entry_t expectedResult;
	entry_t actualResult;
} result_check_t;

// every working data type must have the field wdKey to identify it
// this field must be the first element of the structure
typedef struct {
	wdKey_t wdKey;
	entry_t* src;
	entry_t* dst;
	filedescriptor_t fd;
	result_check_t resultCheck;
} wdStandard_t;

typedef struct {
	wdKey_t wdKey;
	entry_t** dir;
	entry_t* pool;
	result_check_t resultCheck;
} wdSoftwareIndirection_t;

typedef struct {
	wdKey_t wdKey;
	row_t* src;
	entry_t* dst;
	filedescriptor_t fd;
	size_t* histogram;		// stores the number of entries per bucket
	size_t numPartitions;
	size_t blockCount;		// stores the number of pages used (for unmapping)
	result_check_t resultCheck;
} wdPartitioned_t;

typedef struct {
	wdKey_t wdKey;
	entry_t* src;
	entry_t** chunks;
	size_t* sizes;
	size_t numChunks;
	partitionBlock_t** partitionStarts;
	result_check_t resultCheck;
} wdChunked_t;

typedef struct {
	wdKey_t wdKey;
	row_t* src;
	entry_t** chunks;
	size_t* sizes;
	size_t numChunks;
	void** partitionStarts;
	size_t* histogram;		// stores the number of entries per bucket
	size_t numPartitions;
	result_check_t resultCheck;
	entry_t* pool;
} wdChunkedPartitioned_t;

typedef struct {
	wdKey_t wdKey;
	entry_t* srcSmall;
	entry_t* srcHuge;
	entry_t* dst;
	filedescriptor_t fdSmall;
	filedescriptor_t fdHuge;
	result_check_t resultCheck;
} wdMixed_t;

typedef struct {
	wdKey_t wdKey;
	rewiredVector_t* rv;
} wdRewiredVector_t;

typedef struct {
	wdKey_t wdKey;
	softwareIndVector_t* siv;
} wdSoftwareIndVector_t;

typedef struct {
	wdKey_t wdKey;
	stlVec_pt stlv;
} wdStlVector_t;



wdStandard_t* getWdStandard();
void setWdStandard(wdStandard_t* const workingData,
					entry_t* const src,
					entry_t* const dst,
					const filedescriptor_t fd);
wdStandard_t* getWdStandardPtr(wd_pt* const workingData);

wdSoftwareIndirection_t* getWdSoftwareIndirection();
void setWdSoftwareIndirection(wdSoftwareIndirection_t* const workingData,
					entry_t** dir,
					entry_t* pool);
wdSoftwareIndirection_t* getWdSoftwareIndirectionPtr(wd_pt* const workingData);

wdPartitioned_t* getWdPartitioned();
void setWdPartitioned(wdPartitioned_t* const workingData,
						row_t* const src,
						entry_t* const dst,
						const filedescriptor_t fd,
						size_t* const histogram,
						const size_t numPartitions,
						const size_t pageCount);
wdPartitioned_t* getWdPartitionedPtr(wd_pt* const workingData);

wdChunked_t* getWdChunked();
void setWdChunked(wdChunked_t* const workingData,
					entry_t* const src,
					entry_t** chunks,
					size_t* const sizes,
					const size_t numChunks,
					partitionBlock_t** partitionStarts);
wdChunked_t* getWdChunkedPtr(wd_pt* const workingData);

wdChunkedPartitioned_t* getWdChunkedPartitioned();
void setWdChunkedPartitioned(wdChunkedPartitioned_t* const workingData,
					row_t* const src,
					entry_t** chunks,
					size_t* const sizes,
					const size_t numChunks,
					void** partitionStarts,
					size_t* const histogram,
					const size_t numPartitions);
wdChunkedPartitioned_t* getWdChunkedPartitionedPtr(wd_pt* const workingData);

wdMixed_t* getWdMixed();
void setWdMixed(wdMixed_t* const workingData,
					entry_t* const srcSmall,
					entry_t* const srcHuge,
					entry_t* const dst,
					const filedescriptor_t fdSmall,
					const filedescriptor_t fdHuge);
wdMixed_t* getWdMixedPtr(wd_pt* const workingData);

wdRewiredVector_t* getWdRewiredVector();
void setWdRewiredVector(wdRewiredVector_t* const workingData,
					rewiredVector_t* rv);
wdRewiredVector_t* getWdRewiredVectorPtr(wd_pt* const workingData);

wdStlVector_t* getWdStlVector();
void setWdStlVector(wdStlVector_t* const workingData,
					stlVec_pt rv);
wdStlVector_t* getWdStlVectorPtr(wd_pt* const workingData);


wdSoftwareIndVector_t* getWdSoftwareIndVector();
void setWdSoftwareIndVector(wdSoftwareIndVector_t* const workingData,
					softwareIndVector_t* rv);
wdSoftwareIndVector_t* getWdSoftwareIndVectorPtr(wd_pt* const workingData);

wdKey_t getWdKey(wd_pt* const workingData);



// Warning:
// Dummy macros to suppress warnings when developing under OSX.
// This does NOT make the program portable in any sense.
// All code is written for Linux only.
#ifdef __APPLE__
	#define MAP_POPULATE 0
	#define MAP_ANONYMOUS 0
	#define MAP_HUGETLB 0
	#define MAP_NONBLOCK 0
	#define MREMAP_MAYMOVE 0
	#define CLOCK_REALTIME 0
	#define MAP_HUGE_2MB 0
	#define MADV_HUGEPAGE 0
	#define MADV_NOHUGEPAGE 0
	typedef size_t gsize;
	#include <avxintrin.h>
#endif

#endif /* TYPES_H_ */
