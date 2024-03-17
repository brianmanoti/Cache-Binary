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
unsigned long long address_to_block(const unsigned long long address,
                                const Cache *cache) {
}

// Access the cache after successful probing.
void access_cache(const unsigned long long address, const Cache *cache) {
  return;
}

// Calculate the tag of the address. 0s out the bottom set bits and the bottom block bits.
unsigned long long cache_tag(const unsigned long long address,
                             const Cache *cache) {
  unsigned long long tag;
  return tag;
}

// Calculate the set of the address. 0s out the bottom block bits, 0s out the tag bits, and then shift the set bits to the right.
unsigned long long cache_set(const unsigned long long address,
                             const Cache *cache) {
  unsigned long long set;
  return set;
}

// Check if the address is found in the cache. If so, return true. else return false.
bool probe_cache(const unsigned long long address, const Cache *cache) {
}

// Allocate an entry for the address. If the cache is full, evict an entry to create space. This method will not fail. When method runs there should have already been space created. 
void allocate_cache(const unsigned long long address, const Cache *cache) {
}

// Is there space available in the set corresponding to the address?
bool avail_cache(const unsigned long long address, const Cache *cache) {
  // calculate tag and set values
  return true;
}

// If the cache is full, evict an entry to create space. This method figures out which entry to evict. Depends on the policy.
unsigned long long victim_cache(const unsigned long long address,
                                Cache *cache) {
  return 0;
}

// Set can be determined by the address. Way is determined by policy and set by the operate cache. 
void evict_cache(const unsigned long long address, int way, Cache *cache) {
  return;
}

// Given a block address, find it in the cache and when found remove it.
// If not found don't remove it. Useful when implementing 2-level policies. 
// and triggering evictions from other caches. 
void flush_cache(const unsigned long long block_address, Cache *cache) {
  return;
}
// checks if the address is in the cache, if not and if the cache is full
// evicts an address
result operateCache(const unsigned long long address, Cache *cache) {
  // checkCache checks if the address is in the cache
  result r;
  // Hit
  // Miss
  // Evict
  return r;
}

// initialize the cache and allocate space for it
void cacheSetUp(Cache *cache, char *name) {
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
