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
 * bitset.h
 *
 *  Created on: Sep 23, 2014
 *      Author: Felix Martin Schuhknecht
 */

#ifndef BITSET_H_
#define BITSET_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef byte* bitset_t;

/**
 * 	Allocates and initializes the bitset to 0
 * 	numElems must be a multiple of 8
 */
bitset_t initBitset(const unsigned int numElems);

/**
 * 	Free the bitset
 */
void freeBitset(bitset_t bitset);

/**
 * 	Get the bit at a specified index as a boolean
 * 	INDEX must be unsigned!
 */
#define GET_BIT_MACRO(BITSET, INDEX) (bool)(*(BITSET + (INDEX >> 3)) & (1 << (INDEX & 7)))

/**
 * 	Set the bit at a specified index
 * 	INDEX must be unsigned!
 */
#define SET_BIT_MACRO(BITSET, INDEX) (*(BITSET + (INDEX >> 3)) |= 1 << (INDEX & 7))

/**
 * 	Toogle the bit at a specified index
 * 	INDEX must be unsigned!
 */
#define TOOGLE_BIT_MACRO(BITSET, INDEX) (*(BITSET + (INDEX >> 3)) ^= 1 << (INDEX & 7))

#endif /* BITSET_H_ */
