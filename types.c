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
 * types.c
 *
 *  Created on: Feb 26, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "types.h"

wdStandard_t* getWdStandard() {
	wdStandard_t* newWorkingData = (wdStandard_t*) malloc(sizeof(wdStandard_t));
	newWorkingData->wdKey = WD_STANDARD;
	return newWorkingData;
}

void setWdStandard(wdStandard_t* const workingData,
					 entry_t* const src,
					 entry_t* const dst,
					const filedescriptor_t fd) {
	notNull(workingData, "workingData");
	workingData->src = src;
	workingData->dst = dst;
	workingData->fd = fd;
}

wdStandard_t* getWdStandardPtr(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	wdStandard_t* wd = (wdStandard_t*) *workingData;
	assert(wd->wdKey == WD_STANDARD);
	return wd;
}


wdSoftwareIndirection_t* getWdSoftwareIndirection() {
	wdSoftwareIndirection_t* newWorkingData = (wdSoftwareIndirection_t*) malloc(sizeof(wdSoftwareIndirection_t));
	newWorkingData->wdKey = WD_SOFTWARE_INDIRECTION;
	return newWorkingData;
}

void setWdSoftwareIndirection(wdSoftwareIndirection_t* const workingData,
					 entry_t** dir,
					 entry_t* pool) {
	notNull(workingData, "workingData");
	workingData->dir = dir;
	workingData->pool = pool;
}

wdSoftwareIndirection_t* getWdSoftwareIndirectionPtr(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	wdSoftwareIndirection_t* wd = (wdSoftwareIndirection_t*) *workingData;
	assert(wd->wdKey == WD_SOFTWARE_INDIRECTION);
	return wd;
}



wdPartitioned_t* getWdPartitioned() {
	wdPartitioned_t* newWorkingData = (wdPartitioned_t*) malloc(sizeof(wdPartitioned_t));
	newWorkingData->wdKey = WD_PARTITIONED;
	return newWorkingData;
}

void setWdPartitioned(wdPartitioned_t* const workingData,
						row_t* const src,
						entry_t* const dst,
						const filedescriptor_t fd,
						size_t* const histogram,
						const size_t numPartitions,
						const size_t blockCount) {
	notNull(workingData, "workingData");
	workingData->src = src;
	workingData->dst = dst;
	workingData->fd = fd;
	workingData->histogram = histogram;
	workingData->numPartitions = numPartitions;
	workingData->blockCount = blockCount;
}

wdPartitioned_t* getWdPartitionedPtr(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	wdPartitioned_t* wd = (wdPartitioned_t*) *workingData;
	assert(wd->wdKey == WD_PARTITIONED);
	return wd;
}


wdChunked_t* getWdChunked() {
	wdChunked_t* newWorkingData = (wdChunked_t*) malloc(sizeof(wdChunked_t));
	newWorkingData->wdKey = WD_CHUNKED;
	return newWorkingData;
}

void setWdChunked(wdChunked_t* const workingData,
					entry_t* const src,
					entry_t** chunks,
					size_t* const sizes,
					const size_t numChunks,
					partitionBlock_t** partitionStarts) {
	notNull(workingData, "workingData");
	workingData->src = src;
	workingData->chunks = chunks;
	workingData->sizes = sizes;
	workingData->numChunks = numChunks;
	workingData->partitionStarts = partitionStarts;
}

wdChunked_t* getWdChunkedPtr(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	wdChunked_t* wd = (wdChunked_t*) *workingData;
	assert(wd->wdKey == WD_CHUNKED);
	return wd;
}


wdChunkedPartitioned_t* getWdChunkedPartitioned() {
	wdChunkedPartitioned_t* newWorkingData = (wdChunkedPartitioned_t*) malloc(sizeof(wdChunkedPartitioned_t));
	newWorkingData->wdKey = WD_CHUNKED_PARTITIONED;
	return newWorkingData;
}

