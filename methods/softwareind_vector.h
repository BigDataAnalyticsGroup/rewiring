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
 * softwareind_vector.h
 *
 *  Created on: Dec 2, 2015
 *      Author: Felix Martin Schuhknecht
 */

#ifndef SOFTWAREIND_VECTOR_H_
#define SOFTWAREIND_VECTOR_H_

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "../useful_functions.h"
#include "../types.h"
#include "../timediff.h"

softwareIndVector_t* softwareIndGetNewVector(const size_t initCapacity,		// the number of entries for which initially space is reserved
									const size_t poolCapacity,
									const bool hugePages,					// use huge pages (2MB) or small pages(4KB)
									const bool doubleWhenFull,				// when set to true, the size of the space is doubled when full
																			// when set to false, the space is increased page by page
									measurement_t* const measurement);


// inlined
static inline void softwareIndPushBack(softwareIndVector_t* const siv, const entry_t* const entry) {
	if(siv->numEntries == siv->capacity) {
		// we ran out of space
		// depending on the strategy, double the space or allocate only a single page

		// we double the capacity
		const size_t oldCapacityInPages = siv->capacity / siv->entriesPerPage;
		const size_t newCapacityInPages = oldCapacityInPages + (siv->doubleWhenFull ? oldCapacityInPages : 1);

		entry_t** newDir = NULL;
	    posix_memalign((void**) &newDir, CACHELINE_SIZE, newCapacityInPages * sizeof(*newDir));
	    memcpy(newDir, siv->dir, oldCapacityInPages * sizeof(*newDir));

	    // assumes sequential mapping into the pool
	    newDir[oldCapacityInPages] = siv->dir[oldCapacityInPages - 1] + siv->entriesPerPage;
	    for(size_t i = oldCapacityInPages + 1; i < newCapacityInPages; ++i) {
	    	newDir[i] = newDir[i-1] + siv->entriesPerPage;
	    }
	    siv->writePtr = newDir[oldCapacityInPages];

	    SAFE_FREE(siv->dir);
	    siv->dir = newDir;

	    // update capacity
	    siv->capacity += siv->doubleWhenFull ? siv->capacity : siv->entriesPerPage;
	}

	// standard case, simply insert
	siv->writePtr[siv->currentPageSlot++] = *entry;
	if(siv->currentPageSlot == siv->entriesPerPage) {
		++siv->currentDirSlot;
		siv->writePtr = (siv->capacity / siv->entriesPerPage) > siv->currentDirSlot ? siv->dir[siv->currentDirSlot] : NULL;
		siv->currentPageSlot = 0;
	}

	++siv->numEntries;
}

entry_t** softwareIndGetMem(const softwareIndVector_t* const siv, size_t* numEntries, size_t* pageSize);

void softwareIndFreeVector(softwareIndVector_t* const siv);



void softwareIndVector(wd_pt* const workingData,
					const size_t initCapacity,
					const size_t poolCapacity,
					const bool hugePages,
					const bool doubleWhenFull,
					measurement_t* const measurement);

void freeSoftwareIndVector(wd_pt* const workingData);

#endif /* SOFTWAREIND_VECTOR_H_ */
