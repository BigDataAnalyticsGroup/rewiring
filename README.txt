///////////////////////////////////////////////////////////
//                                                       //
//    RUMA has it: Rewired User-space Memory Access      //
//                   is Possible!                        //
// Felix Martin Schuhknecht, Jens Dittrich, Ankur Sharma //
//   Proceedings of the VLDB Endowment, Vol. 9, No. 10   //
//                                                       //
//                    Used Code Base                     //
//                Version August 30, 2016                //
//               Information Systems Group               //
//                  Saarland University                  //
//               infosys.cs.uni-saarland.de              //
//                                                       //
///////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////
//                       Setup                           //
///////////////////////////////////////////////////////////

The codebase must be run on a Linux OS (tested with Debian 8.1, Kernel 3.16). The system has to be set up slightly to run the codebase. Root access is required for some of the changes. Perform the following steps:

- Preallocate a certain number of huge pages (42 here) on a single NUMA node (0 here) with "numactl -m 0 echo 42 > /proc/sys/vm/nr_hugepages_mempolicy". Make sure enough huge pages are allocated for the corresponding test.

- Mount a hugetlbfs under /mnt/hugetlbfs. First create a folder named hugetlbfs under /mnt/ and then mount it using "mount -t hugetlbfs none /mnt/hugetlbfs -o pagesize=2M".
For testing, we recommend to open the folder completely using "chmod 777 /mnt/hugetlbfs" to avoid access right issues.

- /proc/sys/vm/overcommit_memory sets the overcommit policy. By default, it is set to 0 which disallows obvious overcommits. We recommend setting it to 1 which allows overcommits without a check.

- /proc/sys/vm/max_map_count limits the number of mappings that are possible at the same time. By default it is set to 2^16, we recommend setting it to 2^24.

- /sys/kernel/mm/transparent_hugepage/enabled can be set to "always", "madvice", and "never". It influences when vmem is backed by transparent hugepages.



///////////////////////////////////////////////////////////
//                    Compilation                        //
///////////////////////////////////////////////////////////

The code base can be build under Linux using the two scripts "compile.sh" and "compile_debug.sh" in the main folder. The compilation has been tested with gcc/g++ 4.9.



///////////////////////////////////////////////////////////
//                    Execution                          //
///////////////////////////////////////////////////////////

An execution of ./Rewiring_Microbench results in the following output, showing how and which tests can be run. Note that not all methods can be combined with all tests. The codebase will catch invalid combinations.

Usage: ./a.out <method> <test_type>[(test_parameter)] <number_of_entries> <number_of_repetitions> <detailed (true|false)> [<output_file>]
Methods
1. Memcpy
2. Cow memcpy
3. Cow memcpy using populate
4. Cow memcpy using huge pages
5. Cow memcpy using huge pages and populate
6. Cow memcpy with shuffling
7. Cow memcpy with shuffling using populate
8. Cow memcpy with shuffling using huge pages
9. Cow memcpy with shuffling using huge pages and populate
10. Memcpy with shuffling
11. Memcpy with shuffling using huge pages
12. Traditional partitioning with histogram
13. Mapped partitioning without histogram
14. Mapped partitioning without histogram using huge pages
15. Cow memcpy with shuffling using remap_file_pages
16. Cow memcpy with shuffling using remap_file_pages and huge pages
17. Traditional partitioning with block-chains
18. Mapped partitioning with block-chains
19. Mapped partitioning with block-chains using huge pages
20. Mix huge and small pages
21. Mapped partitioning with block-chains using adaptive block sizes
22. Rewired vector
23. Rewired vector using huge pages
24. STL vector
30. Rewired vector using prepopulation
31. Rewired vector using huge pages and prepopulation
32. Map and Populate
36. Mremaped vector
37. Mremaped vector using huge pages
40. Allocation Rewiring (Pool)
41. Allocation Software Indirection (Pool)
42. Allocation Mmap
43. Software Indirection Vector (hugepages)
44. Mremaped partitioning using huge pages
Test Types
1. Read sequential x% src (x is test_parameter)
2. Read sequential x% dst (x is test_parameter)
3. Read x% random,uniform src (x is test_parameter).
4. Read x% random,uniform dst (x is test_parameter).
5. Write sequential x% src (x is test_parameter).
6. Write sequential x% dst (x is test_parameter).
7. Write x% random,uniform src (x is test_parameter).
8. Write x% random,uniform dst (x is test_parameter).
9. Read sequential all dst (two times)
10. Read sequential all dst from several chunks
11. Read sequential all dst and merge several chunks
12. Radix-sort all dst
13. Radix-sort all dst from several partitions (from consecutive memory region)
14. Radix-sort all dst from several partitions (using an indirection)
15. Vector insertion test
18. Page fault test
19. Read x% random,uniform dst (x is test_parameter) through an indirection.
20. Write sequential all dst (two times)
21. Read x% random,uniform dst (x is test_parameter) with dependent access.
22. Read x% random,uniform dst (x is test_parameter) with dependent access (mixed).
23. Read sequential x% dst (x is test_parameter) with dependent access.

An example execution could be:

./Rewiring_Microbench "1" "2(100)" "100000" "3" "false"

This executes the first method in combination with the second test type (reading 100% of the data) for 100000 elements. The run is repeated 3 times and no detailed statistics are collected. As no additional filename is specified, the output is presented on the command line.



///////////////////////////////////////////////////////////
//                       Output                          //
///////////////////////////////////////////////////////////

The output of the previous test is:

Parsing copy method.
Parsing test type.
Number of elements = 100000
Number of runs = 3

=============== Run 1 ===============
Traditional memcpy:
0.000007,malloc-src
0.000577,init-malloc-src
0.000006,malloc-dst
0.000612,memcpy

Starting tests:
0.000101,read-seq-dst //total=4999950000
Cleanup:
Freeing malloc-src
Freeing malloc-dst

=============== Run 2 ===============
Traditional memcpy:
0.000007,malloc-src
0.000429,init-malloc-src
0.000006,malloc-dst
0.000443,memcpy

Starting tests:
0.000101,read-seq-dst //total=4999950000
Cleanup:
Freeing malloc-src
Freeing malloc-dst

=============== Run 3 ===============
Traditional memcpy:
0.000006,malloc-src
0.000373,init-malloc-src
0.000006,malloc-dst
0.000454,memcpy

Starting tests:
0.000100,read-seq-dst //total=4999950000
Cleanup:
Freeing malloc-src
Freeing malloc-dst

Traditional memcpy
malloc-src, init-malloc-src, malloc-dst, memcpy, shm-src, mmap-src, init-shm-src, mmap-dst, remap-dst, shm-src-huge, read-seq-src, read-seq-dst, read2-seq-dst, read-rand-src, read-rand-dst, write-seq-src, write-seq-dst, write-rand-src, write-rand-dst, write-seq-dst-per-elem, build-histogram, squeeze-area-copy, squeeze-area-mmap, shm-dst, shm-dst-huge, merge, radix-sort, indirection, vector-insert, crack-in-place, index-insert, index-access, write2-seq-dst
0.000007, 0.000577, 0.000006, 0.000612, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000101, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000
0.000007, 0.000429, 0.000006, 0.000443, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000101, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000
0.000006, 0.000373, 0.000006, 0.000454, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000100, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000
0.000007, 0.000460, 0.000006, 0.000503, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000101, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000

First, the three executed runs are shown in detail. Then an overview output is generated that shows the three runs in tabular format and adds a row for the average of the three runs.
