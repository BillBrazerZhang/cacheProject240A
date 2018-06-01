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
  uint64_t tag;
  uint64_t address //address recorded
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
  
  bool inclu;

  //performance
  uint32_t readMiss;
  uint32_t readHit;

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
  iCache.inclu = inclusive;
  iCach.hitTime = icacheHitTime;
  iCache.victim = (struct cache*)malloc(sizeof(cache));

  dCache = (struct cache*)malloc(sizeof(cache));
  dCache.blockSize = blocksize;
  dCache.setNum = icacheSets;
  dCache.assocNum = icacheAssoc;
  dCache.inclu = inclusive;
  dCache.hitTime = dcacheHitTime;
  dCache.victim = (struct cache*)malloc(sizeof(cache));

  l2Cache = (struct cache*)malloc(sizeof(cache));
  l2Cache.blockSize = blocksize;
  l2Cache

}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t decodeIndex(uint32_t addr) {

}

uint32_t decodeTag(uint32_t addr) {

}

void checkHitMiss(cache* destCache, uint32_t addr, uint32_t index, uint32_t tag) {
//check if value exists, if so: return, add penalty time
//if not: check victim, if there has, exchange
          // if not check L2, cal time
          //exchange of fill in the cache with current value 
}

uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //
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
