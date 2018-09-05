# ENDUReR

ENDUrance Resiliency by random Remapping

Basic cache coherency and read/write remap mechanism to support RRAM main memory with an SRAM cache.

Implements a fully-associative, coherent, write-back cache.

## Notes
* Remapping must be triggered manually with a call to `remap()`; i.e., it doesn't run on a schedule.
* In the Linux dev environment, all types are 64-bit, though this can be changed in endurer.h.
* When the SRAM cache runs out of space, all dirty words are written back to RRAM and the cache is flushed.
  - A word which was only ever read into the cache, and never written to, needn't be written back.
    + See sram_map
