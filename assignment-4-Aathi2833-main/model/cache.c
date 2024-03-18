#include "cache.h"
#include "dogfault.h"
#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// DO NOT MODIFY THIS FILE. INVOKE AFTER EACH ACCESS FROM runTrace
void print_result(result r) {
  if (r.status == CACHE_EVICT)
    printf(" [status: %d victim_block: 0x%llx insert_block: 0x%llx]", r.status,
           r.victim_block, r.insert_block);
  if (r.status == CACHE_HIT)
    printf(" [status: %d]", r.status);
  if (r.status == CACHE_MISS)
    printf(" [status: %d insert_block: 0x%llx]", r.status, r.insert_block);
}

// HELPER FUNCTIONS USEFUL FOR IMPLEMENTING THE CACHE
// Convert address to block address. 0s out the bottom block bits.
unsigned long long address_to_block(const unsigned long long address, const Cache *cache) {
    unsigned long long block_mask = ((unsigned long long)1 << cache->blockBits) - 1;
    return address & ~block_mask;
}

// Access the cache after successful probing.
void access_cache(const unsigned long long address, const Cache *cache) {
  return;
}

// Calculate the tag of the address. 0s out the bottom set bits and the bottom block bits.
unsigned long long cache_tag(const unsigned long long address,
                             const Cache *cache) {
    // Calculate the number of bits for the block and set indices
    int block_set_bits = cache->blockBits + cache->setBits;
    
    // Create a mask to zero out the bottom block and set bits
    unsigned long long mask = ~(0ULL) << block_set_bits;
    
    // Apply the mask to the address to get the tag
    unsigned long long tag = address & mask;

    return tag;
}

// Calculate the set of the address. 0s out the bottom block bits, 0s out the tag bits, and then shift the set bits to the right.
unsigned long long cache_set(const unsigned long long address, const Cache *cache) {
    unsigned long long block_mask = ((unsigned long long)1 << cache->blockBits) - 1;
    unsigned long long set_mask = ((unsigned long long)1 << cache->setBits) - 1;
    return (address & ~block_mask) >> cache->blockBits & set_mask;
}

// Check if the address is found in the cache. If so, return true. else return false.
bool probe_cache(const unsigned long long address, const Cache *cache) {
    unsigned long long set_index = cache_set(address, cache);
    unsigned long long tag = cache_tag(address, cache);
    for (int i = 0; i < cache->linesPerSet; i++) {
        if (cache->sets[set_index].lines[i].valid && cache->sets[set_index].lines[i].tag == tag) {
            return true; // Cache hit
        }
    }
    return false; // Cache miss
}

// Allocate an entry for the address. If the cache is full, evict an entry to create space. This method will not fail. When method runs there should have already been space created. 
void allocate_cache(const unsigned long long address, Cache *cache) {
    unsigned long long set_index = cache_set(address, cache);
    unsigned long long tag = cache_tag(address, cache);
    for (int i = 0; i < cache->linesPerSet; i++) {
        if (!cache->sets[set_index].lines[i].valid) {
            // Cache line is empty, insert block
            cache->sets[set_index].lines[i].valid = 1;
            cache->sets[set_index].lines[i].tag = tag;
            return;
        }
    }
    // No empty line found, evict and insert block
    int victim_line = victim_cache(address, cache);
    cache->sets[set_index].lines[victim_line].tag = tag;
}

// Is there space available in the set corresponding to the address?
bool avail_cache(const unsigned long long address, const Cache *cache) {
    unsigned long long set_index = cache_set(address, cache);
    for (int i = 0; i < cache->linesPerSet; i++) {
        if (!cache->sets[set_index].lines[i].valid) {
            return true; // Empty way available
        }
    }
    return false; // No empty way available
}

// If the cache is full, evict an entry to create space. This method figures out which entry to evict. Depends on the policy.
unsigned long long victim_cache(const unsigned long long address, Cache *cache) {
    unsigned long long set_index = cache_set(address, cache);
    int min_r_rate = cache->sets[set_index].lines[0].r_rate;
    int victim_index = 0;
    for (int i = 1; i < cache->linesPerSet; i++) {
        if (cache->sets[set_index].lines[i].r_rate < min_r_rate) {
            min_r_rate = cache->sets[set_index].lines[i].r_rate;
            victim_index = i;
        }
    }
    return victim_index; // Return the way of the block (corresponding line index within the set)
}

// Set can be determined by the address. Way is determined by policy and set by the operate cache. 
void evict_cache(const unsigned long long address, int way, Cache *cache) {
    unsigned long long set_index = cache_set(address, cache);
    cache->sets[set_index].lines[way].valid = 0;
}


// Given a block address, find it in the cache and when found remove it.
// If not found don't remove it. Useful when implementing 2-level policies. 
// and triggering evictions from other caches. 
void flush_cache(const unsigned long long block_address, Cache *cache) {
    unsigned long long set_index;
    unsigned long long tag;
    
    // Iterate over all sets to find the block address
    for (int i = 0; i < (1 << cache->setBits); i++) {
        set_index = i;
        for (int j = 0; j < cache->linesPerSet; j++) {
            tag = cache->sets[set_index].lines[j].tag;
            unsigned long long address = (tag << (cache->setBits + cache->blockBits)) | (set_index << cache->blockBits);
            if (address == block_address) {
                // Found the block address, invalidate it
                cache->sets[set_index].lines[j].valid = 0;
                return;
            }
        }
    }
}
// checks if the address is in the cache, if not and if the cache is full
// evicts an address
result operateCache(const unsigned long long address, Cache *cache) {
    result r;
    if (probe_cache(address, cache)) {
        r.status = CACHE_HIT;
        cache->hit_count++;
    } else {
        r.status = CACHE_MISS;
        cache->miss_count++;
        if (!avail_cache(address, cache)) {
            r.status = CACHE_EVICT;
            cache->eviction_count++;
            unsigned long long victim_way = victim_cache(address, cache);
            evict_cache(address, victim_way, cache);
        }
        allocate_cache(address, cache);
    }
    return r;
}

// initialize the cache and allocate space for it
void cacheSetUp(Cache *cache, char *name) {
    cache->name = name;
    cache->sets = (Set*)malloc((1 << cache->setBits) * sizeof(Set));
    for (int i = 0; i < (1 << cache->setBits); i++) {
        cache->sets[i].lines = (Line*)malloc(cache->linesPerSet * sizeof(Line));
        for (int j = 0; j < cache->linesPerSet; j++) {
            cache->sets[i].lines[j].valid = 0;
            // Initialize other metadata if needed
        }
    }
  cache->hit_count = 0;
  cache->miss_count = 0;
  cache->eviction_count = 0;
}

// deallocate memory
void deallocate(Cache *cache) {
}

void printSummary(const Cache *cache) {
  printf("\n%s hits:%d misses:%d evictions:%d", cache->name, cache->hit_count,
         cache->miss_count, cache->eviction_count);
}
