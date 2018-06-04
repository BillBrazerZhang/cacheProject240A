//========================================================//
//  cache->c                                               //
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

//
struct cache *iCache = NULL;
struct cache *dCache = NULL;  
struct cache *l2Cache = NULL; 
//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the blocks
void initBlocks(struct cache* memory, uint32_t blockNum) {
  for(uint32_t i = 0; i < blockNum; i++) {
    memory->blocks[i].tag = 0;
    memory->blocks[i].address = 0;
    memory->blocks[i].RU = memory->assocNum - 1;
    //memory->blocks[i].dirty = FALSE;
    memory->blocks[i].valid = TRUE;
  }
}

//init Cache structure
void initCache(struct cache* memory, uint32_t blocksize, uint32_t cacheSets, uint32_t cacheAssoc, uint32_t hitTime, int isL2) {
  //memory = (struct cache*)malloc(sizeof(struct cache));
  memory->blockSize = blocksize;
  memory->setNum = cacheSets;
  memory->assocNum = cacheAssoc;
  memory->hitTime = hitTime;
  //printf("    basic parameters setting finish...\n");
  memory->offsetBits = log2(memory->blockSize);
  memory->indexBits = log2(memory->setNum);
  memory->tagBits = MAX_BITS - (memory->offsetBits) - (memory->indexBits);
  memory->blockNum = memory->setNum*memory->assocNum;  
  //printf("    bits caclulation finish...\n");
  memory->blocks = (struct cacheBlock*)malloc(sizeof(struct cacheBlock)*(memory->setNum)*(memory->assocNum));
  initBlocks(memory, memory->blockNum);
  //printf("    blocks assign finish...\n");
  if(isL2 == FALSE){
    //initCache(memory->victim, blocksize, 1, 1, hitTime, TRUE);
    memory->nextLevel = l2Cache;
    //printf("    assign L2Cache\n");
  }else{
    return;
  }
  return;
}
// Initialize the Cache Hierarchy
//
void
init_cache()
{
  // Initialize cache stats
  //printf("init begin...\n");
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;

  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;

  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;
  
  //iCache
  //printf("  icache init begin...\n");
  if(icacheAssoc > 0 && icacheSets > 0 && blocksize > 0) {
    iCache = (struct cache*)malloc(sizeof(struct cache));
    //memory->blocks = (struct cacheBlock*)malloc(sizeof(struct cacheBlock)*memory->setNum*memory->assocNum);
    initCache(iCache, blocksize, icacheSets, icacheAssoc, icacheHitTime, FALSE);
  }

  //dCache
  //printf("  dcache init begin...\n");
  if(dcacheAssoc > 0 && dcacheSets > 0 && blocksize > 0) {
    dCache = (struct cache*)malloc(sizeof(struct cache)); 
    initCache(dCache, blocksize, dcacheSets, dcacheAssoc, dcacheHitTime, FALSE);
  }  
  //L2Cache
  if(iCache != NULL || dCache != NULL) {
    //printf("  l2cache init begin...\n");
    l2Cache = (struct cache*)malloc(sizeof(struct cache));
    if(dCache != NULL) dCache->nextLevel = l2Cache;
    if(iCache != NULL) iCache->nextLevel = l2Cache;
    initCache(l2Cache, blocksize, l2cacheSets, l2cacheAssoc, l2cacheHitTime, TRUE);
  }
  //printf("init successful!!!\n");

}

//free memory
void freeCache(struct cache *memory) {
  if(memory->nextLevel->blockNum != 0) freeCache(memory->nextLevel);
  free(memory->blocks);
  free(memory);
}

void cleanSpace() {
  if(iCache->blockNum != 0) freeCache(iCache);
  if(dCache->blockNum != 0) freeCache(dCache);
}
//calculate the Index
uint32_t decodeIndex(struct cache* memory, uint32_t addr) {
  //if(memory == NULL) printf("error!");
  if(memory->indexBits != 0) {
    return 0;
  }
  else {
    addr = addr >> memory->offsetBits;
    uint32_t operand = 0;
    for(int i = 0; i < memory->indexBits; i++) {
      operand += pow(2, i);
    }
    addr = addr&operand;
  }
  return addr;
}

//calculate the Tag
uint32_t decodeTag(struct cache* memory, uint32_t addr) {
  //printf("  decodeTage cal begin\n");
  addr = addr >> (memory->offsetBits + memory->indexBits);
  return addr; 
}


