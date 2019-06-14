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
 * stl_vector.h
 *
 *  Created on: Mar 10, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef STL_VECTOR_H_
#define STL_VECTOR_H_

#include <iostream>
#include <vector>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "../macros.h"

typedef uint64_t entry_t;
typedef struct timespec timespec_t;
typedef void* stlVec_pt;

extern "C" stlVec_pt stlvGetNewVector(const size_t initCapacity);

extern "C" void stlvPushBack(stlVec_pt const v, const entry_t* const entry);

extern "C" void stlvPushBackArray(stlVec_pt const v,
	      	  	  	  	  	  	  const entry_t* const entries,
								  const size_t size,
								  double** diffs);

extern "C" void stlvInsertPage(stlVec_pt const v,
							   const entry_t* const entries,
							   const size_t size,
							   const size_t insertPosition);

extern "C" entry_t* stlvGetMem(const stlVec_pt v, size_t* numEntries);

extern "C" void stlvReserve(stlVec_pt const v, const size_t size);
extern "C" void stlvClear(stlVec_pt const v);

extern "C" void stlvFreeVector(stlVec_pt const v);

#ifdef __APPLE__
	#define CLOCK_REALTIME 0
	void clock_gettime;
#endif

#endif /* STL_VECTOR_H_ */
