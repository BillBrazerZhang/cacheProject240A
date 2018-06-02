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
  //bool dirty;
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
cache *iCache = NULL;
cache *dCache = NULL;  
cache *l2Cache = NULL; 
//------------------------------------//
//          Cache Functions           //
//------------------------------------//
// Initialize the blocks
void initBlocks(struct cache* memory, uint32_t blockNum) {
  for(uint32_t i = 0; i < blockNum; i++) {
    memory.blocks[i].tag = 0;
    memory.blocks[i].address = 0;
    memory.blocks[i].RU = memory.assocNum - 1;
    //memory.blocks[i].dirty = FALSE;
    memory.blocks[i].valid = TRUE;
  }
}

void initCache(struct cache* memory, uint32_t blocksize, uint32_t cacheSets, uint32_t cacheAssoc, uint32_t hitTime, int isL2) {
  memory = (struct cache*)malloc(sizeof(cache));
  memory.blockSize = blocksize;
  memory.setNum = icacheSets;
  memory.assocNum = icacheAssoc;
  memory.hitTime = icacheHitTime;

  memory.offsetBits = log2(memory.blocksize);
  memory.indexBits = log2(memory.setNum);
  memory.tagBits = MAX_BITS - memory.offsetBits - memory.indexBits;
  memory.blockNum = memory.setNum*memory.assocNum;  

  memory.blocks = (struct cacheBlock*)malloc(sizeof(struct cacheBlock)*memory.setNum*memory.assocNum);
  if(isL2 == FALSE){
    initCache(memory.victim, blocksize, 1, 1, hitTime);
    memory.nextLevel = l2Cache;
  }

  initBlocks(memory, memory.blockNum);
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
  if(icacheAssoc > 0 && icacheSets > 0 && blocksize > 0)
    initCache(iCache, blocksize, icacheSets, icacheAssoc, icacheHitTime, FALSE);
  //iCache.victim = (struct cache*)malloc(sizeof(cache));
  //iCache.victim.blocks = (struct cacheBlock*)malloc(sizeof(struct cacheBlock));

  //dCache
  if(dcacheAssoc > 0 && dcacheSets > 0 && blocksize > 0)  
    initCache(dCache, blocksize, dcacheSets, dcacheAssoc, dcacheHitTime, FALSE);
  //dCache.victim = (struct cache*)malloc(sizeof(cache));
  //dCache.victim.blocks = (struct cacheBlock*)malloc(sizeof(cacheBlock));

  //L2Cache
  if(iCache != null || dCache != null)
    initCache(l2Cache, blocksize, l2cacheSets, l2cacheAssoc, l2cacheHitTime, TRUE);
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

int checkHitMiss(cache* memory, uint32_t index, uint32_t tag) {
  uint32_t row = index*memory.assocNum;

  for(int i = 0; i < memory.assocNum; i++) {
    if(memory.blocks[row + i].valid == 1 && memory.blocks[row + i].tag == tag)
      return i;
  }
  return -1;
}

uint64_t accessCache(struct cache* memory, uint32_t addr, char mode) {
  uint32_t index, tag;
  uint64_t timeCost = 0;
  int queryVictim = 0;

  index = decodeIndex(memory, addr);
  tag = decodeTag(memory, addr);
  int queryResult = checkHitMiss(memory, index, tag);
  if(queryResult >= 0) {
    if(mode == 'i') {
      icacheRefs++;
      timeCost += icachePenalties;
    }
    if(mode == 'd') {
      dcacheRefs++;
      timeCost += dcachePenalties;
    }
    if(mode == 'l') {
      l2cacheRefs++;
      timeCost += l2cachePenalties;
    }
  }else {
    if(mode == 'i') {
      icacheMisses++;
      timeCost += icachePenalties;
    }
    if(mode == 'd') {
      dcacheMisses++;
      timeCost += dcachePenalties;
    }
    if(mode == 'l') {
      l2cacheMisses++;
      timeCost += l2cachePenalties;
    }
    if(memory.victim != NULL) {
      queryVictim = accessVictimCache(memory, index, tag, mode);
    }

    if(queryVictim == 0) { //check what would happen in L2
      queryResult = createSpace(memory, index, tag, mode);
    }

    uint64_t l2TimeCost = 0;
    if(queryVictim == 0 && memory.nextLevel != NULL) {
      l2TimeCost = accessCache(l2Cache, addr, 'l');
      timeCost += l2TimeCost;
    }

    if(queryVictim == 0) {
      memory.blocks[index*memory.assocNum + queryResult].valid = 1;
      memory.blocks[index*memory.assocNum + queryResult].tag = tag;
    }
  }
  if(queryVictim == 0) {
    updateLRU(memory, index, queryResult);
  }

  return timeCost;
}

uint32_t
icache_access(uint32_t addr)
{
  uint32_t memspeed = (uint32_t)accessCache(iCache, addr, 'i');
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
  uint32_t memspeed = (uint32_t)accessCache(l2Cache, addr, 'd');
  return memspeed;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
/*uint32_t
l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //
  return memspeed;
}
*/