//replace the LRU
void updateLRU(struct cache *memory, uint32_t addr) {

  uint32_t column = checkHitMiss(memory, addr);
  uint32_t index = decodeIndex(memory, addr);

  if(column == -1) return;
  uint32_t currRU = memory->blocks[(index*memory->assocNum) + column].RU;
  for(uint32_t i = 0; i < memory->assocNum; i++) {
    if(memory->blocks[(index*memory->assocNum) + i].valid == 1 && memory->blocks[(index*memory->assocNum) + i].RU < currRU) {
      memory->blocks[(index*memory->assocNum) + i].RU++;
    }
  }
  memory->blocks[(index*memory->assocNum) + column].RU = 0;
  return; //most recently used -> LRU: 0 -> assocNum - 1 
} 

int checkHitMiss(struct cache* memory, uint32_t addr) {
  uint32_t index = decodeIndex(memory, addr);
  uint32_t tag = decodeTag(memory, addr);

  uint32_t row = index*memory->assocNum;

  for(int i = 0; i < memory->assocNum; i++) {
    if(memory->blocks[row + i].valid == TRUE && memory->blocks[row + i].address == addr)
      return i;
  }
  return -1; //404 not found
}

uint64_t accessCache(struct cache* memory, uint32_t addr, char mode) {
  //printf("  parameters define begin...\n");
  uint32_t index, tag;
  uint64_t timeCost = 0;
  int queryVictim = 0;
  //printf("  parameters define finish...\n");
  
  index = decodeIndex(memory, addr);
  //printf("  index calculate finish...\n");
  tag = decodeTag(memory, addr);
  //printf("  parameters calculate finish...\n");

  int queryResult = checkHitMiss(memory, addr);
  
  //printf("  check cache access begin...\n");

  if(queryResult >= 0) {
    //printf("    in the cache\n");
    if(mode == 'i') {
      printf("  i has\n");
      icacheRefs++;
      timeCost += icachePenalties;
      updateLRU(memory, addr);
    }
    if(mode == 'd') {
      printf("  c has\n");
      dcacheRefs++;
      timeCost += dcachePenalties;
      updateLRU(memory, addr);
    }
    if(mode == 'l') {
      printf("  l2 has\n");
      l2cacheRefs++;
      timeCost += l2cachePenalties;
      //swapCache(memory, addr);//swap the content in l1 and l2;
    }
    //if(queryVictim == 0) {
    //}
  }else {
    //printf("    not in the cache\n");
    //record the miss
    if(mode == 'i') {
      printf("  i not\n");
      icacheMisses++;
      timeCost += icachePenalties;
      timeCost += accessCache(memory->nextLevel, addr, 'l');
      //swapCache(memory, addr);
    }
    if(mode == 'd') {
      printf("  d not\n");
      dcacheMisses++;
      timeCost += dcachePenalties;
      timeCost += accessCache(memory->nextLevel, addr, 'l');
      //swapCache(memory, addr);
    }
    if(mode == 'l') {
      printf("  l2 has\n");
      l2cacheMisses++;
      timeCost += l2cachePenalties;
      timeCost += memspeed;
    }
    //addr not in the l1cache
    //if(mode == 'i' || mode == 'd') {
      //check if the victim is in use
      //if(memory->victim != NULL && memory->victim->blocks[0].valid == FALSE) {
        //in use
        //int queryVictimResult = accessVictimCache(memory, addr, index, tag, mode);
        //timeCost += (mode == 'i')? icachePenalties : dcachePenalties;
        //if(queryVictimResult == TRUE) return timeCost;
      //}
      //if victim not in use/not found, search in L2
      //swapCache(memory, addr);//swap the content in l1 and l2;
    //}
  }
  //printf("  accessCache() exit successful...\n");
  return timeCost;
  
  //return 0;
}

