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
 * allocation_softwaredir.h
 *
 *  Created on: Oct 19, 2015
 *      Author: Felix Martin Schuhknecht
 */

#ifndef ALLOCATION_SOFTWAREDIR_H_
#define ALLOCATION_SOFTWAREDIR_H_

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../timediff.h"
#include "../types.h"
#include "../fields.h"
#include "../error.h"
#include "../useful_functions.h"

void allocationSoftwareIndirection(wd_pt* const workingData,
						const size_t size,
						const bool hugePages,
						const bool randomMapping,
						measurement_t* const measurement);

void allocationSoftwareIndirectionCleanup(wd_pt* const workingData);

#endif /* ALLOCATION_SOFTWAREDIR_H_ */
