/*
 * Use static buffers as arenas of memory to act as RRAM and SRAM.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unordered_map>

#include "endurer.h"

/*
 * Defines the state of the word in the SRAM cache.
 */
typedef enum {
    INVALID, // has value 0; we'll memset to this to clear it
    SYNC,
    DIRTY
} word_state;

static e_uint relative_shift = 0;

/*
 * This is the pure-C replacement for the sram_map. It maps e_addresses
 * to tuples of (index in SRAM cache, word state).
 *
 * Note that it may be a sparse map, and requires M slots.
 */
typedef struct {
    e_address index;    // index, if present in the SRAM cache
    word_state state;   // state (will be 0, i.e. INVALID, if not present)
} sram_map_entry;

static sram_map_entry sram_map[M];
static e_uint sram_map_size = 0;

static inline void sram_map_clear() {
    memset(sram_map, 0, M * sizeof(sram_map_entry));
    sram_map_size = 0;
}













/* Maps address (e_address) to an index within SEG_SRAM. */
//static std::unordered_map<e_address, std::pair<e_uint, word_state>> sram_map;

/*
 * On the 64-bit Linux dev environment, this will (need to) be a larger address.
 * But, on the board, we want the segment to be addressable in 16 bits, so we'll
 * try to map it to a low address.
 */
static e_data SEG_RRAM[M];
static e_data SEG_SRAM[S];

/*
 * NOTE: on the board itself, exercise care that segment_size doesn't exceed the
 * amount of available memory.
 */
// TODO use rand_r() or a better PRNG
void initialize(const e_uint seed) {
    if (seed != 0) srand(seed);
    else srand(time(NULL));
}

/*
 * Internal functions to map virtual (user-space) addresses to
 * physical (RRAM segment) addresses. We need to do a virtual-
 * to-physical translation since physical addresses will change
 * when shifting occurs.
 */
//TODO note + and - switch for FWD-shift loop vs BWD-shift loop
//(just follow the direction of the shift. data goes back, address goes back)
static inline e_address physical_to_virtual(e_address phys) {
    return (phys + relative_shift) % M;
}

/*
 * Note that, to deal with the fact that (virt - relative_shift) may underflow,
 * we add an extra M before subtracting (and it works because (M + x) % M = x).
 */
static inline e_address virtual_to_physical(e_address virt) {
    return (virt + M - relative_shift) % M;
}

void write_back() {
    for (e_address address = 0; address < M; ++address) { // iterate through the whole sram_map table
        e_uint idx = sram_map[address].index;
        word_state state = sram_map[address].state;

        if (state == DIRTY) {
            SEG_RRAM[address] = SEG_SRAM[idx];
        }
    }
}

/* Flushes the SRAM cache, writing back only those words with DIRTY word_state in the cache. */
void flush_and_write_back() {
    //printf("Flushing and writing back dirty words...\n");
    write_back();
    sram_map_clear();
}

/*
 * Performs a random-shift remapping.
 * Updates relative_shift so that virtual-physical translation can work.
 */
void remap() {
    e_uint shift = rand() % (M-1);
    relative_shift = (relative_shift + shift) % M; // for virtual-physical mapping
    e_uint num_shift_loops = 1 << __builtin_ctz(shift);
    e_uint shifts_per_loop = M / num_shift_loops; // requires power-of-two-sized segment

    /* Not strictly necessary, but simplifies things. */
    flush_and_write_back();

    for (e_uint i = 0; i < num_shift_loops; ++i) {
        e_data tmp = SEG_RRAM[i];   // cache the first element before clobbering it
        e_uint dist = 0;
        for (e_uint j = 0; j < shifts_per_loop - 1; ++j) {
            e_address curr_addr = (i + dist) % M;
            e_address next_addr = (curr_addr + shift) % M;

            SEG_RRAM[curr_addr] = SEG_RRAM[next_addr];
            dist += shift;
        }

        e_address last_addr = (i + dist) % M;
        assert(last_addr < M);

        SEG_RRAM[last_addr] = tmp;  // now, put cached first element where it belongs
    }
}

e_data read_word(const e_address virt) {
    e_address address = virtual_to_physical(virt);
    printf("read_word at virtual [%x] (physical [%x])\n", virt, address);
    assert(address < M);

    if (sram_map[address].state != INVALID) {
        // cache hit
        e_uint idx = sram_map[address].index;
        word_state state = sram_map[address].state;
        
        return SEG_SRAM[idx];
    }
    else {
        // cache miss. go to RRAM, fault value into SRAM, return
        e_data value = SEG_RRAM[address];

        // Check if cache is full. If so, flush, write back, and
        // and fault the single new word into the cache.
        if (sram_map_size == S) {
            flush_and_write_back();
        }
        SEG_SRAM[sram_map_size] = value;
        sram_map[address].index = sram_map_size; sram_map[address].state = SYNC; ++sram_map_size;
        return value;
    }
}

/*
 * Make sure you explicitly manage the SRAM cache.
 */
e_uint write_word(const e_address virt, const e_data data) {
    e_address address = virtual_to_physical(virt);
    //printf("write_word %x at virtual [%x] (physical [%x])\n", data, virt, address);
    assert(address < M);

    // If it's in the cache already, update it.
    // If it's not, then put it in
    //      1.) check if full, flush-and-wb if so
    //      2.) if not full, just put in cache and indicate maybe-dirty
    if (sram_map[address].state != INVALID) {
        e_uint idx = sram_map[address].index;
        SEG_SRAM[idx] = data;
        sram_map[address].index = idx; sram_map[address].state = DIRTY;
    }
    else {
        if (sram_map_size == S) {
            flush_and_write_back();
        }
        SEG_SRAM[sram_map_size] = data;
        sram_map[address].index = sram_map_size; sram_map[address].state = DIRTY; ++sram_map_size;
    }

    return data;
}

e_uint teardown() {
    sram_map_clear();

    // zero buffers for good measure
    memset(SEG_SRAM, 0, S * sizeof(e_data));
    memset(SEG_RRAM, 0, M * sizeof(e_data));
    return 0;
}
