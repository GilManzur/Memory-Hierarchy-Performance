#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configuration.h"

typedef struct CacheLine {
    unsigned long data;
    int lastUsed;  // A timestamp or counter to track LRU
} CacheLine;

typedef struct Cache {
    CacheLine *lines;
    int size;
    int currentTime;
    int hits;
    int misses;
} Cache;

Cache* createCache(int size) {
    Cache* cache = (Cache*)malloc(sizeof(Cache));
    cache->lines = (CacheLine*)malloc(size * sizeof(CacheLine));
    cache->size = size;
    cache->currentTime = 0;
    cache->hits = 0;
    cache->misses = 0;
    for (int i = 0; i < size; i++) {
        cache->lines[i].data = -1;  // -1 indicates an empty slot
        cache->lines[i].lastUsed = 0;
    }
    return cache;
}

int findLRU(Cache* cache) {
    int lruIndex = 0;
    for (int i = 1; i < cache->size; i++) {
        if (cache->lines[i].lastUsed < cache->lines[lruIndex].lastUsed) {
            lruIndex = i;
        }
    }
    return lruIndex;
}

void accessCache(Cache* cache, unsigned long data) {
    cache->currentTime++;
    for (int i = 0; i < cache->size; i++) {
        if (cache->lines[i].data == data) {
            // Cache hit
            cache->lines[i].lastUsed = cache->currentTime;
            cache->hits++;
            return;
        }
    }
    // Cache miss
    int lruIndex = findLRU(cache);
    cache->lines[lruIndex].data = data;
    cache->lines[lruIndex].lastUsed = cache->currentTime;
    cache->misses++;
}

void freeCache(Cache* cache) {
    free(cache->lines);
    free(cache);
}

void processTraceFile(const char* filename, Cache* l1Cache, Cache* l2Cache, Cache* l3Cache) {
    FILE *file = fopen(filename,"r");
    if (file == NULL) {
        fprintf(stderr, "Could not open trace file: %s\n", filename);
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        unsigned long address;
        char operation[3];
        sscanf(line, "0x%lx %s", &address, operation);

        if (strcmp(operation, "sw") == 0 || strcmp(operation, "lw") == 0) {
            // For simplicity, we use the address as the data
            accessCache(l1Cache, address);
            accessCache(l2Cache, address);
            accessCache(l3Cache, address);
        }
    }

    fclose(file);
}

void printStatistics(Cache* cache, const char* cacheName) {
    int totalAccesses = cache->hits + cache->misses;
    double hitRate = ((double)cache->hits / totalAccesses) * 100;
    double missRate = ((double)cache->misses / totalAccesses) * 100;

    printf("%s Cache Statistics:\n", cacheName);
    printf("Total Accesses: %d\n", totalAccesses);
    printf("Hits: %d\n", cache->hits);
    printf("Misses: %d\n", cache->misses);
    printf("Hit Rate: %.2f%%\n", hitRate);
    printf("Miss Rate: %.2f%%\n", missRate);
}

double calculateAMAT(int l1Hits, int l1Misses, int l2Hits, int l2Misses, int l3Hits, int l3Misses) {
    int l1Accesses = l1Hits + l1Misses;
    int l2Accesses = l2Hits + l2Misses;
    int l3Accesses = l3Hits + l3Misses;
    int totalAccesses = l1Accesses + l2Accesses + l3Accesses;

    double l1HitRate = (double)l1Hits / l1Accesses;
    double l1MissRate = 1 - l1HitRate;
    double l2HitRate = (double)l2Hits / l2Accesses;
    double l2MissRate = 1 - l2HitRate;
    double l3HitRate = (double)l3Hits / l3Accesses;
    double l3MissRate = 1 - l3HitRate;

    double amat = L1_HIT_TIME + l1MissRate * (L2_HIT_TIME + l2MissRate * (L3_HIT_TIME + l3MissRate * (RAS_TIME + CAS_TIME)));
    return amat;
}

int main() {
    int l1Sizes[] = {128 * 1024, 64 * 1024, 32 * 1024, 16 * 1024};
    const char* traceFile = "C:\\Users\\GilMa\\PycharmProjects\\Memory-Hierarchy-Performance\\tracefile.txt";  // Replace with your actual trace file path

    for (int i = 0; i < sizeof(l1Sizes) / sizeof(l1Sizes[0]); i++) {
        int l1Size = l1Sizes[i];
        Cache* l1Cache = createCache(l1Size);
        Cache* l2Cache = createCache(L2_CACHE_SIZE);
        Cache* l3Cache = createCache(L3_CACHE_SIZE);
        
        processTraceFile(traceFile, l1Cache, l2Cache, l3Cache);

        printStatistics(l1Cache, "L1");
        printStatistics(l2Cache, "L2");
        printStatistics(l3Cache, "L3");

        double amat = calculateAMAT(l1Cache->hits, l1Cache->misses, l2Cache->hits, l2Cache->misses, l3Cache->hits, l3Cache->misses);
        printf("Average Memory Access Time (AMAT) for L1 size %d: %.2f cycles\n", l1Size, amat);

        freeCache(l1Cache);
        freeCache(l2Cache);
        freeCache(l3Cache);
    }

    return 0;
}
