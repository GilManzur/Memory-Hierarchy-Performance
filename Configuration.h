#ifdef CONFIGURATION_H
#define CONFIGURATION_H

// Cache parameters
#define L1_CACHE_SIZE 16384      // 16KB
#define L2_CACHE_SIZE 524288     // 512KB
#define L3_CACHE_SIZE 2097152    // 2MB

#define L1_ASSOCIATIVITY 1       // Direct mapped
#define L2_ASSOCIATIVITY 1       // Direct mapped
#define L3_ASSOCIATIVITY 1       // Direct mapped

#define L1_HIT_TIME 1            // 1 cycle
#define L2_HIT_TIME 6            // 6 cycles
#define L3_HIT_TIME 30           // 30 cycles

#define WRITE_ALLOCATE 1         // Write allocate
#define WRITE_BACK 1             // Write back

// Replacement policy
#define REPLACEMENT_POLICY_LRU 1 // LRU

// DRAM parameters
#define BUS_WIDTH 4              // 4 bytes
#define CHANNELS 1               // 1 channel
#define DIMMS 1                  // 1 DIMM
#define BANKS 4                  // 4 banks
#define RAS_TIME 100             // 100 cycles
#define CAS_TIME 50              // 50 cycles

// add more parameters here if needed


#endif // CONFIGURATION_H