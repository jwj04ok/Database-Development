//
//  SQLProcessor.hpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef SQLProcessor_hpp
#define SQLProcessor_hpp

#include <stdio.h>
#include "CommandProcessor.hpp"
#include "Tokenizer.hpp"
#include "Schema.hpp"
#include "SQLStatement.hpp"
#include "TableView.hpp"
#include "Timer.hpp"
class Statement;
class Database; //define this later...

namespace ECE141 {

  class SQLProcessor : public CommandProcessor {
  public:
    
    SQLProcessor(CommandProcessor *aNext=nullptr);
    virtual ~SQLProcessor();
    
    virtual Statement*    getStatement(Tokenizer &aTokenizer);
    virtual StatusResult  interpret(const Statement &aStatement);
    
    StatusResult          createTable(Schema *aSchema);
    StatusResult          dropTable(const std::string &aName);
    StatusResult          describeTable(const std::string &aName);
    StatusResult          showTables();
    StatusResult          insert(const Row& aRow, const std::string& aTableName);
    StatusResult          deleteRows(const std::string& aName, const Filters &theFilters);
    StatusResult          selectRows(const std::string &tableName, const Filters &theFilters, const std::string &orderKey, const int &limit, const std::vector<Join> &joins);
    StatusResult          mergeTwoTables(RowCollection &leftRows, RowCollection &rightRows, const Join &join, RowCollection &mergedRows);
    StatusResult          mergeTwoSchemas(Schema& leftSchema, Schema& rightSchema, Schema& mergedSchema);
    StatusResult          updateRows(const std::string& aName, const KeyValues& aKeyValues, const Filters& theFilters);
    StatusResult          showIndexs();

  };




}
#endif /* SQLProcessor_hpp */
