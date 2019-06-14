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
 * parser.c
 *
 *  Created on: Mar 06, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "parser.h"

void parseArgument(const char* arg, void* resultType, parameters_t* parameters, bool isMethodArgument) {
	notNull(arg, "arg");
	notNull(resultType, "resultType");
	notNull(parameters, "parameters");

	// distinct between method and test type parsing
	copyMethod_t* copyMethod = NULL;
	testType_t* testType = NULL;
	char* openingBracket = NULL;
	if(isMethodArgument) {
		printf("Parsing copy method.\n");
		copyMethod = (copyMethod_t*) resultType;
		// parse leading copy metgid
		*copyMethod = (int) strtol(arg, &openingBracket, 10);
	}
	else {
		printf("Parsing test type.\n");
		testType = (testType_t*) resultType;
		// parse leading test type
		*testType = (int) strtol(arg, &openingBracket, 10);
	}

	if(*openingBracket != '\0') {
		// the user passed an additional parameter
		// check whether it starts with a '('
		if(*openingBracket != '(') { printf("Error: pass test_parameter inside brackets: (test_parameter)\n"); exit(1); }
		// get start of parameter
		const char* parameterStart = openingBracket + 1;
		// parse one or more parameters, comma separated

		char* afterParameter = NULL;
		double parsedParameters[MAX_NUMBER_OF_PARAMETERS];
		size_t currentParameter = 0;
		do {
			if(afterParameter && *afterParameter != ',') { printf("Error: parameters not separated using ','.\n"); exit(1); }
			if(afterParameter) { parameterStart = afterParameter + 1; };
			parsedParameters[currentParameter++] = strtod(parameterStart, &afterParameter);
		} while(*afterParameter != ')');
		// check whether it ends with a ')'
		if(*afterParameter != ')') { printf("Error: pass test_parameter inside brackets: (test_parameter)\n"); exit(1); }
		// check whether the test type is compatible with the parameter and assign it

		if(isMethodArgument) {
			switch(*copyMethod) {
				case COW_MEMCPY_SHUFFLE:
				case COW_MEMCPY_SHUFFLE_POPULATE:
				case COW_MEMCPY_SHUFFLE_HUGE:
				case COW_MEMCPY_SHUFFLE_HUGE_POPULATE:
				case COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES:
				case COW_MEMCPY_SHUFFLE_REMAP_FILE_PAGES_HUGE:
					if(currentParameter != 1) { printf("Error: Expected 1 parameter.\n"); exit(1); }
					parameters->pagesPerChunk = (size_t) parsedParameters[0]; // the argument will be an integer
					break;
				case TR_MEMCPY_SHUFFLE:
				case TR_MEMCPY_SHUFFLE_HUGE:
					if(currentParameter != 2) { printf("Error: Expected 2 parameter.\n"); exit(1); }
					parameters->pagesPerChunk = (size_t) parsedParameters[0]; // the argument will be an integer
					parameters->initializeFile = parsedParameters[1] == 0 ? false : true;
					break;
				case MAPPED_PARTITIONING:
				case MAPPED_PARTITIONING_HUGE:
				case TR_PARTITIONING_BLOCK_CHAINS:
				case MAPPED_PARTITIONING_BLOCK_CHAINS:
				case MAPPED_PARTITIONING_BLOCK_CHAINS_HUGE:
				case MREMAPED_PARTITIONING:
					if(currentParameter != 2) { printf("Error: Expected 2 parameters.\n"); exit(1); }
					parameters->numPartitions = (size_t) parsedParameters[0]; // the argument will be an integer
					parameters->pagesPerBlock = (size_t) parsedParameters[1]; // the argument will be an integer
					break;
				case TR_PARTITIONING_WITH_HISTO:
					if(currentParameter != 3) { printf("Error: Expected 3 parameters.\n"); exit(1); }
					parameters->numPartitions = (size_t) parsedParameters[0]; // the argument will be an integer
					parameters->pagesPerBlock = (size_t) parsedParameters[1]; // the argument will be an integer
					parameters->populate = (size_t) parsedParameters[2]; // the argument will be an integer
					break;
				case MAPPED_PARTITIONING_BLOCK_CHAINS_ADAPTIVE:
					if(currentParameter != 1) { printf("Error: Expected 1 parameter.\n"); exit(1); }
					parameters->numPartitions = (size_t) parsedParameters[0];
					break;
				case MAP_AND_POPULATE:
					if(currentParameter != 7) { printf("Error: Expected 7 parameters.\n"); exit(1); }
					parameters->hugePages = parsedParameters[0] == 0 ? false : true;
					parameters->file = parsedParameters[1] == 0 ? false : true;
					parameters->shared = parsedParameters[2] == 0 ? false : true;
					parameters->populate = parsedParameters[3] == 0 ? false : true;
					parameters->populateByOption = parsedParameters[4] == 0 ? false : true;
					parameters->initializeFile = parsedParameters[5] == 0 ? false : true;
					parameters->randomShuffle = parsedParameters[6] == 0 ? false : true;
					break;
				case ALLOCATION_REWIRING:
				case ALLOCATION_SOFTWARE_INDIRECTION:
					if(currentParameter != 2) { printf("Error: Expected 2 parameters.\n"); exit(1); }
					parameters->hugePages = parsedParameters[0] == 0 ? false : true;
					parameters->randomMapping = parsedParameters[1] == 0 ? false : true;
					break;
				case ALLOCATION_MMAP:
					if(currentParameter != 1) { printf("Error: Expected 1 parameters.\n"); exit(1); }
					parameters->hugePages = parsedParameters[0] == 0 ? false : true;
					break;
				default: {
					printf("Error: passed parameter for copy method, that does not need a parameter.\n");
					exit(1);
				}
			}
		}
		else {
			switch(*testType) {
				case RAND_READ_SRC_TEST:
				case RAND_READ_DST_TEST:
				case RANDOM_READ_DST_DEPENDENT_TEST:
				case SEQ_READ_DST_DEPENDENT_TEST:
				case RAND_READ_DST_INDIRECTION_TEST:
				case SEQ_WRITE_SRC_TEST:
				case SEQ_WRITE_DST_TEST:
				case RAND_WRITE_SRC_TEST:
				case RAND_WRITE_DST_TEST:
				case SEQ_READ_SRC_TEST:
				case SEQ_READ_DST_TEST:
					if(currentParameter != 1) { printf("Error: Expected 1 parameter.\n"); exit(1); }
					parameters->selectivity = parsedParameters[0];
					break;
				case PAGE_FAULT_TEST:
					if(currentParameter != 1) { printf("Error: Expected 1 parameter.\n"); exit(1); }
					parameters->hugePages = parsedParameters[0] == 0 ? false : true;
					break;
				case RANDOM_READ_DST_DEPENDENT_MIXED_TEST:
					if(currentParameter != 2) { printf("Error: Expected 2 parameters.\n"); exit(1); }
					parameters->selectivity = parsedParameters[0];
					parameters->numRandomAccesses = parsedParameters[1];
					break;
				default: {
					printf("Error: passed parameter for test type, that does not need a parameter.\n");
					exit(1);
				}
			}
		}
	}
	else {
		// the user did not pass an additional parameter
		// check whether the test type is allowed to pass without a parameter
		if(isMethodArgument) {
			switch(*copyMethod) {
				case TR_MEMCPY:
				case COW_MEMCPY:
				case COW_MEMCPY_POPULATE:
				case COW_MEMCPY_HUGE:
				case COW_MEMCPY_HUGE_POPULATE:
				case COW_HUGE_SMALL_MIX:
				case REWIRED_VECTOR:
				case REWIRED_VECTOR_HUGE:
				case MREMAPED_VECTOR:
				case MREMAPED_VECTOR_HUGE:
				case REWIRED_VECTOR_PREPOPULATE:
				case REWIRED_VECTOR_HUGE_PREPOPULATE:
				case STL_VECTOR:
				case SOFTWARE_IND_VECTOR_HUGE:
					break;
				default: {
					printf("Error: missing parameter for copy method.\n");
					exit(1);
				}
			}
		}
		else {
			switch(*testType) {
				case SEQ_READ_DST_FROM_SEVERAL_CHUNKS:
				case SEQ_READ_DST_MERGE_FROM_CHUNKS:
				case SEQ_READ_DST_TWICE_TEST:
				case SEQ_WRITE_DST_TWICE_TEST:
				case RADIX_SORT_DST_TEST:
				case RADIX_SORT_PER_PARTITION_CONSECUTIVE:
				case RADIX_SORT_PER_PARTITION_INDIRECTION:
				case VECTOR_INSERT_TEST:
					break;
				default: {
					printf("Error: missing parameter for test type.\n");
					exit(1);
				}
			}
		}
	}
}




