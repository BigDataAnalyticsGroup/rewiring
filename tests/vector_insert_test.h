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
 * vector_insert_test.h
 *
 *  Created on: Mar 13, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef VECTOR_INSERT_TEST_H_
#define VECTOR_INSERT_TEST_H_

#include "../fields.h"
#include "../types.h"
#include "../timediff.h"
#include "../error.h"
#include "../methods/rewired_vector.h"
#include "../methods/stl_vector_c.h"

void rewiredVectorInsertTest(rewiredVector_t* const rv, const size_t size, measurement_t* const measurement, const bool detailed);
void stlVectorInsertTest(stlVec_pt const rv, const size_t size, measurement_t* const measurement, const bool detailed, const bool ensureCapacity);

#endif /* VECTOR_INSERT_TEST_H_ */
