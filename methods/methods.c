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
 * methods.c
 *
 *  Created on: Mar 06, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "methods.h"

const char* getMethodName(const copyMethod_t copy_method) {
	switch(copy_method) {
		case TR_MEMCPY: return "Traditional memcpy";
		case COW_MEMCPY: return "COW memcpy";
		case COW_MEMCPY_POPULATE: return "COW memcpy (populated)";
		case COW_MEMCPY_HUGE: return "COW memcpy (huge_pages)";
		case COW_MEMCPY_HUGE_POPULATE: return "COW memcpy (huge_pages, populated)";
		case COW_MEMCPY_SHUFFLE: return "COW memcpy shuffled";
		case COW_MEMCPY_SHUFFLE_POPULATE: return "COW memcpy shuffled (populated)";
		case COW_MEMCPY_SHUFFLE_HUGE: return "COW memcpy shuffled (huge_pages)";
		case COW_MEMCPY_SHUFFLE_HUGE_POPULATE: return "COW memcpy shuffled (huge_pages, populated)";
		case COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES: return "COW memcpy shuffled (remap file pages)";
		case COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES_HUGE: return "COW memcpy shuffled (remap file pages, hugepages)";
		case TR_MEMCPY_SHUFFLE: return "Traditional memcpy shuffled";
		case TR_MEMCPY_SHUFFLE_HUGE: return "Traditional memcpy shuffled (huge_pages)";
		case TR_PARTITIONING_WITH_HISTO: return "Traditional partitioning with histogram";
		case MAPPED_PARTITIONING: return "Mapped partitioning without histogram";
		case MAPPED_PARTITIONING_HUGE: return "Mapped partitioning without histogram (huge_pages)";
		case TR_PARTITIONING_BLOCK_CHAINS: return "Traditional partitioning with block-chains";
		case MAPPED_PARTITIONING_BLOCK_CHAINS: return "Mapped partitioning with block-chains";
		case MAPPED_PARTITIONING_BLOCK_CHAINS_HUGE: return "Mapped partitioning with block-chains (hugepages)";
		case COW_HUGE_SMALL_MIX: return "Mix huge and small pages";
		case MAPPED_PARTITIONING_BLOCK_CHAINS_ADAPTIVE: return "Mapped partitioning with block-chains (adaptive block sizes)";
		case REWIRED_VECTOR: return "Rewired vector";
		case REWIRED_VECTOR_HUGE: return "Rewired vector (hugepages)";
		case REWIRED_VECTOR_PREPOPULATE: return "Rewired vector (prepopulate)";
		case REWIRED_VECTOR_HUGE_PREPOPULATE: return "Rewired vector (hugepages, prepopulate)";
		case STL_VECTOR: return "STL Vector";
		case MAP_AND_POPULATE: return "Map and Populate";
		case MREMAPED_VECTOR: return "Mremaped vector";
		case MREMAPED_VECTOR_HUGE: return "Mremaped vector (huge pages)";
		case ALLOCATION_REWIRING: return "Allocation Rewiring (Pool)";
		case ALLOCATION_SOFTWARE_INDIRECTION: return "Allocation Software Indirection (Pool)";
		case ALLOCATION_MMAP: return "Allocation Mmap";
		case SOFTWARE_IND_VECTOR_HUGE: return "Software Indirection Vector (hugepages)";
		case MREMAPED_PARTITIONING: return "Mremaped partitioning (hugepages)";
		default: printf("%s\n", "Error: method unknown."); exit(1);
	}
}
