#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configuration.h"
#include <math.h>

unsigned int search_address_by_name(Address addressList[], const char* name);
void change_address_by_name(Address addressList[], const char* name, unsigned int val);
Cache* init_cache(size_t size, int access);
uint32_t getTAG(Cache* cache, uint32_t address) { return address >> (cache->index_bits + cache->offset_bits); }
uint32_t getINDEX(Cache* cache, uint32_t address);
void free_cache(Cache* cache);
int check_access_cache(Cache* cache, uint32_t address, int is_write);
void access_cache(Cache* cache, uint32_t address,bool dirty);
void programProccess(const char* filename, Cache* L1, Cache* L2, Cache* L3);



void main() {
    Cache* L1 = init_cache(L1_CACHE_SIZE ,L1_HIT_TIME);
    Cache* L2 = init_cache(L2_CACHE_SIZE ,L2_HIT_TIME);
    Cache* L3 = init_cache(L2_CACHE_SIZE ,L3_HIT_TIME);

    programProccess("C:\\Users\\GilMa\\PycharmProjects\\Memory-Hierarchy-Performance\\fibonacci_val_filtered.trc" , L1, L2, L3);


}


// Function to search for the address by name
unsigned int search_address_by_name(Address addressList[], const char* name) {
    for (int i = 0; i < NUM_ADDRESSES; i++) {
        if (strcmp(addressList[i].name, name) == 0) {
            return addressList[i].address;
        }
    }
    // Return a special value (e.g., 0xFFFFFFFF) if the name is not found
    printf("search_address_by_name() adrees not found"); // error incase of not found
    return 0xFFFFFFFF;
}

void change_address_by_name(Address addressList[], const char* name, unsigned int val) {
    for (int i = 0; i < NUM_ADDRESSES; i++) {
        if (strcmp(addressList[i].name, name) == 0) {
            addressList[i].address = val;
            return;
        }
    }
    printf("change_address_by_name adress not found");
    return;
}

//initilize the cash size and lines number
Cache* init_cache(size_t size, int access) {
    Cache* cache = (Cache*)malloc(sizeof(Cache));
    if (!cache) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    cache->size = size;
    cache->num_lines = size / CACHE_LINE_SIZE;
    cache->access = access;
    cache->offset_bits = (int)log2(CACHE_LINE_SIZE);
    cache->index_bits = (int)log2(cache->num_lines);
    cache->tag_bits = 32 - cache->index_bits - cache->offset_bits;
    cache->hits = 0;
    cache->miss = 0;

    cache->lines = (CacheLine*)malloc(sizeof(CacheLine) * cache->num_lines);
    if (cache->lines == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }

    for (size_t i = 0; i < cache->num_lines; i++) {
        cache->lines[i].tag = 0;
        cache->lines[i].valid = false;
        cache->lines[i].dirty = false;
    }

    return cache;
}

void free_cache(Cache* cache) {
    free(cache->lines);
    free(cache);
}

uint32_t getINDEX(Cache* cache, uint32_t address)
{
    uint32_t INDEX = address << cache->tag_bits;
    INDEX = INDEX >> (cache->tag_bits + cache->offset_bits);
    return INDEX;
}