void setWdChunkedPartitioned(wdChunkedPartitioned_t* const workingData,
								row_t* const src,
								entry_t** chunks,
								size_t* const sizes,
								const size_t numChunks,
								void** partitionStarts,
								size_t* const histogram,
								const size_t numPartitions) {
	notNull(workingData, "workingData");
	workingData->src = src;
	workingData->chunks = chunks;
	workingData->sizes = sizes;
	workingData->numChunks = numChunks;
	workingData->partitionStarts = partitionStarts;
	workingData->histogram = histogram;
	workingData->numPartitions = numPartitions;
}

wdChunkedPartitioned_t* getWdChunkedPartitionedPtr(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	wdChunkedPartitioned_t* wd = (wdChunkedPartitioned_t*) *workingData;
	assert(wd->wdKey == WD_CHUNKED_PARTITIONED);
	return wd;
}


wdMixed_t* getWdMixed() {
	wdMixed_t* newWorkingData = (wdMixed_t*) malloc(sizeof(wdMixed_t));
	newWorkingData->wdKey = WD_MIXED;
	return newWorkingData;
}

void setWdMixed(wdMixed_t* const workingData,
					entry_t* const srcSmall,
					entry_t* const srcHuge,
					entry_t* const dst,
					const filedescriptor_t fdSmall,
					const filedescriptor_t fdHuge) {
	notNull(workingData, "workingData");
	workingData->srcSmall = srcSmall;
	workingData->srcHuge = srcHuge;
	workingData->dst = dst;
	workingData->fdSmall = fdSmall;
	workingData->fdHuge = fdHuge;
}

wdMixed_t* getWdMixedPtr(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	wdMixed_t* wd = (wdMixed_t*) *workingData;
	assert(wd->wdKey == WD_MIXED);
	return wd;
}


wdRewiredVector_t* getWdRewiredVector() {
	wdRewiredVector_t* newWorkingData = (wdRewiredVector_t*) malloc(sizeof(wdRewiredVector_t));
	newWorkingData->wdKey = WD_REWIRED_VECTOR;
	return newWorkingData;
}

void setWdRewiredVector(wdRewiredVector_t* const workingData,
					rewiredVector_t* rv) {
	notNull(workingData, "workingData");
	workingData->rv = rv;
}

wdRewiredVector_t* getWdRewiredVectorPtr(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	wdRewiredVector_t* wd = (wdRewiredVector_t*) *workingData;
	assert(wd->wdKey == WD_REWIRED_VECTOR);
	return wd;
}


wdStlVector_t* getWdStlVector() {
	wdStlVector_t* newWorkingData = (wdStlVector_t*) malloc(sizeof(wdStlVector_t));
	newWorkingData->wdKey = WD_STL_VECTOR;
	return newWorkingData;
}

void setWdStlVector(wdStlVector_t* const workingData,
					stlVec_pt stlv) {
	notNull(workingData, "workingData");
	workingData->stlv = stlv;
}

wdStlVector_t* getWdStlVectorPtr(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	wdStlVector_t* wd = (wdStlVector_t*) *workingData;
	assert(wd->wdKey == WD_STL_VECTOR);
	return wd;
}

wdSoftwareIndVector_t* getWdSoftwareIndVector() {
	wdSoftwareIndVector_t* newWorkingData = (wdSoftwareIndVector_t*) malloc(sizeof(wdSoftwareIndVector_t));
	newWorkingData->wdKey = WD_SOFTWARE_INDIRECTION_VECTOR;
	return newWorkingData;
}

void setWdSoftwareIndVector(wdSoftwareIndVector_t* const workingData,
					softwareIndVector_t* siv) {
	notNull(workingData, "workingData");
	workingData->siv = siv;
}

wdSoftwareIndVector_t* getWdSoftwareIndVectorPtr(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	wdSoftwareIndVector_t* wd = (wdSoftwareIndVector_t*) *workingData;
	assert(wd->wdKey == WD_SOFTWARE_INDIRECTION_VECTOR);
	return wd;
}


wdKey_t getWdKey(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	wdKey_t wdKey = *((wdKey_t*) *workingData);
	return wdKey;
}

