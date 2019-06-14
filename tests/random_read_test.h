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
 * random_read_test.h
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef RANDOM_READ_TEST_H_
#define RANDOM_READ_TEST_H_

#include <math.h>
#include "../fields.h"
#include "../types.h"
#include "../timediff.h"
#include "../error.h"
#include "../useful_functions.h"

void randomReadTest(const entry_t* const p,
					const size_t size,
					const double selectivity,
					const field_t f,
					measurement_t* const measurement);

void randomReadTestDependent(entry_t* const p,
								const size_t size,
								const double selectivity,
								const field_t f,
								measurement_t* const measurement);

void randomReadTestDependentMixed(entry_t* const p,
									const size_t size,
									const double selectivity,
									const size_t numRandomAccesses,
									const field_t f,
									measurement_t* const measurement);

void randomReadSoftwareIndirectionTestHugePages(entry_t** dir,
												const size_t size,
												const double selectivity,
												const field_t f,
												measurement_t* const measurement);

void randomReadSoftwareIndirectionTestDependentHugePages(entry_t** dir,
															const size_t size,
															const double selectivity,
															const field_t f,
															measurement_t* const measurement);

void randomReadSoftwareIndirectionTestDependentMixedHugePages(entry_t** dir,
															const size_t size,
															const double selectivity,
															const size_t numRandomAccesses,
															const field_t f,
															measurement_t* const measurement);

void randomReadSoftwareIndirectionTestSmallPages(entry_t** dir,
													const size_t size,
													const double selectivity,
													const field_t f,
													measurement_t* const measurement);

void randomReadSoftwareIndirectionTestDependentSmallPages(entry_t** dir,
															const size_t size,
															const double selectivity,
															const field_t f,
															measurement_t* const measurement);

void randomReadSoftwareIndirectionTestDependentMixedSmallPages(entry_t** dir,
															const size_t size,
															const double selectivity,
															const size_t numRandomAccesses,
															const field_t f,
															measurement_t* const measurement);

void randomReadIndirectionTest(entry_t** indirection,
								const size_t size,
								const double selectivity,
								const field_t f,
								measurement_t* const measurement);

void randomReadTableTest(const row_t* const p,
							const size_t size,
							const double selectivity,
							const field_t f,
							measurement_t* const measurement);

#endif /* RANDOM_READ_TEST_H_ */