void swapCache(struct cache* memory, uint32_t addr) {
  //currently not in l1 nor l2;
  uint32_t index = decodeIndex(memory, addr);
  uint32_t tag = decodeTag(memory, addr);

  uint32_t l2Index = decodeIndex(memory->nextLevel, addr);
  uint32_t l2Tag = decodeTag(memory->nextLevel, addr);

  //int l2QueryRes = checkHitMiss(memory->nextLevel, addr);
  //uint32_t tempTag = memory->victim->blocks[0].tag;

  uint32_t tempAddr = fillCache(memory, addr);
  if(tempAddr == 0) return; //fill in the new block successful
  else {
    if(inclusive) {
      tempAddr = fillCache(memory->nextLevel, tempAddr);
      tempAddr = fillCache(memory->nextLevel, addr);
    }else {
      tempAddr = fillCache(memory->nextLevel, tempAddr);
      deleteL2Cache(memory->nextLevel, addr);
    }
  }
  return;

}
uint32_t
icache_access(uint32_t addr)
{ 
  printf("using icache begin... \n");
  uint32_t memspeed = (uint32_t)accessCache(iCache, addr, 'i');
  printf("using icache finish... \n\n");
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
  printf("using dcache begin... \n");
  uint32_t memspeed = (uint32_t)accessCache(dCache, addr, 'd');
  printf("using dcache finish... \n\n");
  return memspeed;
}

//fill the cache: return True if the cache has clean space, return False if the filling fails
uint32_t fillCache(struct cache *memory, uint32_t addr) {
  uint32_t index = decodeIndex(memory, addr);
  uint32_t tag = decodeTag(memory, addr);
  uint32_t fillResult = 0;

  int queryExist = checkHitMiss(memory, addr);
  if(queryExist >= 0) {
    updateLRU(memory, addr);
    return fillResult;
  }

  uint32_t row = index*memory->assocNum;
  for(int i = 0; i < memory->assocNum; i++) {
    if(memory->blocks[row + i].valid == TRUE) {
      memory->blocks[row + i].valid = FALSE;
      memory->blocks[row + i].tag = tag;
      memory->blocks[row + i].address = addr;
      fillResult = -1;
      updateLRU(memory, addr);
      return fillResult;
    }
  }

  uint32_t replaceLRU = memory->blocks[row].RU;
  int swapPos = 0;
  for(int i = 0; i < memory->assocNum; i++) {
    if(replaceLRU < memory->blocks[row + i].RU) {
      replaceLRU = memory->blocks[row + i].RU;
      swapPos = i;
    }
  }
  fillResult = memory->blocks[row + swapPos].address;
  memory->blocks[row + swapPos].address = addr;
  updateLRU(memory, addr);
  return fillResult;
}

void deleteL2Cache(struct cache *memory, uint32_t addr) {
  uint32_t index = decodeIndex(memory, addr);
  uint32_t tag = decodeTag(memory, addr);
  int fillResult = -1;
  uint32_t row = index*memory->assocNum;
  for(int i = 0; i < memory->assocNum; i++) {
    if(memory->blocks[row + i].address == addr) {
      memory->blocks[row + i].valid = TRUE;
      memory->blocks[row + i].RU = memory->assocNum - 1;
      memory->blocks[row + i].address = 0;
      memory->blocks[row + i].tag = 0;     
      return;
    }
  }
  return;
} 

/*uint32_t fillL2Cache(struct cache *memory, uint32_t addr) {
  uint32_t index = decodeIndex(memory, addr);
  uint32_t tag = decodeTag(memory, addr);
  uint32_t fillResult = 0;
  uint32_t row = index*memory->assocNum;
  for(int i = 0; i < memory->assocNum; i++) {
    if(memory->blocks[row + i].valid == TRUE) {
      memory->blocks[row + i].valid = FALSE;
      memory->blocks[row + i].tag = tag;
      memory->blocks[row + i].address = addr;
      fillResult = -1;
      updateLRU(memory, index, tag);
      return;
    }
  }

  uint32_t replaceLRU = memory->blocks[row].RU;
  int swapPos = 0;
  for(int i = 0; i < memory->assocNum; i++) {
    if(replaceLRU < memory->blocks[row + i].RU) {
      replaceLRU = memory->blocks[row + i].RU;
      swapPos = i;
    }
  }
  memory->blocks[row + swapPos].valid = FALSE;
  memory->blocks[row + swapPos].tag = tag;
  memory->blocks[row + swapPos].address = addr;
  updateLRU(memory, index, tag);

  return;
}
*/

