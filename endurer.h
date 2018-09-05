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

#define M 16
#define S 4
#define TW 4

//#define M 524288
//#define S 8192

void initialize(const e_uint _segment_size);
void write_back();
void remap();
e_data read_word(const e_address address);
e_uint write_word(const e_address address, const e_data data);
e_uint teardown();

/*
 * Getters and setters for the internal state of the ENDUReR mechanism.
 */
e_address get_segment_base();
e_uint get_segment_size();

/*
 * Only exposing these for testing purposes.
 */
static inline e_address physical_to_virtual(e_address phys);
static inline e_address virtual_to_physical(e_address virt);
