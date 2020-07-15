//
//  Schema.hpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Schema_hpp
#define Schema_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include "Attribute.hpp"
#include "Errors.hpp"
#include "View.hpp"
#include "Helpers.hpp"
#include <optional>
#include "Value.hpp"
#include <iomanip>

namespace ECE141 {
  
  struct StorageBlock;
  struct Expression;
  class  Database;
  class  Tokenizer;
  

  using AttributeList = std::vector<Attribute>;
  using AttributeOpt  = std::optional<Attribute>;

  class Schema  {
  public:
                          Schema(const std::string aName);
                          Schema(const Schema &aCopy);
                          Schema(StorageBlock &aBlock, uint32_t aBlockId = 0);
                          Schema();
                          ~Schema();

    operator              StorageBlock();

    const std::string&    getName() const {return name;}
    const AttributeList&  getAttributes() const {return attributes;}
    uint32_t              getBlockNum() const {return blockNum;}
    uint32_t              getHash() const { return hash; }
    uint32_t              getIndexBlockNum() const { return indexBlockNum; }

    bool                  isChanged() {return changed;}

    StatusResult          encodeRows(KeyValues &aKeyValueList, StorageBlock &aBlock);
    StatusResult          decodeRows(const StorageBlock &aBlock, KeyValues &aList)const;
    
    Schema&               addAttribute(const Attribute &anAttribute);
    AttributeOpt          getAttribute(const std::string &aName) const;
      
    StatusResult          validate(KeyValues &aList);
    bool                  validateAttribute(const std::string& name, const Value& aValue);
    
    std::string           getPrimaryKeyName() const;
    uint32_t              getNextAutoIncrementValue();
    uint32_t              hashString(const char *aString);
    
    
    friend class Database; 

  protected:
    
    AttributeList   attributes;
    std::string     name;
    uint32_t        count;
    uint32_t        blockNum;  //storage location.
    uint32_t        autoInc;
    uint32_t        hash;
    uint32_t        indexBlockNum;
    bool            changed;
      
  };
  
}
#endif /* Schema_hpp */
