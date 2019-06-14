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
 * radix_sort_test.c
 *
 *  Created on: Feb 26, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "radix_sort_test.h"

// insertion sort
void insertionSort(entry_t* const c, const size_t n) {
	for (size_t k = 1; k < n; ++k) {
		const entry_t key = c[k];
		long i = k-1;
		while ((i >= 0) && (key < c[i])) {
			c[i+1] = c[i];
			--i;
		}
		c[i+1] = key;
	}
}

void insertionSortIndirect(entry_t** const c, const size_t n) {
	for (size_t k = 1; k < n; ++k) {
		const entry_t key = *(c[k]);
		long i = k-1;
		while ((i >= 0) && (key < *(c[i]))) {
			*(c[i+1]) = *(c[i]);
			--i;
		}
		*(c[i+1]) = key;
	}
}

// hybrid radix sort: switches to insertion sort after a threshold
void doHybridRadixsortInsert(entry_t* const c, const size_t n, size_t shift) {
	const size_t numBuckets = 256;
	const entry_t mask = 0xFF;

	size_t last[numBuckets];
	size_t ptr[numBuckets];
	size_t cnt[numBuckets];

	size_t i,j,k,sorted,remain;
	entry_t temp, swap;

	memset(cnt, 0, numBuckets * sizeof(size_t)); // Zero counters

	switch(shift) {
		case 0: 	for(i=0;i<n;++i) ++cnt[c[i] & mask]; break;
		case 8: 	for(i=0;i<n;++i) ++cnt[(c[i] >> 8) & mask]; break;
		case 16: 	for(i=0;i<n;++i) ++cnt[(c[i] >> 16) & mask]; break;
		case 24: 	for(i=0;i<n;++i) ++cnt[(c[i] >> 24) & mask]; break;
		case 32: 	for(i=0;i<n;++i) ++cnt[(c[i] >> 32) & mask]; break;
		case 40: 	for(i=0;i<n;++i) ++cnt[(c[i] >> 40) & mask]; break;
		case 48: 	for(i=0;i<n;++i) ++cnt[(c[i] >> 48) & mask]; break;
		case 56: 	for(i=0;i<n;++i) ++cnt[c[i] >> 56]; break;
	}

	sorted = (cnt[0]==n);	// Accumulate counters into pointers
	ptr[0] = 0;
	last[0] = cnt[0];
	for(i=1; i<numBuckets; ++i) {
		last[i] = (ptr[i]=last[i-1]) + cnt[i];
		sorted |= (cnt[i]==n);
	}
	if(!sorted) {	// Go through all swaps
		i = numBuckets-1;
		remain = n;
		while(remain > 0){
			while(ptr[i] == last[i])
				--i;	// Find uncompleted value range
			j = ptr[i];	// Grab first element in cycle
			swap = c[j];
			k = (swap >> shift) & mask;
			if(i != k){	// Swap into correct range until cycle completed
				do{
					temp = c[ptr[k]];
					c[ptr[k]] = swap;
					++ptr[k];
					k = ((swap=temp) >> shift) & mask;
					--remain;
				}while(i != k);
				c[j] = swap;	// Place last element in cycle
			}
			++ptr[k];
			--remain;
		}
	}

	if(shift > 0) {	// Sort on next digit
		shift -= 8;
		for(i=0; i<numBuckets; ++i){
			if (cnt[i] > INSERT_SORT_LEVEL) {
				doHybridRadixsortInsert(&c[last[i]-cnt[i]], cnt[i], shift);
			}
			else if(cnt[i] > 1) {
				insertionSort(&c[last[i]-cnt[i]], cnt[i]);
			}
		}
 	}
}

