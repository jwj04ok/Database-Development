//
//  Index.hpp
//  RGAssignment8
//
//  Created by rick gessner on 5/17/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Index_h
#define Index_h

#include "Storage.hpp"
#include "keywords.hpp"
#include "Value.hpp"
#include "Helpers.hpp"

namespace ECE141 {
  
  using IntOpt = std::optional<uint32_t>;
  
  struct LessKey {
    bool operator()(const ValueType& anLHS, const ValueType& aRHS) const {
      return anLHS < aRHS; 
    }
  };

  class Index : public BlockIterator {
  public:

    using ValueMap = std::map<ValueType, uint32_t, LessKey>;

      Index(const std::string &aField, uint32_t aSchemaHash, DataType aType, Storage& aStorage)
      : field(aField), type(aType), schemaId(aSchemaHash), blockNum(0), theStorage(aStorage) {
        type=aType;
        changed=false;
      }
    
      Index(Storage &aStorage):field(""), type(DataType::no_type), schemaId(0), theStorage(aStorage){};
      
    virtual ~Index() {}
    
    ValueMap&           getList() {return list;}
    void                setChanged(bool aValue=true) {changed=aValue;}
    bool                isChanged() {return changed;}
    const std::string&  getFieldName() const {return field;}
    uint32_t            getBlockNum() const {return blockNum;}
    
      //manage keys/values in the index...
    Index& addKeyValue(const ValueType &aKey, uint32_t aValue);
    Index& removeKeyValue(const ValueType &aKey);    
    bool contains(const ValueType &aKey);    
    uint32_t getValue(const ValueType &aKey);
    
      //don't forget to support the storable interface IF you want it...
    StatusResult encode(StorageBlock& aBlock);
    StatusResult decode(const StorageBlock& aBlock);
    //void initBlock(StorageBlock &aBlock);

      //and the blockIterator interface...
    bool each(BlockVisitor &aVisitor) override;
    bool canIndexBy(const std::string &aField) override; 
    
  protected:
    
    std::string   field; //what field are we indexing?
    DataType      type;
    uint32_t      schemaId; //
    bool          changed;
    uint32_t      blockNum;  //storage block# of index...
    Storage       &theStorage;
    ValueMap      list;
  };

}
#endif /* Index_h */
