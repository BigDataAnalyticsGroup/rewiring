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
 * stl_vector_c.h
 *
 *  Created on: Mar 10, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef STL_VECTOR_C_H_
#define STL_VECTOR_C_H_

#include "../types.h"
#include "../timefile.h"

// declarations for the counterparts in cpp files
stlVec_pt stlvGetNewVector(const size_t initCapacity);
void stlvPushBack(stlVec_pt const v, const entry_t* const entry);
void stlvPushBackArray(stlVec_pt const v, const entry_t* const entries, const size_t size, double** diffs);
void stlvInsertPage(stlVec_pt const v, const entry_t* const entries, const size_t size, const size_t insertPosition);
entry_t* stlvGetMem(const stlVec_pt v, size_t* numEntries);
void stlvReserve(stlVec_pt const v, const size_t size);
void stlvClear(stlVec_pt const v);
void stlvFreeVector(stlVec_pt const v);

void stlVector(wd_pt* const workingData, const size_t size, measurement_t* const measurement);
void freeStlVector(wd_pt* const workingData);

#endif /* STL_VECTOR_C_H_ */