void doHybridRadixsortInsertIndirect(entry_t** const c, const size_t n, size_t shift) {
	const size_t numBuckets = 256;
	const entry_t mask = 0xFF;

	size_t last[numBuckets];
	size_t ptr[numBuckets];
	size_t cnt[numBuckets];

	size_t i,j,k,sorted,remain;
	entry_t temp, swap;

	memset(cnt, 0, numBuckets * sizeof(size_t)); // Zero counters

	switch(shift) {
		case 0: 	for(i=0;i<n;++i) ++cnt[*(c[i]) & mask]; break;
		case 8: 	for(i=0;i<n;++i) ++cnt[(*(c[i]) >> 8) & mask]; break;
		case 16: 	for(i=0;i<n;++i) ++cnt[(*(c[i]) >> 16) & mask]; break;
		case 24: 	for(i=0;i<n;++i) ++cnt[(*(c[i]) >> 24) & mask]; break;
		case 32: 	for(i=0;i<n;++i) ++cnt[(*(c[i]) >> 32) & mask]; break;
		case 40: 	for(i=0;i<n;++i) ++cnt[(*(c[i]) >> 40) & mask]; break;
		case 48: 	for(i=0;i<n;++i) ++cnt[(*(c[i]) >> 48) & mask]; break;
		case 56: 	for(i=0;i<n;++i) ++cnt[*(c[i]) >> 56]; break;
	}

	sorted = (cnt[0]==n);	// Accumulate counters into pointers
	ptr[0] = 0;
	last[0] = cnt[0];
	for(i=1; i<numBuckets; ++i) {
		last[i] = (ptr[i]=last[i-1]) + cnt[i];
		sorted |= (cnt[i]==n);
	}
	if(!sorted) {	// Go through all swaps
		i = numBuckets-1;
		remain = n;
		while(remain > 0){
			while(ptr[i] == last[i])
				--i;	// Find uncompleted value range
			j = ptr[i];	// Grab first element in cycle
			swap = *(c[j]);
			k = (swap >> shift) & mask;
			if(i != k){	// Swap into correct range until cycle completed
				do{
					temp = *(c[ptr[k]]);
					*(c[ptr[k]]) = swap;
					++ptr[k];
					k = ((swap=temp) >> shift) & mask;
					--remain;
				}while(i != k);
				*(c[j]) = swap;	// Place last element in cycle
			}
			++ptr[k];
			--remain;
		}
	}

	if(shift > 0) {	// Sort on next digit
		shift -= 8;
		for(i=0; i<numBuckets; ++i){
			if (cnt[i] > INSERT_SORT_LEVEL) {
				doHybridRadixsortInsertIndirect(&c[last[i]-cnt[i]], cnt[i], shift);
			}
			else if(cnt[i] > 1) {
				insertionSortIndirect(&c[last[i]-cnt[i]], cnt[i]);
			}
		}
 	}
}


void hybridRadixsortInsert(entry_t* const c, const size_t n) {
	doHybridRadixsortInsert(c, n, 56);
}

void hybridRadixsortInsertIndirect(entry_t** const c, const size_t n) {
	doHybridRadixsortInsertIndirect(c, n, 56);
}




void radixSortTest(entry_t* const p, const size_t size, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");

	timeval_t start, end;

    measure(&start);
    hybridRadixsortInsert(p, size);
    measure(&end);

	#ifdef ERRORCHECK
		entry_t total = 0;
		for(size_t i = 0; i < size; i++) {
			total += p[i];
		}
		isSorted(p, size);
		printTimeDifferenceChecksum(&start, &end, f, total, measurement);
	#else
		printTimeDifference(&start, &end, f, measurement);
	#endif
}


void radixSortTestFromSeveralChunks(entry_t** const p, const size_t* size, const size_t numChunks, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");
	notNull(size, "size");

	timeval_t start, end;

	measure(&start);
	for(size_t c = 0; c < numChunks; ++c) {
		hybridRadixsortInsert(p[c], size[c]);
	}
	measure(&end);

	#ifdef ERRORCHECK
		entry_t total = 0;
		for(size_t c = 0; c < numChunks; ++c) {
			for(size_t i = 0; i < size[c]; i++) {
				total += p[c][i];
			}
			isSorted(p[c], size[c]);
		}
		printTimeDifferenceChecksum(&start, &end, f, total, measurement);
	#else
		printTimeDifference(&start, &end, f, measurement);
	#endif
}

