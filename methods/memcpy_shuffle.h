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
 * memcpy_shuffle.h
 *
 *  Created on: Feb 11, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef MEMCPY_SHUFFLE_H_
#define MEMCPY_SHUFFLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../timediff.h"
#include "../types.h"
#include "../fields.h"
#include "../error.h"
#include "../timefile.h"
#include "../useful_functions.h"

void traditionalMemcpyShuffle(wd_pt* const workingData,
								const size_t size,
								const size_t pagesPerChunk,
								const bool hugePages,
								const bool initialized,
								measurement_t* const measurement);

#endif /* MEMCPY_SHUFFLE_H_ */
