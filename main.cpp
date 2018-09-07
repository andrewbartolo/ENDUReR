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
#define RESULT_VEC_LEN 18

e_data A[RESULT_VEC_LEN];
e_data B[RESULT_VEC_LEN];

/* Compares A and B element-wise. Returns 0 if A and B match; 1 otherwise. */
int vec_equals(e_data *A, e_data *B) {
    for (e_uint i = 0; i < RESULT_VEC_LEN; ++i) {
        if (A[i] != B[i]) {
            return 1;
        }
    }
    return 0;
}

int test_0() {
    printf("--------Begin %s()--------\n", __func__);
    // initialize the test results vectors
    memset(A, 0, sizeof(e_data)*RESULT_VEC_LEN);
    memset(B, 0, sizeof(e_data)*RESULT_VEC_LEN);

    for (e_uint i = 0; i < TEST_NUM_ITERS; ++i) {
        e_address address = (e_address)i;
        e_data data = (e_data) 1000 - i;

        // reference
        A[i] = data;

        // simple consistency check
        write_word(address, data);
        B[i] = read_word(address);
    }

    return vec_equals(A, B);
}

int test_1() {
    printf("--------Begin %s()--------\n", __func__);
    // initialize the test results vectors
    memset(A, 0, sizeof(e_data)*RESULT_VEC_LEN);
    memset(B, 0, sizeof(e_data)*RESULT_VEC_LEN);

    // Use the first four addresses as a "scratch area,"
    // and ensure that the rest of RRAM remains consistent.

    for (e_uint i = 0; i < 16; ++i) {
        write_word(i, i);
        A[i] = i;   // reference
    }

    for (e_uint i = 0; i < 4; ++i) {
        write_word(i, 99);
        A[i] = 99;
    }

    for (e_uint i = 0; i < 16; ++i) {
        B[i] = read_word(i);
    }

    return vec_equals(A, B);
}

/*
 * Tests remap().
 */
int test_2() {
    printf("--------Begin %s()--------\n", __func__);
    // initialize the test results vectors
    memset(A, 0, sizeof(e_data)*RESULT_VEC_LEN);
    memset(B, 0, sizeof(e_data)*RESULT_VEC_LEN);

    // Use the first four addresses as a "scratch area,"
    // and ensure that the rest of RRAM remains consistent.

    for (e_uint i = 0; i < 16; ++i) {
        write_word(i, i);
        A[i] = i;   // reference
    }

    remap();

    for (e_uint i = 0; i < 16; ++i) {
        B[i] = read_word(i);
    }

    return vec_equals(A, B);
}

/*
 * Tests multiple remap()s.
 */
int test_3() {
    printf("--------Begin %s()--------\n", __func__);
    // initialize the test results vectors
    memset(A, 0, sizeof(e_data)*RESULT_VEC_LEN);
    memset(B, 0, sizeof(e_data)*RESULT_VEC_LEN);

    // Use the first four addresses as a "scratch area,"
    // and ensure that the rest of RRAM remains consistent.

    for (e_uint i = 0; i < 16; ++i) {
        write_word(i, i);
        A[i] = i;   // reference
    }

    remap();
    remap();
    remap();
    remap();
    remap();

    for (e_uint i = 0; i < 16; ++i) {
        B[i] = read_word(i);
    }

    return vec_equals(A, B);
}

int main(int argc, char *argv[]) {
    printf("Hello from ENDUReR.\n");


    initialize(12345);
    assert(!test_0());
    teardown();

    initialize(12346);
    assert(!test_1());
    teardown();

    initialize(12347);
    assert(!test_2());
    teardown();

    initialize(12348);
    assert(!test_3());
    teardown();

    printf("All results vectors match.\n");
    printf("Done.\n");

    return 0;
}