void radixSortTestFromSeveralChunksIndirection(entry_t*** const p, const size_t* size, const size_t numChunks, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");
	notNull(size, "size");

	timeval_t start, end;

	measure(&start);
	for(size_t c = 0; c < numChunks; ++c) {
		hybridRadixsortInsertIndirect(p[c], size[c]);
	}
	measure(&end);

	#ifdef ERRORCHECK
		entry_t total = 0;
		for(size_t c = 0; c < numChunks; ++c) {
			for(size_t i = 0; i < size[c]; i++) {
				total += *(p[c][i]);
			}
			isSortedIndirect(p[c], size[c]);
		}
		printTimeDifferenceChecksum(&start, &end, f, total, measurement);
	#else
		printTimeDifference(&start, &end, f, measurement);
	#endif
}

void softwareIndRadixSortTestFromSeveralChunks(entry_t** chunks,
												const size_t numChunks,
												const size_t* size,
												const size_t numPartitions,
												const size_t* histogram,
												const field_t f,
												measurement_t* const measurement) {
	timeval_t start, end;

	measure(&start);
	size_t seenChunks = 0;
	for(size_t i = 0; i < numPartitions; ++i) {
		const size_t numEntriesInPartition = histogram[i];
		const size_t numChunksInvolved = (numEntriesInPartition / ENTRIES_PER_HUGE_PAGE) + (numEntriesInPartition % ENTRIES_PER_HUGE_PAGE == 0 ? 0 : 1);
		entry_t* partition[numChunksInvolved];
		for(size_t j = 0; j < numChunksInvolved; ++j) {
			partition[j] = chunks[j+seenChunks];
		}
		seenChunks += numChunksInvolved;
		softwareIndDoHybridRadixsortInsert(partition, 0, ENTRIES_PER_HUGE_PAGE, ENTRIES_PER_HUGE_PAGE_LOG2, numEntriesInPartition, 56);
	}
	measure(&end);

	#ifdef ERRORCHECK
		entry_t total = 0;
		for(size_t c = 0; c < numChunks; ++c) {
			for(size_t i = 0; i < size[c]; i++) {
				total += chunks[c][i];
			}
			isSorted(chunks[c], size[c]);
		}
		printTimeDifferenceChecksum(&start, &end, f, total, measurement);
	#else
		printTimeDifference(&start, &end, f, measurement);
	#endif
}



entry_t** createIndirectionForChunks(entry_t** chunks, const size_t* const sizes, const size_t numChunks, size_t* const totalSize, const field_t f, measurement_t* const measurement) {
	notNull(chunks, "chunks");
	notNull(sizes, "sizes");
	notNull(measurement, "measurement");

	timeval_t start, end;
	measure(&start);

	size_t ts = 0;
	for(size_t i = 0; i < numChunks; ++i) {
		ts += sizes[i];
	}
	if(totalSize) *totalSize = ts;

	entry_t** indirection = malloc(ts * sizeof(*indirection));

	// link
	size_t i = 0;
	for(size_t c = 0; c < numChunks; ++c) {
		for(size_t e = 0; e < sizes[c]; ++e) {
			indirection[i] = &chunks[c][e];
			++i;
		}
	}

	measure(&end);
    printTimeDifference(&start, &end, f, measurement);

	return indirection;
}





void softwareIndInsertionSort(entry_t** const dir,
							const size_t offset,
							const size_t gran,
							const size_t granLog2,
							const size_t n) {
	for (size_t k = 1; k < n; ++k) {
		const entry_t key = dir[(offset + k) >> granLog2][(offset + k) & (gran - 1)];
		long i = k-1;
		while ((i >= 0) && (key < dir[(offset + i) >> granLog2][(offset + i) & (gran - 1)])) {
			dir[(offset + (i+1)) >> granLog2][(offset + (i+1)) & (gran - 1)] = dir[(offset + i) >> granLog2][(offset + i) & (gran - 1)];
			--i;
		}
		dir[(offset + (i+1)) >> granLog2][(offset + (i+1)) & (gran - 1)] = key;
	}
}

