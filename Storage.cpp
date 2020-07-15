//
//  Storage.cpp
//  Assignment2
//
//  Created by rick gessner on 4/5/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Storage.hpp"
#include <fstream>
#include <cstdlib>
#include <iostream>

namespace ECE141 {

  const char* StorageInfo::getDefaultStoragePath() {
    //STUDENT -- MAKE SURE TO SET AN ENVIRONMENT VAR for DB_PATH! 
    //           This lets us change the storage location during autograder testing

    //WINDOWS USERS:  Use forward slash (/) not backslash (\) to separate paths.
    //                (Windows convert forward slashes for you)
    
    const char* thePath = std::getenv("DB_PATH");
    return thePath;
  }

  //----------------------------------------------------------

  //path to the folder where you want to store your DB's...
  std::string getDatabasePath(const std::string &aDBName) {
    std::ostringstream theStream;
    theStream << StorageInfo::getDefaultStoragePath() << "/" << aDBName << ".db";
    return theStream.str();
  }

//-------------------------------------------------------------
StatusResult BlockVisitor:: operator()(Storage &aStorage, const StorageBlock &aBlock, uint32_t aBlockID){
    StatusResult theResult{noError};
    KeyValues theValues;
    if (aBlock.header.type == 'D' && aBlock.header.schemaHash == aSchema.getHash()) {
        theResult = aSchema.decodeRows(aBlock, theValues);
        if (filters.matches(theValues)) {
            aCollection.add(new Row(theValues, aBlockID));
        }
    }
    return theResult;
}

  //create a new storage
  Storage::Storage(const std::string aName, CreateNewStorage) : tempBlock(BlockType::meta_block),name(aName) {
    std::string thePath = getDatabasePath(name);
    //try to create a new db file in known storage location.
    //throw error if it fails...
    StatusResult createResult = initialFile();
    if (!createResult) throw std::runtime_error("Can't create file");
  }

  //open an existing storage from db file
  Storage::Storage(const std::string aName, OpenExistingStorage) : tempBlock(BlockType::meta_block), name(aName) {
    std::string thePath = getDatabasePath(aName);
    //try to OPEN a db file a given storage location
    //if it fails, throw an error
    stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out);
    if (stream) {
        stream.seekg(stream.tellg(), std::ios::beg);
        
        if (!loadTOC()) {
            throw std::runtime_error("Can't read file");
        }
    }else {
        throw std::runtime_error("Can't open file");
  }
  }

  Storage::~Storage() {
    stream.close();
  }

  //read the meta block to get stored schemas
  StatusResult Storage::loadTOC() {
    stream.seekg(stream.tellg(), std::ios::beg);
    return Storage::readBlock(tempBlock, 0);
}

  StatusResult  Storage::initialFile() {
      std::string thePath = getPath(name);
      stream.clear();
      stream.open(thePath.c_str(), std::fstream::in | std::fstream::out | std::fstream::trunc);
      stream.close();
      stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out);
      if (isReady()) {
          return initialStorage();
      }
      return StatusResult{ writeError };
  }

  StatusResult  Storage::initialStorage() {
      StatusResult aResult = findFreeBlockNum();
      if (aResult) {
          return writeBlock(tempBlock,aResult.value);
      }

      stream.seekg(stream.tellg(), std::ios::beg);
      stream.seekp(0, std::ios::end);
      return write(tempBlock, stream);
  }

  std::string Storage::getPath(const std::string& aName) {
      std::ostringstream theStream;
      theStream << StorageInfo::getDefaultStoragePath() << "/" << aName << ".db";
      return theStream.str();
  }

//check it is open
  bool Storage::isReady() const {
    return stream.is_open();
  }

