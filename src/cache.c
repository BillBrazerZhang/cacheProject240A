//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"

//
// TODO:Student Information
//
const char *studentName = "Wenyu Zhang, Ge Chang";
const char *studentID   = "A53238371, A53240181";
const char *email       = "wez078@ucsd.edu, chg073@eng.ucsd.edu";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
struct cacheBlock {
  uint32_t tag;
  uint32_t address //address recorded
  uint32_t RU; //recently used
  bool dirty;
  bool valid;
};

struct cache {
  //parameters
  uint32_t blockSize; //size for one block
  uint32_t blockNum;  //total block number 
  uint32_t assocNum;  //associaitiivty of a cache
  uint32_t setNum;    //set number of a cache
  uint32_t hitTime;

  uint32_t indexBits;
  uint32_t offsetBits;
  uint32_t tagBits;

  struct cacheBlock *blocks;
  struct cache *victim;
  struct cache *nextLevel;
};

//
cache *iCache;
cache *dCache;  
cache *l2Cache; 
//------------------------------------//
//          Cache Functions           //
//------------------------------------//
// Initialize the blocks
void initBlocks(struct cache* memory, uint32_t blockNum) {
  for(uint32_t i = 0; i < blockNum; i++) {
    memory.blocks[i].tag = 0;
    memory.blocks[i].address = 0;
    memory.blocks[i].RU = memory.assocNum - 1;
    memory.blocks[i].dirty = FALSE;
    memory.blocks[i].valid = TRUE;
  }
}
// Initialize the Cache Hierarchy
//
void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;

  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;

  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;
  
  //
  //TODO: Initialize Cache Simulator Data Structures
  //
  //iCache
  iCache = (struct cache*)malloc(sizeof(cache));
  iCache.blockSize = blocksize;
  iCache.setNum = icacheSets;
  iCache.assocNum = icacheAssoc;
  iCache.hitTime = icacheHitTime;

  iCache.offsetBits = log2(iCache.blocksize);
  iCache.indexBits = log2(iCache.setNum);
  iCache.tagBits = MAX_BITS - iCache.offsetBits - iCache.indexBits;
  iCache.blockNum = iCache.setNum*iCache.assocNum;  

  iCache.blocks = (struct cacheBlock*)malloc(sizeof(struct cacheBlock)*iCache.setNum*iCache.assocNum);
  iCache.victim = NULL;
  iCache.nextLevel = l2Cache;

  initBlocks(iCache, iCache.blockNum);
  //iCache.victim = (struct cache*)malloc(sizeof(cache));
  //iCache.victim.blocks = (struct cacheBlock*)malloc(sizeof(struct cacheBlock));

  //dCache
  dCache = (struct cache*)malloc(sizeof(cache));
  dCache.blockSize = blocksize;
  dCache.setNum = dcacheSets;
  dCache.assocNum = dcacheAssoc;
  dCache.hitTime = dcacheHitTime;

  dCache.offsetBits = log2(dCache.blocksize);
  dCache.indexBits = log2(dCache.setNum);
  dCache.tagBits = MAX_BITS - dCache.offsetBits - dCache.indexBits;
  dCache.blockNum = dCache.setNum*dCache.assocNum;  
  
  dCache.blocks = (struct cacheBlock*)malloc(sizeof(struct cacheBlock)*dCache.setNum*dCache.assocNum);
  dCache.victim = NULL;
  dCache.nextLevel = l2Cache;

  initBlocks(dCache, dCache.blockNum);
  //dCache.victim = (struct cache*)malloc(sizeof(cache));
  //dCache.victim.blocks = (struct cacheBlock*)malloc(sizeof(cacheBlock));

  //L2Cache
  l2Cache = (struct cache*)malloc(sizeof(cache));
  l2Cache.blockSize = blocksize;
  l2Cache.setNum = l2cacheSets;
  l2Cache.assocNum = l2cacheAssoc;
  l2Cache.hitTime = l2cacheHitTime;
  
  l2Cache.offsetBits = log2(l2Cache.blocksize);
  l2Cache.indexBits = log2(l2Cache.setNum);
  l2Cache.tagBits = MAX_BITS - l2Cache.offsetBits - l2Cache.indexBits;
  l2Cache.blockNum = l2Cache.setNum*dCache.assocNum;  
  
  l2Cache.blocks = (struct cacheBlock*)malloc(sizeof(struct cacheBlock)*l2Cache.setNum*l2Cache.assocNum);
  l2Cache.victim = NULL;
  l2Cache.nextLevel = NULL;

  initBlocks(l2Cache, l2Cache.blockNum);
  //l2Cache.blocks = (struct cacheBlock*)malloc(sizeof(struct cacheBlock)*l2Cache.setNum*l2Cache.assocNum);
  //l2Cache.victim = (struct cache*)malloc(sizeof(cache));
  //l2Cache.victim.blocks = (struct cacheBlock*)malloc(sizeof(cacheBlock));

}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
void freeCache(struct cache *memory) {
  free(memory.blocks);
  free(memory);
}

uint32_t decodeIndex(cache* memory, uint32_t addr) {
  if(memory.indexBits == 0) return 0;
  else {
    addr = addr >> memory.offsetBits;
    uint32_t operand = 0;
    for(int i = 0; i < memory.indexBits; i++) {
      num += pow(2, i);
    }
    addr = addr&num;
  }
  return addr;
}

uint32_t decodeTag(cache* memory, uint32_t addr) {
  addr = addr >> (memory.offsetBits + memory.indexBits);
  return address; 
}

void updateLRU(struct cache *memory, uint32_t index, uint32_t tag) {
  uint32_t currRU = memory.blocks[(index*memory.assocNum) + tag].RU;
  for(uint32_t i = 0; i < memory.assocNum; i++) {
    if(memory.blocks[(index*memory.assocNum) + i].valid == 1 && memory.blocks[(index*memory.assocNum) + i].RU < currRU) {
      memory.blocks[(index*memory.assocNum) + i].RU++;
    }
  }
  memory.blocks[(index*memory.assocNum) + tag].RU = 0;
} 

void checkHitMiss(cache* memory, uint32_t index, uint32_t tag) {
  uint32_t row = index*memory.assocNum;

  for(uint32_t i = 0; i < memory.assocNum; i++) {
    if(memory.blocks[row + i].valid == 1 && memory.blocks[row + i].tag == tag)
      return i;
  }
  return -1;
}

uint32_t
icache_access(uint32_t addr)
{
  uint32_t index, tag;
  index = decodeIndex(iCache, addr);
  tag = decodeTag(dCache, addr);

  return memspeed;
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  //
  //TODO: Implement D$
  //
  return memspeed;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //
  return memspeed;
}