/*
int createSpace(struct cache* memory, uint32_t index, uint32_t tag) {
  uint32_t row = index * memory->assocNum;
  int  i = 0;
  for(i; i < memory->assocNum; i++) {
    if(memory->blocks[row + i].count == (memory->assocNum -1))
      break;
  }

  uint32_t blockAddress;
  blockAddress = memory->blocks[row + i].tag << memory->indexBits;
  blockAddress = blockAddress << memory->offsetBits;
  blockAddress = blockAddress + (index<<memory->offsetBits);

  if(memory->blocks[row + i].valid == 1 && memory->nextLevel != NULL) {
    accessCache(memory->nextLevel, blockAddress, 'l');
  }

  memory->blocks[row + i].tag = 0;
  memory->blocks[row + i].valid = 0;
  memory->blocks[row + i].count = memory->assocNum - 1;

  return j;

}
*/

//access Victim, if the result hits the victim, swap the value, update the count,
//return the result indicate if hits

/*int accessVictimCache(struct cache *memory, uint32_t addr, uint32_t index, uint32_t tag, char mode)
{
  int query = FALSE;
  if(memory->victim->blocks[0].address == addr) {
    //find the result in the victim cache
    query = TRUE;
    //swap value with l1cache
    uint32_t row = index * memory->assocNum;
    uint32_t replaceLRU = memory->blocks[row].RU;
    uint32_t swapPos = 0;
    uint32_t i = 0;
    for(i = 0; i < memory->assocNum; i++) {
      if(replaceLRU < memory->blocks[row + i].RU) {
        replaceLRU = memory->blocks[row + i].RU;
        swapPos = i;
      }
    }
    memory->victim->blocks[0].tag = memory->blocks[row + i].tag;
    memory->victim->blocks[0].address = memory->blocks[row + i].address;

    memory->blocks[row + i].tag = tag;
    memory->blocks[row + i].address = addr;
    updateLRU(memory, index, tag);
    return query;
  }else {
    return FALSE;
  }   
}
*/
     /*int j;
     uint32_t row = index * memory->assocNum;
     for(j=0;j<memory->assocNum;j++)
     {
        //assert(memory->blocks[row + j].count <= (memory->assocNum - 1));
        if( memory->blocks[row + j].valid == 0)
        {
             return 0;
        }
     }
     uint32_t requiredAddress;
     requiredAddress = tag<<memory->indexBits;
     requiredAddress = requiredAddress<<memory->offsetBits;
     requiredAddress = requiredAddress + (index<<memory->offsetBits);

     for(j=0;j<memory->assocNum;j++)
     {
        //assert(mem->blocks[row + j].count <= (mem->assoc - 1));
        if( memory->blocks[row + j].count == (memory->assocNum - 1)) {
             break;
        }
     }
     //assert(j <= (mem->assoc - 1));

     uint32_t identifiedAddress;
     identifiedAddress = memory->blocks[row+j].tag<<memory->indexBits;
     identifiedAddress = identifiedAddress<<memory->offsetBits;
     identifiedAddress = identifiedAddress + (index<<memory->offsetBits);
     
     //assert(mem->vc->rows == 1);

     //INFO: victim hit or miss
     int column;
     column = checkHitMiss(memory->victim, decodeTag(memory->victim, requiredAddress),0);
     if(column < 0)
     {

         //memory->victim->readMiss++;
         column = createSpace(memory->victim,decodeTag(memory->victim, identifiedAddress), 0);
         if(memory->nextLevel != NULL)
         {
            accessCache(memory->nextLevel, requiredAddress, mode);
         }
         memory->blocks[row + j].valid = 1;
         memory->blocks[row + j].tag = tag;
         memory->victim->blocks[column].tag = decodeTag(memory->victim, identifiedAddress);
         memory->victim->blocks[column].valid = 1;
         updateLRU(memory->victim, 0, column);
         updateLRU(memory, index, j);
     }
     else
     {
         //memory->victim->readhi
         //mem->swaps++;
         memory->victim->blocks[column].tag = decodeTag(memory->victim, identifiedAddress);
         memory->victim->blocks[column].valid = 1;
         updateLRU(memory->victim, 0, column);
         memory->blocks[row + j].valid = 1;
         memory->blocks[row + j].tag = tag;
         updateLRU(memory, index, j);
     }
         //DEBUG HELP: printf("%d %d %d %d\n",mem->vc->read_hits,mem->vc->read_misses,mem->vc->write_hits,mem->vc->write_misses);
     return 1;
     */

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

