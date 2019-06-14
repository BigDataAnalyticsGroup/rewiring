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
 * fields.c
 *
 *  Created on: Jan 30, 2015
 *      Author: Felix Martin Schuhknecht, Pankaj Khanchandani
 */

#include "fields.h"

const char* getFieldName(field_t field) {
	switch(field) {
		case MALLOC_SRC: return "malloc-src";
		case INIT_MALLOC_SRC: return "init-malloc-src";
		case MALLOC_DST: return "malloc-dst";
		case MEMCPY: return "memcpy";
		case SHM_SRC: return "shm-src";
		case MMAP_SRC: return "mmap-src";
		case INIT_SHM_SRC: return "init-shm-src";
		case MMAP_DST: return "mmap-dst";
		case REMAP_DST: return "remap-dst";
		case SHM_SRC_HUGE: return "shm-src-huge";
		case READ_SEQ_SRC: return "read-seq-src";
		case READ_SEQ_DST: return "read-seq-dst";
		case READ2_SEQ_DST: return "read2-seq-dst";
		case READ_RAND_SRC: return "read-rand-src";
		case READ_RAND_DST: return "read-rand-dst";
		case WRITE_SEQ_SRC: return "write-seq-src";
		case WRITE_SEQ_DST: return "write-seq-dst";
		case WRITE_RAND_SRC: return "write-rand-src";
		case WRITE_RAND_DST: return "write-rand-dst";
		case WRITE_SEQ_DST_PER_ELEM: return "write-seq-dst-per-elem";
		case BUILD_HISTO: return "build-histogram";
		case SQUEEZE_AREA_COPY: return "squeeze-area-copy";
		case SQUEEZE_AREA_MMAP: return "squeeze-area-mmap";
		case SHM_DST: return "shm-dst";
		case SHM_DST_HUGE: return "shm-dst-huge";
		case MERGE: return "merge";
		case RADIX_SORT: return "radix-sort";
		case INDIRECTION: return "indirection";
		case VECTOR_INSERT: return "vector-insert";
		case CRACK_IN_PLACE: return "crack-in-place";
		case INDEX_INSERT: return "index-insert";
		case INDEX_ACCESS: return "index-access";
		case WRITE2_SEQ_DST: return "write2-seq-dst";
		default: printf("%s\n", "Error: Field to name mapping unknown."); exit(1);
	}
}


