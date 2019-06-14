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
 * page_fault_test.h
 *
 *  Created on: Jul 24, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#ifndef PAGE_FAULT_H_
#define PAGE_FAULT_H_

#include "../fields.h"
#include "../types.h"
#include "../timediff.h"
#include "../error.h"

void pageFaultTest(entry_t* const p, const size_t size, const bool hugePages, const field_t f, measurement_t* const measurement);

#endif /* PAGE_FAULT_H_ */
