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
 * timefile.h
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef TIMEFILE_H_
#define TIMEFILE_H_

#include <string.h>
#include "types.h"
#include "fields.h"
#include "error.h"

typedef struct {
	double* dimension;
	size_t numTimes;
} detailedDimension_t;

typedef struct {
	double dimensions[num_fields];
	detailedDimension_t detailedDimensions[num_fields];
} measurement_t;

typedef struct {
	measurement_t* measurements;
	char* methodName;
	size_t runs;
} experiment_t;

experiment_t getFreshExperiment(const char* methodName, const size_t runs);
void printExperimentalData(const experiment_t* const experiment, FILE* const file, const bool xFields_yRuns);
void printDetailedExperimentalData(const experiment_t* const experiment, FILE* const file);
void freeExperimentalData(experiment_t* const experiment);
measurement_t getFreshMeasurement();

void prepareDetailedTimeDifference(const field_t field,
									  measurement_t* const measurement,
									  const size_t numTimes);

#endif /* TIMEFILE_H_ */