int check_access_cache(Cache* cache, uint32_t address, int is_write) {

    uint32_t writeback_TAG = 0, writeback_INDEX = 0, writeback_OFFSET = 0;
    CacheLine* writeback_LINE;

    uint32_t TAG = getTAG(cache, address);
    uint32_t INDEX = getINDEX(cache, address);
    //uint32_t OFFSET = getOFFSET(cache, address);

    /*uint64_t index = (address / CACHE_LINE_SIZE) % cache->num_lines;
      uint64_t tag = address / cache->size;*/
    CacheLine* line = &cache->lines[INDEX];



    // CACHE MISS LOGIC
    if (!line->valid) {
        // case the index is empty
        line->valid = true;
        line->tag = TAG;
        cache->miss += 1;
        return -2;
    }

    if (line->valid && line->tag != TAG && line->dirty) {
        // line is dirty and need to save it in the lower level
        cache->miss += 1;
        return 0;
    }

    if (line->valid && line->tag != TAG) {
        // case is valid and the tag is wrong miss and go to lower level
        cache->miss += 1;
        return -1;
    }


    if (line->valid && line->tag == TAG ) {
        // case the index is valid and is the correct adress and "SW"
        line->dirty = true;
        cache->hits += 1;
        return 1;
    }



    //// CACHE MISS LOGIC
    //// WRITE BACK

    //if (line->valid && line->dirty && line->tag != TAG && is_write == 1) { //if .its./ a miss, but the line is valid and dirty ( there is some data, //butweneed .to .overwrite it with another data... )
    //    // Write back if dirty
    //    // This is simplified; in a real system, we would write //theblockto .the .next level cache or DRAM

    //    if (level == 1)
    //    {
    //        uint32_t x = reconstructAddress(line->tag, //INDEX,L1>index_bits, .L1-.>offset_bits);
    //        printf("restored address is: 0x%08x", x);

    //        writeback_INDEX = getINDEX(L2, x);

    //        writeback_LINE = &L2->lines[writeback_INDEX];

    //        writeback_LINE->tag = line->tag;

    //    }

    //    if (level == 2)
    //    {

    //        writeback_INDEX = getINDEX(L3, reconstructAddress//(line>tag,INDEX, .L2-.>index_bits, L2->offset_bits));

    //        writeback_LINE = &L3->lines[writeback_INDEX];

    //        writeback_LINE->tag = line->tag;

    //    }
    //}


    //line->valid = 1;
    //line->tag = TAG;
    //line->dirty = is_write;

    //return -1; // Indicating a miss


}

void access_cache(Cache* cache, uint32_t address,bool dirty) {
    uint32_t TAG = getTAG(cache, address);
    uint32_t INDEX = getINDEX(cache, address);
    cache->lines[INDEX].valid = true;
    cache->lines[INDEX].tag = TAG;
    cache->lines[INDEX].dirty = dirty;
}

