//========================================================//
//  cache.h                                               //
//  Header file for the Cache Simulator                   //
//                                                        //
//  Includes function prototypes and global variables     //
//  and defines for the cache simulator                   //
//========================================================//

#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
// Student Information
//
extern const char *studentName;
extern const char *studentID;
extern const char *email;

//------------------------------------//
//          Global Defines            //
//------------------------------------//

#define TRUE 1
#define FALSE 0
#define MAX_BITS 32
//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

extern uint32_t icacheSets;     // Number of sets in the I$
extern uint32_t icacheAssoc;    // Associativity of the I$
extern uint32_t icacheHitTime;  // Hit Time of the I$

extern uint32_t dcacheSets;     // Number of sets in the D$
extern uint32_t dcacheAssoc;    // Associativity of the D$
extern uint32_t dcacheHitTime;  // Hit Time of the D$

extern uint32_t l2cacheSets;    // Number of sets in the L2$
extern uint32_t l2cacheAssoc;   // Associativity of the L2$
extern uint32_t l2cacheHitTime; // Hit Time of the L2$
extern uint32_t inclusive;      // Indicates if the L2 is inclusive

extern uint32_t blocksize;      // Block/Line size
extern uint32_t memspeed;       // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

extern uint64_t icacheRefs;       // I$ references
extern uint64_t icacheMisses;     // I$ misses
extern uint64_t icachePenalties;  // I$ penalties

extern uint64_t dcacheRefs;       // D$ references
extern uint64_t dcacheMisses;     // D$ misses
extern uint64_t dcachePenalties;  // D$ penalties

extern uint64_t l2cacheRefs;      // L2$ references
extern uint64_t l2cacheMisses;    // L2$ misses
extern uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//      Cache Function Prototypes     //
//------------------------------------//

struct cacheBlock {
  uint32_t tag;
  uint32_t address; //address recorded
  uint32_t RU; //recently used
  //bool dirty;
  int valid;
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
  //struct cache *victim;
  struct cache *nextLevel;
};

// Initialize the predictor
//
void initBlocks(struct cache* memory, uint32_t blockNum);

void initCache(struct cache* memory, uint32_t blocksize, uint32_t cacheSets, uint32_t cacheAssoc, uint32_t hitTime, int isL2);

void init_cache();

void freeCache(struct cache *memory);


uint32_t accessCache(struct cache* memory, uint32_t addr, char mode);

uint32_t icache_access(uint32_t addr);

uint32_t dcache_access(uint32_t addr);


uint32_t decodeTag(struct cache* memory, uint32_t addr);

uint32_t decodeIndex(struct cache* memory, uint32_t addr);

int checkHitMiss(struct cache* memory, uint32_t addr);

void updateLRU(struct cache *memory, uint32_t addr);


uint32_t fillCache(struct cache *memory, uint32_t addr);

void fillL2Cache(struct cache *memory, uint32_t addr);

void deleteL2Cache(struct cache *memory, uint32_t addr);

void swapCache(struct cache* memory, uint32_t addr);

void cleanSpace();

#endif
