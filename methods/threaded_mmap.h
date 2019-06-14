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
 * threaded_mmap.h
 *
 *  Created on: Dec 09, 2015
 *      Author: Felix Martin Schuhknecht
 */

#ifndef THREADED_MMAP_H_
#define THREADED_MMAP_H_

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "../macros.h"
#include "../error.h"

typedef struct {
	uint8_t* vmem;
	pthread_mutex_t m;
	pthread_cond_t c;
	filedescriptor_t fd;
} threadData;


void* rewirePage(void* data) {
	threadData* td = (threadData*) data;

	pthread_mutex_lock(&td->m);
	pthread_cond_wait(&td->c, &td->m);

	// rewire page
	printf("before rewiring\n");
    ERRNO_CHECK(
    mmap(td->vmem, HUGE_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, td->fd, HUGE_PAGE_SIZE);
    VALID_PTR(td->vmem, -1, "mmap vmem", false);
    , "mmap vmem", true);
	printf("after rewiring\n");
	pthread_mutex_unlock(&td->m);

    uint32_t checksum = 0;
    for(size_t i = 0; i < HUGE_PAGE_SIZE; ++i) {
    	checksum += td->vmem[i];
    }
    printf("Thread Checksum = %" PRIu32 "\n", checksum);

	return NULL;
}

void threadedMmap() {
	// create a file with two pages
    ERRNO_CHECK(
    filedescriptor_t fd = open("/mnt/hugetlbfs/test", O_RDWR | O_CREAT, 0666);
    , "open", true);

    ERRNO_CHECK(
    uint8_t* tmp = (uint8_t*) mmap(NULL, 2 * HUGE_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    VALID_PTR(tmp, -1, "mmap tmp", false);
    , "mmap tmp", true);
    memset(tmp, 1, HUGE_PAGE_SIZE);
    memset(tmp + HUGE_PAGE_SIZE, 2, HUGE_PAGE_SIZE);

    // map to first page
    ERRNO_CHECK(
    uint8_t* vmem = (uint8_t*) mmap(NULL, HUGE_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    VALID_PTR(vmem, -1, "mmap vmem", false);
    , "mmap vmem", true);


    // create mmap thread
    pthread_t thread;
    threadData data;
    data.vmem = vmem;
    data.fd = fd;
    pthread_mutex_init(&data.m, NULL);
    pthread_cond_init(&data.c, NULL);
    pthread_create(&thread, NULL, rewirePage, (void*) &data);

    // read first page and sum up
    const size_t halfTheHugePage = HUGE_PAGE_SIZE / 2;
    uint32_t checksum = 0;
	printf("before first loop\n");
    for(size_t i = 0; i < halfTheHugePage; ++i) {
    	checksum += vmem[i];
    }
	printf("after first loop\n");
    pthread_cond_signal(&data.c);
	printf("before second loop\n");
    for(size_t i = halfTheHugePage; i < HUGE_PAGE_SIZE; ++i) {
    	checksum += vmem[i];
    }
	printf("after second loop\n");
    printf("Checksum = %" PRIu32 "\n", checksum);

    // cleanup
    pthread_join(thread, NULL);

	ERRNO_CHECK(
    munmap(tmp, 2 * HUGE_PAGE_SIZE);
	tmp = NULL;
    , "munmap tmp", true);

	ERRNO_CHECK(
    munmap(vmem, HUGE_PAGE_SIZE);
	vmem = NULL;
    , "munmap vmem", true);

	ERRNO_CHECK(
	close(fd);
	fd = -1;
	, "close", true);
	ERRNO_CHECK(
	unlink("/mnt/hugetlbfs/test");
	, "unlinck /mnt/hugetlbfs/test", true);
}

#endif /* ALLOCATION_REWIRING_H_ */