void programProccess(const char* filename, Cache* L1, Cache* L2, Cache* L3) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(1);
    }
    uint32_t address;
    char line[256];
    char instruction[10];
    char var1[10], var2[10], var3[10];
    unsigned int num1, num2;

    while (fgets(line, sizeof(line), file)) {
        // Check for load or store instructions

        if (strstr(line, "->")) { // if the line has -> in it, then update the values of each register
            sscanf(line, "%s %x -> %x", var1, &num1, &num2);

            change_address_by_name(addressList, var1, num2); //there the registers values (Address) stored.
            continue;
        }


        sscanf(line, "%s %[^,],%d(%[^)])", instruction, var1, &num1, var2);
        //printf("Parsed lw/sw line: instruction=%s, var1=%s, num1=%d, var2=%s\n", instruction, var1, num1, var2);

        int isWrite = 0;
        if (strstr(line, "lw")) {
            isWrite = 0;
        }
        else if (strstr(line, "sw")) {
            isWrite = 1;
        }
        else {
            continue;
        }

        address = search_address_by_name(addressList, var2) + num1;
        int L1_state;
        int L2_state;
        int L3_state;
        uint32_t dirtyAddress = 0;
        // Simulate cache access
        L1_state = check_access_cache(L1, address, isWrite);
        switch (L1_state) {
        case  -2: //evry thing is empty L1 miss
            // pay DRAM
            access_cache(L2, address, false);
            L2->miss += 1;
            access_cache(L3, address, false);
            L3->miss += 1;
            access_cache(L1, address, isWrite);
            break;
        case -1: //L1 miss
            L2_state = check_access_cache(L2, address, isWrite);
            switch (L2_state) {
            case -2: // L2 miss and line is empty
                //pay DRAM
                L3->miss += 1;
                access_cache(L1, address, isWrite);
                access_cache(L2, address, false);
                access_cache(L3, address, false);
            case -1: //miss check in L3
                L3_state = check_access_cache(L3, address, isWrite);
                switch (L3_state) {
                case -2: // L3 miss line is empty
                    //pay DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                case -1: // every cache got miss
                    //pay DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                case 0: //every cache got miss and L3 line is dirty
                    //pay 2 DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                case 1: //line found got hit
                    // NO PAY
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                }
            case 0: // L2 got miss and the line is dirty
                L3_state = check_access_cache(L3, address, isWrite);
                dirtyAddress = (L2->lines[getINDEX(L2, address)].tag << (L2->offset_bits + L2->index_bits)) | (getINDEX(L2, address) << L2->offset_bits);
                access_cache(L3, dirtyAddress, true);
                dirtyAddress = 0;
                switch (L3_state) {
                case -2: // L3 line is empty
                    //pay DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                case -1: // // every cache got miss
                    //pay DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                case 0: //L3 line is the same as L2
                    //pay 2 DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                case 1: // L3 hit
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                }
            case 1: // L2 hit
                access_cache(L1, address, isWrite);
                access_cache(L2, address, false);
                break;
            }
            break;
        case 0: // L1 line is dirty
            dirtyAddress = (L1->lines[getINDEX(L1, address)].tag << (L1->offset_bits + L1->index_bits)) | (getINDEX(L1, address) << L1->offset_bits);
            access_cache(L2, dirtyAddress, true);
            L2_state = check_access_cache(L2, address, isWrite);
            switch (L2_state) {
            case -2: // L2 miss and line is empty
                //pay DRAM
                L3->miss += 1;
                access_cache(L1, address, isWrite);
                access_cache(L2, address, false);
                access_cache(L3, address, false);
            case -1: //miss check in L3
                L3_state = check_access_cache(L3, address, isWrite);
                switch (L3_state) {
                case -2: // L3 miss line is empty
                    //pay DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                case -1: // every cache got miss
                    //pay DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                case 0: //every cache got miss and L3 line is dirty
                    //pay 2 DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                case 1: //line found got hit
                    // NO PAY
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                }
            case 0: // L2 got miss and the line is dirty
                L3_state = check_access_cache(L3, address, isWrite);
                dirtyAddress = (L2->lines[getINDEX(L2, address)].tag << (L2->offset_bits + L2->index_bits)) | (getINDEX(L2, address) << L2->offset_bits);
                access_cache(L3, dirtyAddress, true);
                dirtyAddress = 0;
                switch (L3_state) {
                case -2: // L3 line is empty
                    //pay DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                case -1: // // every cache got miss
                    //pay DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                case 0: //L3 line is the same as L2
                    //pay 2 DRAM
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                case 1: // L3 hit
                    access_cache(L1, address, isWrite);
                    access_cache(L2, address, false);
                    access_cache(L3, address, false);
                    break;
                }
            case 1: // L2 hit
                access_cache(L1, address, isWrite);
                break;
            }
            break;
        case 1: // L1 hit
            access_cache(L1, address, isWrite);
            break;
        }
    }

    fclose(file);

    printf("Total memory accesses: %d\n", L1->miss + L1->hits + L2->miss + L2->hits + L3->miss + L3->hits);
    printf("L1 cache hits: %d\n", L1->hits);
    printf("L2 cache hits: %d\n", L2->hits);
    printf("L3 cache hits: %d\n", L3->hits);

    printf("L1 cache misses: %d\n", L1->miss);
    printf("L2 cache misses: %d\n", L2->miss);
    printf("L3 cache misses: %d\n", L3->miss);

    //printf("Average memory access time: %.2f\n", (float)(total_time + write_back_cost) /(L1_hits + L2_hits + L3_hits + L3_misses + L1_misses + L2_misses));


    free_cache(L1);
    free_cache(L2);
    free_cache(L3);

}
