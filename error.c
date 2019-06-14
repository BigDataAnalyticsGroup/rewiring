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
 * error.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "error.h"

void notNull(const void* const ptr, const char* ptrName) {
	if(!ptrName) { printf("Error: please provide a ptr name."); exit(1); }
	if(!ptr) { printf("Error: argument %s is NULL", ptrName); exit(1); }
}

void validPointer(const void* const ptr, intptr_t errorValue, const char* functionName, const bool exitOnError) {
	intptr_t intPtr = (intptr_t) ptr;
	if(intPtr == errorValue) {
		printf("Error: pointer has the address %" PRIiPTR ", which is the error value of %s.\n", errorValue, functionName);
		if(exitOnError) exit(1);
	}
}

void resetErrno() {
	errno = 0;
}

void checkErrno(const char* causer, const bool exitOnError) {
	if(errno != 0) {
		printf("Error: errno = %d, probably caused by %s.\n", errno, causer);
		if(exitOnError) exit(1);
	}
}
