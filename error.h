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
 * error.h
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include "types.h"

void notNull(const void* const ptr, const char* ptrName);
void validPointer(const void* const ptr, intptr_t errorValue, const char* functionName, const bool exitOnError);
void resetErrno();
void checkErrno(const char* causer, const bool exitOnError);

#ifdef ERRORCHECK
#define VALID_PTR(ptr, errorValue, functionName, exitOnError) (validPointer(ptr, errorValue, functionName, exitOnError))
#else
#define VALID_PTR(ptr, errorValue, functionName, exitOnError)
#endif

#ifdef ERRORCHECK
#define ERRNO_CHECK(code, message, exitOnError) resetErrno(); code; checkErrno(message, exitOnError);
#else
#define ERRNO_CHECK(code, message, exitOnError) code;
#endif

#ifdef ERRORCHECK
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#endif /* ERROR_H_ */
