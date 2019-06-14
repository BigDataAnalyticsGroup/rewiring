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
 * map_and_populate.h
 *
 *  Created on: Jul 17, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef MAP_AND_POPULATE_H_
#define MAP_AND_POPULATE_H_

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>
#include "../timediff.h"
#include "../types.h"
#include "../fields.h"
#include "../error.h"
#include "../useful_functions.h"

void mapAndPopulate(wd_pt* const workingData,
					const size_t size,
					const bool hugePages,			// otherwise small pages
					const bool file,				// otherwise anonymous
					const bool shared,				// otherwise private mapping
					const bool populate,			// populate at all?
					const bool populateByOption,	// otherwise asynchronous prefaulting
					const bool initializeFile,		// otherwise fresh file
					const bool randomShuffle,		// otherwise one sequential mapping
					measurement_t* const measurement);

void mapAndPopulateCleanup(wd_pt* const workingData,
							const size_t size,
							const bool hugePages,
							const bool file);

#endif /* MAP_AND_POPULATE_H_ */
