//
//  StorageBlock.hpp
//  Assignment3
//
//  Created by rick gessner on 4/11/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef StorageBlock_hpp
#define StorageBlock_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <variant>
#include <iostream>
#include <cstring>

namespace ECE141 {

  //a "storage" file is comprised of fixed-sized blocks (defined below)

const size_t kPayloadSize = 1012; //area reserved in storage-block for user data...
const size_t kStoredIdentifierLen = 15;

//a short version of Schema for encode/decode
struct SchemaSummary { 
    SchemaSummary() {
          name[0] = 0;
          blocknum = 0;
    };

    SchemaSummary(const SchemaSummary& aCopy) {

          size_t theLen = strlen(aCopy.name);
#if defined(__APPLE__) || defined(__linux)
          std::strncpy(name, aCopy.name, theLen);
#else
          strncpy_s(name, kStoredIdentifierLen, aCopy.name, theLen);
#endif
          name[theLen] = 0;
          blocknum = aCopy.blocknum;
     }

      char name[kStoredIdentifierLen + 1];
      uint32_t blocknum;
  };

  struct SchemaSummaryHeader {
      int count; //number of schemas
      uint32_t reserved;
  };

  const size_t kEntitiesPerBlock = (kPayloadSize - sizeof(SchemaSummaryHeader)) / sizeof(SchemaSummary);

  struct SchemaSummaries {
      SchemaSummaryHeader header;
      SchemaSummary items[kEntitiesPerBlock];
  };


 enum class BlockType {
   meta_block='T',
   data_block='D',
   entity_block='E',
   free_block='F',
   index_block='I',
   unknown_block='V',
 };

 //Translate a char type to a full name
 static std::map<char, std::string> TranslateBack={
   std::make_pair('T', "meta"),
   std::make_pair('E', "schema"),
   std::make_pair('D', "data"),
   std::make_pair('I', "index"),
 };

 struct BlockHeader {
  
   BlockHeader(BlockType aType=BlockType::data_block)
     : type(static_cast<char>(aType)), id(0) {}
  
   BlockHeader(const BlockHeader &aCopy) {
     *this=aCopy;
   }
  
   BlockHeader& operator=(const BlockHeader &aCopy) {
     type=aCopy.type;
     id=aCopy.id;
     schemaHash = aCopy.schemaHash;
     return *this;
   }
  
   char      type;     //char version of block type {[D]ata, [F]ree... }
   uint32_t  id;       //use this anyway you like
   uint32_t  schemaHash; //hashed table name
 };

 struct StorageBlock {
       
   StorageBlock(BlockType aType=BlockType::data_block);
   
   StorageBlock(const StorageBlock &aCopy);
   StorageBlock& operator=(const StorageBlock &aCopy);
   StorageBlock& store(std::ostream &aStream);
      

   BlockHeader         header;
   union {
       char            data[kPayloadSize];
       SchemaSummaries schemas; //only meta block can have this.
   };

   friend class Schema;

 };
 
}

#endif /* StorageBlock_hpp */
