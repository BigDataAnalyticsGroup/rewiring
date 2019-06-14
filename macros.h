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
 * macros.h
 *
 *  Created on: Feb 03, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef MACROS_H_
#define MACROS_H_

// pass via gcc -D
//#define ERRORCHECK

#define CACHELINE_SIZE 64

#define SMALL_PAGE_SIZE 4096
#define ENTRIES_PER_SMALL_PAGE (SMALL_PAGE_SIZE / sizeof(entry_t))
#define ENTRIES_PER_SMALL_PAGE_LOG2 9

#define HUGE_PAGE_SIZE 2097152
#define ENTRIES_PER_HUGE_PAGE (HUGE_PAGE_SIZE / sizeof(entry_t))
#define ENTRIES_PER_HUGE_PAGE_LOG2 18
#define HUGE_PAGE_TLB_SLOTS 32

#define INDEX_ENTRIES_PER_HUGE_PAGE (HUGE_PAGE_SIZE / sizeof(entry_pair_t))

#define MAX_KEY 18446744073709551615u

#define TUPLES_PER_CACHELINE (CACHELINE_SIZE / sizeof(entry_t))
#define INDEX_ENTRIES_PER_CACHELINE (CACHELINE_SIZE / sizeof(entry_pair_t))

// specifies the number of columns in row_t
#ifndef NUM_COLUMNS
	#define NUM_COLUMNS 1
#endif

#define SEED 42
#define MAX_NUMBER_OF_PARAMETERS 10
#define DETAILED_GRANULARITY 10000	// measure a block of 10000 elements
#define ACCESS_PERMISSION 0666

#define X_FIELDS_Y_RUNS true

#endif /* MACROS_H_ */
