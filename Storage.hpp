//
//  Storage.hpp
//  Assignment2
//
//  Created by rick gessner on 4/5/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <fstream>
#include <variant>
#include "Errors.hpp"
#include "StorageBlock.hpp"
#include <functional>
#include <sstream>
#include "Value.hpp"
#include "Row.hpp"
#include "Schema.hpp"
#include "Filters.hpp"


namespace ECE141 {
    
  //first, some utility classes...

  class StorageInfo {
  public:
    static const char* getDefaultStoragePath();
  };

  struct CreateNewStorage {};
  struct OpenExistingStorage {};

  class Storage;

  using StorageCallback = std::function<StatusResult(Storage& aStorage, const StorageBlock & aBlock, uint32_t aBlockNum)>;

class BlockVisitor{
public:
    BlockVisitor(RowCollection &theCollection, const Schema &theSchema, const Filters &theFilters): aCollection(theCollection), aSchema(theSchema), filters(theFilters){}
     
    StatusResult operator()(Storage &aStorage, const StorageBlock &aBlock, uint32_t aBlockID);
protected:
    RowCollection &aCollection;
    const Schema  aSchema;
    const Filters filters;
    
};

struct BlockIterator {
   virtual bool each(BlockVisitor &aVisitor)=0;
   virtual bool canIndexBy(const std::string &aField) {return false;} //override this
};

  class Storage: public BlockIterator{
  public:
    Storage() {};
    Storage(const std::string aName, CreateNewStorage);
    Storage(const std::string aName, OpenExistingStorage);
    ~Storage();        
    uint32_t        getTotalBlockCount();

    
    //high-level IO (you're not required to use this, but it may help)...    
    StatusResult    save(Storable &aStorable); //using a stream api
    StatusResult    load(Storable &aStorable); //using a stream api
    
    //low-level IO...    
    StatusResult    readBlock(StorageBlock &aBlock, uint32_t aBlockNumber);
    StatusResult    writeBlock(StorageBlock &aBlock, uint32_t aBlockNumber);
    StatusResult    write(StorageBlock& aBlock, std::fstream& aStream);
    StatusResult    addBlock(StorageBlock& aBlock); 

    StatusResult    initialFile();
    StatusResult    initialStorage();
      
    static std::string     getPath(const std::string& aName);
    bool            each(BlockVisitor &aVisitor){return true;};
    bool            canIndexBy(const std::string &aField) {return false;};
    StatusResult    eachBlock(StorageCallback aCallback);
    StatusResult    addSchema(const std::string& aName, StorageBlock& aBlock);
    StatusResult    dropSchema(const std::string &aName);
    StatusResult    releaseBlock(int aBlockNumber);

    StatusResult    loadTOC();
    StorageBlock&   getTOC(){return tempBlock;};
    SchemaSummary*  findSchemaInTOC(const std::string &aName);
      
  protected:
    bool            isReady() const;

    StorageBlock    tempBlock;
    std::string     name;    
    StatusResult    findFreeBlockNum();        
    std::fstream    stream;
      
    friend class Database;
  };



}

#endif /* Storage_hpp */
