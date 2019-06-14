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
 * random_write_test.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "random_write_test.h"

void randomWriteTest(entry_t* const p, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * selectivity;

    srand(SEED);
    measure(&start);
    entry_t total = 0;
    for (size_t i = 0; i < num_reads; i++) {
        size_t index = ( (1.0 * rand()) / RAND_MAX) * (num_reads - 1);
        p[index] = size - 1 - index;
        total = total + p[index];
    }
    measure(&end);
    printTimeDifferenceSelectivityChecksum(&start, &end, f, selectivity, total, measurement);
}

void randomWriteTableTest(row_t* const p, const size_t size, const double selectivity, const field_t f, measurement_t* const measurement) {
	notNull(p, "p");
	notNull(measurement, "measurement");

	timeval_t start, end;

    const size_t num_reads = size * selectivity;

    srand(SEED);
    measure(&start);
    entry_t total = 0;
    for (size_t i = 0; i < num_reads; i++) {
        size_t index = ( (1.0 * rand()) / RAND_MAX) * (num_reads - 1);
        p[index].cols[0] = size - 1 - index;
        total = total + p[index].cols[0];
    }
    measure(&end);
    printTimeDifferenceSelectivityChecksum(&start, &end, f, selectivity, total, measurement);
}
