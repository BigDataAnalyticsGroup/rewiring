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
 * timediff.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "timediff.h"

void measure(timeval_t* time) {
	gettimeofday(time, NULL);
}

void measureHighRes(timespec_t* time) {
    clock_gettime(CLOCK_REALTIME, time);
}

double timeDifference(const timeval_t* first, const timeval_t* second) {
    double total_time = (second->tv_sec - first->tv_sec) * 1000000 + (second->tv_usec - first->tv_usec);
    // return seconds
    return total_time / 1000000;
}

double timeDifferenceHighRes(const timespec_t* first, const timespec_t* second) {
    double total_time = (second->tv_sec - first->tv_sec) * 1000000000 + (second->tv_nsec - first->tv_nsec);
    // return microseconds
    return total_time / 1000;
}

void printTimeDifference(timeval_t* start,
						   timeval_t* end,
						   const field_t field,
						   measurement_t* const measurement) {
	double diff = timeDifference(start, end);
	measurement->dimensions[field] = diff;
	printf("%f,%s\n", diff, getFieldName(field));
}

void printTimeDifferenceChecksum(timeval_t* start,
									timeval_t* end,
									const field_t field,
									const entry_t total,
									measurement_t* const measurement) {
	double diff = timeDifference(start, end);
	measurement->dimensions[field] = diff;
	printf("%f,%s //total=%ld\n", diff, getFieldName(field), (uint64_t) total);
}

void printTimeDifferenceSelectivityChecksum(timeval_t* start,
												timeval_t* end,
												const field_t field,
												const double selectivity,
												const entry_t total,
												measurement_t* const measurement) {
	double diff = timeDifference(start, end);
	measurement->dimensions[field] = diff;
	printf("%f,%s,%f //total=%ld\n", diff, getFieldName(field), selectivity, (uint64_t) total);
}

void printTimeDifferenceSelectivityPercentChecksum(timeval_t* start,
														timeval_t* end,
														const field_t field,
														const size_t effective_size,
														const double selectivity,
														const entry_t total,
														measurement_t* const measurement) {
	double diff = timeDifference(start, end);
	measurement->dimensions[field] = diff;
	printf("%f,%s //effective_size=%ld, selectivity=%f%%, total=%ld\n", diff, getFieldName(field), effective_size, selectivity, (uint64_t) total);
}


void collectDetailedTimeDifference(timespec_t* start,
							timespec_t* end,
							const size_t iteration,
							const field_t field,
							measurement_t* const measurement) {
	double diff = timeDifferenceHighRes(start, end);
	measurement->detailedDimensions[field].dimension[iteration] = diff;
}

void collectDetailedTime(const double diff,
							const size_t iteration,
							const field_t field,
							measurement_t* const measurement) {
	measurement->detailedDimensions[field].dimension[iteration] = diff;
}






