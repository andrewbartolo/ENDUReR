/*
 * ENDUReR RRAM endurance mitigation mechanism, embedded implementation.
 *
 * This file defines the write_word() and read_word() methods, which are interfaces
 * into the RRAM+SRAM memory area.
 *
 * Symbols for segment size, remapping period, etc. are consistent with the paper.
 */

/*
 * Here are some typedefs to make things work in the 64-bit Linux development
 * environment. These can be switched to 16-bit for the board itself.
 *
 * (Note that <stdint.> should *NOT* be included.)
 */
typedef unsigned long long e_address;
typedef unsigned long long e_data;
typedef unsigned long long e_uint;

//#define M (16384/sizeof(e_data))
//#define W (M/4) // TODO

#define M 16
#define W 4

void *initialize(const e_uint _segment_size);
e_data read_word(const e_address address);
e_uint write_word(const e_address address, const e_data data);
e_uint teardown();

/*
 * Getters and setters for the internal state of the ENDUReR mechanism.
 */
e_address get_segment_base();
e_uint get_segment_size();
