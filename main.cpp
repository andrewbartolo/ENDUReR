/*
 * This file defines a test driver for the ENDUReR mechanism.
 *
 * Note that ENDUReR addresses are 16-bit unsigned ints (e_address).
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "endurer.h"

#define TEST_NUM_ITERS 13
#define TEST_SEG_SIZE 10000000
#define RESULT_VEC_LEN 1000

e_data A[RESULT_VEC_LEN];
e_data B[RESULT_VEC_LEN];

int main(int argc, char *argv[]) {
    printf("Hello from ENDUReR.\n");

    // initialize the test results vectors
    memset(A, 0, sizeof(e_data)*RESULT_VEC_LEN);
    memset(B, 0, sizeof(e_data)*RESULT_VEC_LEN);

    // TODO clean up initialize()
    void *seg_base = initialize(TEST_SEG_SIZE);

    for (size_t i = 0; i < TEST_NUM_ITERS; ++i) {
        e_address address = (e_address)i;
        e_data data = (e_data) 1000 - i;

        // reference
        A[i] = data;

        // simple consistency check
        write_word(address, data);
        B[i] = read_word(address);
    }

    for (size_t i = 0; i < RESULT_VEC_LEN; ++i) {
        assert(A[i] == B[i]);
    }
    printf("Results vectors match.\n");

    printf("Done.\n");

    return 0;
}