void softwareIndDoHybridRadixsortInsert(entry_t** const dir,
										const size_t offset,
										const size_t gran,
										const size_t granLog2,
										const size_t n,
										size_t shift) {
	const size_t numBuckets = 256;
	const entry_t mask = 0xFF;

	size_t last[numBuckets];
	size_t ptr[numBuckets];
	size_t cnt[numBuckets];

	size_t i,j,k,sorted,remain;
	entry_t temp, swap;

	memset(cnt, 0, numBuckets * sizeof(size_t)); // Zero counters

	switch(shift) {
		case 0: 	for(i=0;i<n;++i) ++cnt[dir[(offset + i) >> granLog2][(offset + i) & (gran - 1)] & mask]; break;
		case 8: 	for(i=0;i<n;++i) ++cnt[(dir[(offset + i) >> granLog2][(offset + i) & (gran - 1)] >> 8) & mask]; break;
		case 16: 	for(i=0;i<n;++i) ++cnt[(dir[(offset + i) >> granLog2][(offset + i) & (gran - 1)] >> 16) & mask]; break;
		case 24: 	for(i=0;i<n;++i) ++cnt[(dir[(offset + i) >> granLog2][(offset + i) & (gran - 1)] >> 24) & mask]; break;
		case 32: 	for(i=0;i<n;++i) ++cnt[(dir[(offset + i) >> granLog2][(offset + i) & (gran - 1)] >> 32) & mask]; break;
		case 40: 	for(i=0;i<n;++i) ++cnt[(dir[(offset + i) >> granLog2][(offset + i) & (gran - 1)] >> 40) & mask]; break;
		case 48: 	for(i=0;i<n;++i) ++cnt[(dir[(offset + i) >> granLog2][(offset + i) & (gran - 1)] >> 48) & mask]; break;
		case 56: 	for(i=0;i<n;++i) ++cnt[dir[(offset + i) >> granLog2][(offset + i) & (gran - 1)] >> 56]; break;
	}

	sorted = (cnt[0]==n);	// Accumulate counters into pointers
	ptr[0] = 0;
	last[0] = cnt[0];
	for(i=1; i<numBuckets; ++i) {
		last[i] = (ptr[i]=last[i-1]) + cnt[i];
		sorted |= (cnt[i]==n);
	}
	if(!sorted) {	// Go through all swaps
		i = numBuckets-1;
		remain = n;
		while(remain > 0){
			while(ptr[i] == last[i])
				--i;	// Find uncompleted value range
			j = ptr[i];	// Grab first element in cycle
			swap = dir[(offset + j) >> granLog2][(offset + j) & (gran - 1)];
			k = (swap >> shift) & mask;
			if(i != k){	// Swap into correct range until cycle completed
				do{
					temp = dir[(offset+ptr[k]) >> granLog2][(offset+ptr[k]) & (gran - 1)];
					dir[(offset+ptr[k]) >> granLog2][(offset+ptr[k]) & (gran - 1)] = swap;
					++ptr[k];
					k = ((swap=temp) >> shift) & mask;
					--remain;
				}while(i != k);
				dir[(offset+j) >> granLog2][(offset+j) & (gran - 1)] = swap;	// Place last element in cycle
			}
			++ptr[k];
			--remain;
		}
	}

	if(shift > 0) {	// Sort on next digit
		shift -= 8;
		for(i=0; i<numBuckets; ++i){
			if (cnt[i] > INSERT_SORT_LEVEL) {
				softwareIndDoHybridRadixsortInsert(dir, offset + (last[i]-cnt[i]), gran, granLog2, cnt[i], shift);
			}
			else if(cnt[i] > 1) {
				softwareIndInsertionSort(dir, offset + (last[i]-cnt[i]), gran, granLog2, cnt[i]);
			}
		}
 	}
}




