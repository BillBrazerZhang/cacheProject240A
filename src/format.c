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