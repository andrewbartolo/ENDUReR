/*
 * Use mmap() to allocate a big arena of memory that we can use to act as the RRAM+SRAM memory area.
 * (mmap() instead of malloc() so we can map onto the device itself more easily in the future.
 */
#include "endurer.h"
#include <assert.h>
#include <sys/mman.h>

// TODO DEBUG REMOVE
#include <stdio.h>
#include <unordered_map>

/*
 * Defines the state of the word in the SRAM cache.
 */
typedef enum {
    SYNC,
    DIRTY//,
    //INVALID   // currently unused (just check if present in the map instead)
} word_state;

static void *segment_base = NULL;
static e_uint segment_size = 0;

/* Maps address (e_address) to an index within SEG_SRAM. */
static std::unordered_map<e_address, std::pair<e_uint, word_state>> sram_map;

/*
 * On the 64-bit Linux dev environment, this will (need to) be a larger address.
 * But, on the board, we want the segment to be addressable in 16 bits, so we'll
 * try to map it to a low address.
 */
static e_data SEG_RRAM[M];
static e_data SEG_SRAM[W];

/*
 * Calls mmap() to memory-map the region, then sets up the associative array, etc.
 *
 * NOTE: on the board itself, exercise care that segment_size doesn't exceed the
 * amount of available memory.
 */
void *initialize(const e_uint _segment_size) {
    /*
    segment_base = mmap(SEGMENT_MAP_HINT, _segment_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                         MAP_ANONYMOUS, -1, 0);
    */
    segment_base = &SEG_RRAM;

    printf("RRAM base: %x\n", segment_base);
    printf("SRAM base: %x\n", &SEG_SRAM);
    
    segment_size = W;

    return segment_base;
}

/* Flushes the SRAM cache, writing back only those words with DIRTY word_state in the cache. */
void flush_and_writeback() {
    for (const auto& pair : sram_map) {
        e_address address = pair.first;
        e_uint idx = pair.second.first;
        word_state status = pair.second.second;

        if (status == DIRTY) {
            SEG_RRAM[address] = SEG_SRAM[idx];
        }
    }

    sram_map.clear();
}

e_data read_word(const e_address address) {
    /* TODO bounds-check address? */
    printf("read_word at %x\n", address);

    if (!!sram_map.count(address)) {
        // cache hit
        e_uint idx = sram_map[address].first;
        word_state status = sram_map[address].second;
        
        return SEG_SRAM[idx];
    }
    else {
        // cache miss. go to RRAM, fault value into SRAM, return
        e_data value = SEG_RRAM[address];

        // Check if cache is full. If so, flush, write back, and
        // and fault the single new word into the cache.
        if (sram_map.size() == W) {
            flush_and_writeback();
            SEG_SRAM[0] = value;
            sram_map[address] = std::pair<e_uint, word_state>(0, SYNC);
        }
        return value;
    }
}

/*
 * Make sure you explicitly manage the SRAM cache.
 */
e_uint write_word(const e_address address, const e_data data) {
    printf("write_word %x at %x\n", data, address);

    // If it's in the cache already, update it.
    // If it's not, then put it in
    //      1.) check if full, flush-and-wb if so
    //      2.) if not full, just put in cache and indicate maybe-dirty
    if (!!sram_map.count(address)) {
        e_uint idx = sram_map[address].first;
        SEG_SRAM[idx] = data;
        sram_map[address] = std::pair<e_uint, word_state>(sram_map.size(), DIRTY); // TODO typecast map size to e_uint!
    }
    else {
        if (sram_map.size() == W) {
            flush_and_writeback();
            SEG_SRAM[0] = data;
            sram_map[address] = std::pair<e_uint, word_state>(0, SYNC);
        }
        else {
            // just update (and dirty) the appropriate cache entry
            SEG_SRAM[sram_map.size()] = data;
            sram_map[address] =  std::pair<e_uint, word_state>(sram_map.size(), DIRTY);
        }
    }
    

    return data;
}

e_uint teardown() {
    assert(munmap(segment_base, segment_size) == 0);

    return 0;
}