//count used block number (including released)
  uint32_t Storage::getTotalBlockCount() {
      stream.seekg(stream.tellg(), std::ios::beg);
      stream.seekg(0, std::ios::end);
      int thePos = (int)stream.tellg();
      return thePos / sizeof(StorageBlock);
  }

  // Call this to locate a free block in this storage file.
  // If you can't find a free block, then append a new block and return its blocknumber
  StatusResult Storage::findFreeBlockNum() {
      uint32_t theCount = getTotalBlockCount();
      StatusResult theResult{ noError };
      StorageBlock theBlock;

      stream.seekg(stream.tellg(), std::ios::beg);
      for (uint32_t theIndex = 0; theIndex < theCount; theIndex++) {
          theResult = readBlock(theBlock, theIndex);
          if (!theResult) return theResult;
          if ('F' == theBlock.header.type) {
              return StatusResult{ noError, theIndex };
          }
      }
    return StatusResult{noError, theCount}; //return blocknumber in result.value
  }

  StatusResult Storage::save(Storable &aStorable) {
    //High-level IO: save a storable object (like a table row)...
     return StatusResult{noError};
  }

  StatusResult Storage::load(Storable &aStorable) {
    //high-level IO: load a storable object (like a table row)
    return StatusResult{noError};
  }

  //flush data from stream to db file
  StatusResult Storage::write(StorageBlock& aBlock, std::fstream& aStream) {
      if (aStream.write((char*)&aBlock, sizeof(StorageBlock))) {
          aStream.flush();
          return StatusResult{};
      }
      return StatusResult{ writeError };
  }

  // USE: write data a given block (after seek)
  StatusResult Storage::writeBlock(StorageBlock &aBlock, uint32_t aBlockNumber) {
    //STUDENT: Implement this; this is your low-level block IO...
      stream.seekg(stream.tellg(), std::ios::beg); 
      stream.seekp(aBlockNumber * sizeof(StorageBlock));
      if (write(aBlock, stream)) return StatusResult{noError, aBlockNumber};
      return StatusResult{writeError};
  }

  StatusResult Storage::addBlock(StorageBlock& aBlock) {
      StatusResult theResult = findFreeBlockNum();
      if (theResult) {
          theResult = writeBlock(aBlock,theResult.value);
          return theResult;
      }
      stream.seekg(stream.tellg(), std::ios::beg); 
      stream.seekp(0, std::ios::end);
      StatusResult writeResult = write(aBlock, stream);
      if (!writeResult) return writeResult;
      return theResult; //theResult.value = blockNum
  }

  // USE: read data from a given block (after seek)
  StatusResult Storage::readBlock(StorageBlock &aBlock, uint32_t aBlockNumber) {
    //STUDENT: Implement this; this is your low-level block IO...
      stream.seekg(aBlockNumber * sizeof(StorageBlock));
      if (stream.read((char*)&aBlock, sizeof(StorageBlock))) {
          return StatusResult{ noError };
      }
      return StatusResult(readError);
  }

  StatusResult Storage::eachBlock(StorageCallback aCallback) {
      int Count = getTotalBlockCount();
      StorageBlock theBlock;
      StatusResult theResult{noError};
      int freeBlockCount = 0;
      for (uint32_t theIndex = 0; theIndex < Count; theIndex++) {
          theResult = readBlock(theBlock, theIndex);
          if (theResult) {
              theResult = aCallback(*this, theBlock, theIndex);
              if (theResult.value == 1) freeBlockCount++;
              if (!theResult) break;
          }
          else break;
      }
      theResult.value = Count - freeBlockCount;
      return theResult;
  }

StatusResult Storage::addSchema(const std::string& aName, StorageBlock& aBlock) {
    StatusResult theResult = findFreeBlockNum();
     
    if (theResult) {
          SchemaSummary theSchema;

          size_t theLength = aName.length();
#if defined(__APPLE__) || defined(__linux__)
          std::strncpy(theSchema.name, aName.c_str(), theLength);
#else
          strncpy_s(theSchema.name, kStoredIdentifierLen, aName.c_str(), theLength);
#endif
          theSchema.name[theLength] = 0;
          theSchema.blocknum = theResult.value;
          memcpy(&tempBlock.schemas.items[tempBlock.schemas.header.count++],  &theSchema, sizeof(SchemaSummary));
          theResult = writeBlock(aBlock,theSchema.blocknum);
          theResult.value = theSchema.blocknum;
      }

      return theResult;
  }

//get quick access to schema block by finding its index(block num)
int findIndexOfSchemaInTOC(const SchemaSummaries &aList, const std::string &aName) {
     for (int i = 0; i < aList.header.count; i++) {
         if (aName == aList.items[i].name) {
             return i;
         }
     }
     return -1;
 }

SchemaSummary* Storage:: findSchemaInTOC(const std::string &aName){
    int index = findIndexOfSchemaInTOC(tempBlock.schemas, aName);
    if (0 <= index){
        return &tempBlock.schemas.items[index];
    }
    return nullptr;
}

StatusResult Storage::releaseBlock(int aBlockNumber) {
//write back an empty block at the place of the original block
    StorageBlock theBlock;
    memset(&theBlock, 0, sizeof(theBlock)); //make an empty block, mark as free block
    theBlock.header.type = 'F';
    return writeBlock(theBlock, aBlockNumber);
}

StatusResult Storage::dropSchema(const std::string &aName) {
//    find schema; header --;
    int theIndex = findIndexOfSchemaInTOC(tempBlock.schemas, aName);
    if (theIndex >= 0) {
        //if theindex < toc.schemas.count, overwrite entity at theIndex, with last schema
        SchemaSummary *theLastSchemaSmry = &tempBlock.schemas.items[tempBlock.schemas.header.count - 1];
        SchemaSummary *theSchemaSmry = &tempBlock.schemas.items[theIndex];
        releaseBlock(theSchemaSmry->blocknum); //mark the block as free...
        memset(theSchemaSmry, 0, sizeof(SchemaSummary)); 
        if (theSchemaSmry < theLastSchemaSmry) {
            memcpy(theSchemaSmry, theLastSchemaSmry, sizeof(SchemaSummary));
        }
        tempBlock.schemas.header.count--;
        return StatusResult{noError};
    }
    return StatusResult{notImplemented};
}


}


