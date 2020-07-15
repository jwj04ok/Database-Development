//
//  Database.hpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/18.
//  Copyright © 2018 rick gessner. All rights reserved.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <string>
#include <iostream>
#include "Storage.hpp"
#include "View.hpp"
#include <iomanip>
#include "Schema.hpp"
#include <map>
#include "Row.hpp"
#include "Filters.hpp"
#include "Index.hpp"


namespace ECE141 {
  
  class Database  {
  public:
    
    Database() {};
    Database(const std::string aPath, CreateNewStorage);
    Database(const std::string aPath, OpenExistingStorage);
    ~Database();

    StatusResult      createTable(Schema* aSchema);
    StatusResult      showTables(std::ostream& anOutput);
    StatusResult      dropTable(const std::string& aName);
    StatusResult      describe(std::ostream& anOutput); //database
    StatusResult      describeTable(std::ostream& anOutput, Schema* aSchema);
    StatusResult      insertRows(const Row& aRow, const std::string& aTableName);
    StatusResult      deleteRows(RowCollection& aCollection, const Schema& aSchema, const Filters &theFilters);
    StatusResult      selectRows(RowCollection& aCollection, const Schema& aSchema, const Filters& aFilters);
    StatusResult      updateRow(Row& aRow, const KeyValues& aKVList, Schema& aSchema);
    StatusResult      showIndexs(std::ostream& anOutput);
    StatusResult      saveToc();
    
    Storage&          getStorage() {return storage;}
    std::string&      getName() {return name;}
    Schema*           getSchema(const std::string &aName);
    Index*            getIndex(const std::string &tableName, const std::string &aField);
   
  protected:
    std::string                     name;
    Storage                         storage;
    std::map<std::string, Schema*>  schemas;
    std::map<std::string, Index*>   indexs;
  };

//This class is used when describe database statement is called
class DescribeDatabaseView : public View {
 public:
     DescribeDatabaseView(Storage& aStorage) : storage{ aStorage }, stream(nullptr) {}
        
     StatusResult operator()(Storage& aStorage, const StorageBlock& aBlock, int aBlockNum) {
         StatusResult theResult{noError};
         switch (aBlock.header.type) {
             case 'T':
                 (*stream) << std::setw(6) << std::left<<aBlockNum
                           << std::setw(8) << std::left << TranslateBack[aBlock.header.type]<<std::endl;
                 break;
             case 'E':
                 (*stream) << std::setw(6) << std::left<<aBlockNum
                           << std::setw(8) << std::left << TranslateBack[aBlock.header.type]
                           << std::setw(20) << std::left << aBlock.header.schemaHash<<std::endl;
                 break;
             case 'D':
                 (*stream) << std::setw(6) << std::left<<aBlockNum
                           << std::setw(8) << std::left << TranslateBack[aBlock.header.type]
                           << std::setw(20) << std::left << aBlock.header.schemaHash
                           <<"id =  " <<aBlock.header.id
                           <<std::endl;
                 break;
            case 'I':
                (*stream) << std::setw(6) << std::left << aBlockNum
                    << std::setw(8) << std::left << TranslateBack[aBlock.header.type]
                    << std::setw(20) << std::left << aBlock.header.schemaHash << std::endl;
                 break;
             case 'F':
                theResult.value = 1;
             default:
                 break;
         }

         return theResult;
         
     }
    
     bool show(std::ostream& anOutput) {
         stream = &anOutput;
         StatusResult theResult = storage.eachBlock(*this);
         
         anOutput << theResult.value << " rows in set"<<std::endl;
         return true;
     }

 protected:
     Storage&      storage;
     std::ostream* stream;
     
 };
  
}

#endif /* Database_hpp */
