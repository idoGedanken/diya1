#ifndef CAPSULE_H
#define CAPSULE_H
#define MAP_SIZE 50 // Amount of bits to write to the rfid
#define END_BLOCK 44 
#include "rfidInOut.h"
/**
 The data type and exact location in chip
  
  @field key: Data type
  @field block: Number of block the byte sits
  @field offset: How many blocks we skip
*/
struct DataTypeRfid
{
  String key;
  int block;  
  int offset;
  int numOfBytes;
};
/**
 struct that has an array of DataTypeRfid with locations
  
  @field arrayOfDataType: array that contains DataTypeRfids
*/
struct mapTypeLocation {
  DataTypeRfid arrayOfDataType[MAP_SIZE];
};
class chipMapper
{
  private:
    mapTypeLocation mapLocation;
    int indexCounter;
  
  public:
/**
  Constructor.
  initilizers counter to zero.
*/
    chipMapper() {
      indexCounter = 0;
    }
/**
 Maps each Data type to its location and inserts the struct objects to mapTypeLocation.
*/
    void initMap() {
      // init location to the first block of the USER DATA.
      int block = 4;
      int offset = 0;
      // Map between type to block and offset
      insertKey("CapType", &block, &offset,1);
      insertKey("currentAmount", &block, &offset,1);
      insertKey("hight", &block, &offset,1);
      insertKey("mixed", &block, &offset,1);
      //handles all the UID bytes
      handleUidBytes(&block, &offset);
      insertKey("CapsuleVolume1", &block, &offset,1);
      insertKey("CapsuleVolume2", &block, &offset,1);
      insertKey("QuantityResolution", &block, &offset,1);
      insertKey("mixingSpeed", &block, &offset,1);
      insertKey("mixingDuration", &block, &offset,1);
      insertKey("SingleOrMultiMix", &block, &offset,1);
      insertKey("PushOrPullRatio", &block, &offset,1);
      insertKey("CentralPistonMove", &block, &offset,1);
      insertKey("externalPistonMove", &block, &offset,1);
      insertKey("over/under pressure", &block, &offset,1);
    }
/**
Inserts key with location to map
  @param String key : the data type name
  @param int *block : pointer to the block number
  @param int *offset : pointer to the offset number
*/
    void insertKey(String key, int *block, int *offset,int numOfBytes) {
      int index = this->indexCounter;
      //check if the index not negative and if index is smaller than MAP_SIZE
      if ((index >= 0) && (index < MAP_SIZE)) {
        mapLocation.arrayOfDataType[index].key = key;
        mapLocation.arrayOfDataType[index].block = *block;
        mapLocation.arrayOfDataType[index].offset = *offset;
        mapLocation.arrayOfDataType[index].numOfBytes = numOfBytes;
        this->indexCounter = this->indexCounter +1;
        // If offset is three we need to move to next block in offset 0
        if (*offset == 3) {
          *block = *block + 1;
          *offset = 0;
        }
        else {
          *offset = *offset + 1;// Else, move one byte right
        }
        
      } else {
        Serial.println(F("unkown index"));
        return;
      }
    }
/**
Inserts key with location to map
  
  @param String key : the data type name
  @return  DataTypeRfid *: 
    - Found :block and offset by giving key
    - Not found : NULL
*/
   DataTypeRfid * findInMap(String key){
      int counterIndex = this->indexCounter - 1;
      for (int i=0;i <= counterIndex;i++){
        //Serial.println(mapLocation.arrayOfDataType[i].key);
        if (key == this->mapLocation.arrayOfDataType[i].key){
            return &((this->mapLocation).arrayOfDataType[i]);
        }
      }
      Serial.println(F("unkown key"));
      return NULL;
    }
/**
  Maps the UID bytes
 
  @param int* block : pointer to the block number
  @param int* offset: pointer to the offset number
*/
    void handleUidBytes(int* block, int* offset) {
      insertKey("UID0", block, offset,1);
      insertKey("UID1", block, offset,1);
      insertKey("UID2", block, offset,1);
      insertKey("UID3", block, offset,1);
      insertKey("UID4", block, offset,1);
      insertKey("UID5", block, offset,1);
      insertKey("UID6", block, offset,1);
    }
};
#endif