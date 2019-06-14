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
 * timefile.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "timefile.h"

experiment_t getFreshExperiment(const char* methodName, const size_t runs) {
	notNull(methodName, "methodName");

	experiment_t experiment;
	// set method name
	experiment.methodName = (char*) malloc(sizeof(char) * (strlen(methodName) + 1));
	strcpy(experiment.methodName, methodName);
	// set number of runs
	experiment.runs = runs;
	// allocate memory for the runs and initialize them
	experiment.measurements = (measurement_t*) malloc(sizeof(measurement_t) * experiment.runs);
	for(size_t r = 0; r < experiment.runs; ++r) {
		for(size_t f = 0; f < num_fields; ++f) {
			experiment.measurements[r].dimensions[f] = 0.0;
			experiment.measurements[r].detailedDimensions[f].numTimes = 0;
			experiment.measurements[r].detailedDimensions[f].dimension = NULL;
		}
	}
	return experiment;
}

void printExperimentalData(const experiment_t* const experiment, FILE* const file, const bool xFields_yRuns) {
	notNull(experiment, "experiment");

	// redirect to stdout if no file is provided
	FILE* const stream = file ? file : stdout;

	// compute averages
	double averages[num_fields];
	memset(averages, 0, sizeof(double) * num_fields);
	for(size_t f = 0; f < num_fields; ++f) {
		for(size_t r = 0; r < experiment->runs; ++r) {
			averages[f] += experiment->measurements[r].dimensions[f];
		}
		averages[f] /= (double) experiment->runs;
	}

	// first line: method name
	fprintf(stream, "%s\n", experiment->methodName);

	if(xFields_yRuns) {
		// x-axis: fields
		// y-axis: runs

		// second row: all field names, tab separated
		for(size_t f = 0; f < num_fields; ++f) {
			fprintf(stream, "%s", getFieldName(f));
			fprintf(stream, f < num_fields - 1 ? ", " : "\n");
		}
		// remaining rows: experimental results
		for(size_t r = 0; r < experiment->runs; ++r) {
			for(size_t f = 0; f < num_fields; ++f) {
				fprintf(stream, "%f", experiment->measurements[r].dimensions[f]);
				fprintf(stream, f < num_fields - 1 ? ", " : "\n");
			}
		}
		// final row: average of experimental results
		for(size_t f = 0; f < num_fields; ++f) {
			fprintf(stream, "%f", averages[f]);
			fprintf(stream, f < num_fields - 1 ? ", " : "\n");
		}
	}
	else {
		// x-axis: runs
		// y-axis: fields

		for(size_t f = 0; f < num_fields; ++f) {
			// first column: field name
			fprintf(stream, "%s, ", getFieldName(f));
			// remaining columns: experimental results
			for(size_t r = 0; r < experiment->runs; ++r) {
				fprintf(stream, "%f, ", experiment->measurements[r].dimensions[f]);
			}
			// final column: average of experimental result
			fprintf(stream, "%f\n", averages[f]);
		}
	}
}

void printDetailedExperimentalData(const experiment_t* const experiment, FILE* const file) {
	notNull(experiment, "experiment");

	printf("Test\n");

	// redirect to stdout if no file is provided
	FILE* const stream = file ? file : stdout;

	// first line: method name
	fprintf(stream, "%s\n", experiment->methodName);

	// second row: all field names, tab separated
	for(size_t f = 0; f < num_fields; ++f) {
		if(experiment->measurements[0].detailedDimensions[f].numTimes == 0)  {
			continue;
		}
		fprintf(stream, "%s", getFieldName(f));
		fprintf(stream, f < num_fields - 1 ? ", " : "\n");
	}

	double* averages[num_fields];
	memset(averages, 0, sizeof(*averages) * num_fields);

	size_t maxDim = 0;
	for(size_t f = 0; f < num_fields; ++f) {
		// get number of rows to print
		if(experiment->measurements[0].detailedDimensions[f].numTimes > maxDim) {
			maxDim = experiment->measurements[0].detailedDimensions[f].numTimes;
		}
	}

	// remaining rows: experimental results
	size_t numNonZeroFields = 0;

	for(size_t r = 0; r < experiment->runs; ++r) {
		numNonZeroFields = 0;
		for(size_t f = 0; f < num_fields; ++f) {
			// allocate memory for the averages, if necessary
			if(experiment->measurements[r].detailedDimensions[f].numTimes > 0) {
				++numNonZeroFields;
				if(!averages[f]) {
					averages[f] = malloc(sizeof(**averages) * maxDim);
					memset(averages[f], 0, sizeof(**averages) * maxDim);
				}
			}
		}

		// limit output to stdout
		if(!file) maxDim = 10;

		double zero = 0;
		for(size_t d = 0; d < maxDim; ++d) {
			for(size_t f = 0; f < num_fields; ++f) {
				// skip columns that are entirely zeroed
				if(experiment->measurements[r].detailedDimensions[f].numTimes == 0) {
					continue;
				}

				if(d < experiment->measurements[r].detailedDimensions[f].numTimes) {
					// print detailed measurement
					fprintf(stream, "%f", experiment->measurements[r].detailedDimensions[f].dimension[d]);
					averages[f][d] += experiment->measurements[r].detailedDimensions[f].dimension[d];
				}
				else {
					// this field has no more measurements, but another field still has, so print zeroes
					fprintf(stream, "%f", zero);
					averages[f][d] += 0;
				}
				fprintf(stream, f < numNonZeroFields - 1 ? ", " : "\n");
			}
		}
	}

	for(size_t d = 0; d < maxDim; ++d) {
		for(size_t f = 0; f < num_fields; ++f) {
			if(averages[f]) {
				averages[f][d] /= experiment->runs;
				fprintf(stream, "%f", averages[f][d]);
				if(f < numNonZeroFields - 1) {
					fprintf(stream, ", ");
				}
			}
		}
		fprintf(stream, "\n");
	}

}


void freeExperimentalData(experiment_t* const experiment) {
	notNull(experiment, "experiment");

	if(experiment->methodName) {
		free(experiment->methodName);
		experiment->methodName = NULL;
	}

	if(experiment->measurements) {
		for(size_t r = 0; r < experiment->runs; ++r) {
			for(size_t f = 0; f < num_fields; ++f) {
				if(experiment->measurements[r].detailedDimensions[f].numTimes > 0) {
					free(experiment->measurements[r].detailedDimensions[f].dimension);
					experiment->measurements[r].detailedDimensions[f].dimension = NULL;
				}
			}
		}

		free(experiment->measurements);
		experiment->measurements = NULL;
	}
}


measurement_t getFreshMeasurement() {
	measurement_t measurement;
	for(size_t f = 0; f < num_fields; ++f) {
		measurement.dimensions[f] = 0.0;
		measurement.detailedDimensions[f].numTimes = 0;
		measurement.detailedDimensions[f].dimension = NULL;
	}
	return measurement;
}

void prepareDetailedTimeDifference(const field_t field,
									  measurement_t* const measurement,
									  const size_t numTimes) {
	measurement->detailedDimensions[field].dimension = malloc(sizeof(*(measurement->detailedDimensions[field].dimension)) * numTimes);
	measurement->detailedDimensions[field].numTimes = numTimes;
}
