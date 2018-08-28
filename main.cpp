/*
 * This file defines a test driver for the ENDUReR mechanism.
 */
#include "endurer.h"
#include <stdio.h>

#define TEST_NUM_ITERS 4
#define TEST_SEG_SIZE 10000000


int main(int argc, char *argv[]) {
    printf("Hello from ENDUReR.\n");

    void *seg_base = initialize(TEST_SEG_SIZE);
    printf("Segment base is at %x\n", seg_base);

    for (size_t i = 0; i < TEST_NUM_ITERS; ++i) {
        read_word((const unsigned int)i);
        write_word((const unsigned int)i, (const unsigned int)i);
    }

    return 0;
}
