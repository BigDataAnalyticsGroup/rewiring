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
 * timediff.h
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef TIMEDIFF_H_
#define TIMEDIFF_H_

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "types.h"
#include "timefile.h"

void measure(timeval_t* time);
void measureHighRes(timespec_t* time);
double timeDifference(const timeval_t* first, const timeval_t* second);
double timeDifferenceHighRes(const timespec_t* first, const timespec_t* second);

void printTimeDifference(	timeval_t* start,
							timeval_t* end,
							const field_t field,
							measurement_t* const measurement);

void printTimeDifferenceChecksum(timeval_t* start,
									timeval_t* end,
									const field_t field,
									const entry_t total,
									measurement_t* const measurement);

void printTimeDifferenceSelectivityChecksum(timeval_t* start,
												timeval_t* end,
												const field_t field,
												const double selectivity,
												const entry_t total,
												measurement_t* const measurement);

void printTimeDifferenceSelectivityPercentChecksum(timeval_t* start,
														timeval_t* end,
														const field_t field,
														const size_t effective_size,
														const double selectivity,
														const entry_t total,
														measurement_t* const measurement);

void collectDetailedTimeDifference(timespec_t* start,
							timespec_t* end,
							const size_t iteration,
							const field_t field,
							measurement_t* const measurement);

void collectDetailedTime(const double diff,
							const size_t iteration,
							const field_t field,
							measurement_t* const measurement);

#endif /* TIMEDIFF_H_ */
