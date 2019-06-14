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
 * bitset.c
 *
 *  Created on: Sep 23, 2014
 *      Author: Felix Martin Schuhknecht
 */

#include "bitset.h"

bitset_t initBitset(const unsigned int numElems) {
	const size_t sizeInByte = numElems / 8;
	__attribute__((aligned(64))) bitset_t bitset = NULL;
	posix_memalign((void**) &bitset, 64, sizeInByte);
	memset(bitset, 0, sizeInByte);
	return bitset;
}

void freeBitset(bitset_t bitset) {
	if(bitset) free(bitset);
}




