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
 * main.h
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef MAIN_H_
#define MAIN_H_

// methods
#include "methods/memcpy.h"
#include "methods/memcpy_shuffle.h"
#include "methods/cow_memcpy.h"
#include "methods/cow_memcpy_huge.h"
#include "methods/cow_memcpy_shuffle.h"
#include "methods/traditional_partitioning.h"
#include "methods/mapped_partitioning.h"
#include "methods/blocked_partitioning.h"
#include "methods/mapped_blocked_partitioning.h"
#include "methods/mapped_blocked_partitioning_adaptive.h"
#include "methods/cow_huge_small_mix.h"
#include "methods/rewired_vector.h"
#include "methods/map_and_populate.h"
#include "methods/mremaped_vector.h"
#include "methods/allocation_rewiring.h"
#include "methods/allocation_softwaredir.h"
#include "methods/allocation_mmap.h"
#include "methods/softwareind_vector.h"
#include "methods/mremaped_partitioning.h"

// tests
#include "tests/sequential_read_test.h"
#include "tests/random_read_test.h"
#include "tests/sequential_write_test.h"
#include "tests/random_write_test.h"
#include "tests/radix_sort_test.h"
#include "tests/vector_pushback_test.h"
#include "tests/page_fault_test.h"

#include "timefile.h"
#include "macros.h"
#include "parser.h"

void run(const copyMethod_t copyMethod, const testType_t testType, const parameters_t* testParameters, const size_t size, measurement_t* const measurement, const bool detailed);
void check(const entry_t* const src, const entry_t* const dst, const testType_t testType, const size_t size);

#endif /* MAIN_H_ */
