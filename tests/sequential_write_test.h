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
 * sequential_write_test.h
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef SEQUENTIAL_WRITE_TEST_H_
#define SEQUENTIAL_WRITE_TEST_H_

#include <math.h>
#include "../fields.h"
#include "../types.h"
#include "../timediff.h"
#include "../error.h"

typedef struct {
	entry_t** dir;
	size_t entriesPerPage;
	size_t currentDirSlot;
	size_t currentInPage;
} iteratorState;

static inline entry_t* next(iteratorState* state) {
	entry_t* res = state->dir[state->currentDirSlot] + state->currentInPage;
	++state->currentInPage;
	if(state->currentInPage >= state->entriesPerPage) {
		state->currentInPage = 0;
		++state->currentDirSlot;
	}
	return res;
}

void sequentialWriteTest(entry_t* const p, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement);
void sequentialWriteSoftwareIndirectionTest(entry_t** dir, const size_t size, const size_t hugePages, const double selectivity, const field_t f, measurement_t* const measurement);
void sequentialWriteTableTest(row_t* const p, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement);

#endif /* SEQUENTIAL_WRITE_TEST_H_ */
