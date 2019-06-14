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
 * stl_vector_c.c
 *
 *  Created on: Mar 10, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "stl_vector_c.h"

void stlVector(wd_pt* const workingData, const size_t size, measurement_t* const measurement) {
	notNull(workingData, "workingData");

	// creating wdStlVector_t
	wdStlVector_t* newWorkingData = getWdStlVector();
	*workingData = newWorkingData;

	stlVec_pt stlv = stlvGetNewVector(size);

	setWdStlVector(newWorkingData, stlv);
}

void freeStlVector(wd_pt* const workingData) {
	notNull(workingData, "workingData");
	// expecting wdStlVector_t
	wdStlVector_t* wd = (wdStlVector_t*) getWdStlVectorPtr(workingData);

	stlvFreeVector(wd->stlv);
}
