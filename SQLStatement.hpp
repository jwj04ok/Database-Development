//
//  SQLStatement.hpp
//  Datatabase4
//
//  Created by rick gessner on 4/13/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#ifndef SQLStatement_hpp
#define SQLStatement_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include "Statement.hpp"
#include "Attribute.hpp"
#include "Schema.hpp"
#include "Helpers.hpp"
#include "Database.hpp"
#include "Row.hpp"
#include "Value.hpp"

namespace ECE141 {

  class SQLProcessor;
  
  //----------------------------------

  class SQLStatement: public Statement {
  public:
    SQLStatement(SQLProcessor &aProcessor, Keywords aStatementType=Keywords::unknown_kw);
    virtual StatusResult  parse(Tokenizer &aTokenizer);
    virtual StatusResult  run(std::ostream &anOutput);
    
    std::string     name;
    SQLProcessor&   processor;
  };

  //----------------------------------
  
  
  class CreateTableStatement: public SQLStatement {
  public:

    CreateTableStatement(SQLProcessor &aProcessor);
    StatusResult parseAttributes(Tokenizer &aTokenizer);
    StatusResult parse(Tokenizer &aTokenizer);
    
    StatusResult run(std::ostream &aStream) const;
    std::vector<Attribute>  attributes;
  };

  //----------------------------------
  
  class DescribeStatement: public SQLStatement {
  public:

    DescribeStatement(SQLProcessor &aProcessor);
    
    StatusResult parse(Tokenizer &aTokenizer);
    StatusResult run(std::ostream &aStream) const;    
  };

 
  //----------------------------------
  
  class DropTableStatement: public SQLStatement {
  public:
    
    DropTableStatement(SQLProcessor &aProcessor);
    
    StatusResult parse(Tokenizer &aTokenizer);
    StatusResult run(std::ostream &aStream) const;

  };

    //----------------------------------
  

  class ShowTablesStatement: public SQLStatement {
  public:
    
    ShowTablesStatement(SQLProcessor &aProcessor);
    
    StatusResult parse(Tokenizer &aTokenizer);
    StatusResult run(std::ostream &aStream) const;    
  };

  class InsertRowStatement : public SQLStatement {
  public:

    InsertRowStatement(SQLProcessor& aProcessor);

    StatusResult parse(Tokenizer& aTokenizer);
    StatusResult run(std::ostream& aStream) const;

    StatusResult parseKeyValuePair(Tokenizer& aTokenizer, Schema& aSchema, std::vector<std::string>& keyList);

    std::vector<Row> &getRows() { return rows; }
    
  protected:
    std::vector<Row>  rows;
  };

  class DeleteStatement : public SQLStatement {
  public:

      DeleteStatement(SQLProcessor& aProcessor);

      StatusResult parse(Tokenizer& aTokenizer);
      StatusResult run(std::ostream& aStream) const;
  protected:
      Filters     theFilters;
  };

struct TableField{
    TableField():tableName(""), field(""){};
//    TableField(const std::string):tableName(""), field(""){};
    TableField(const TableField &aTableField):tableName(aTableField.tableName), field(aTableField.field){};
    TableField &operator()(TableField &aTableField){
        tableName = aTableField.tableName;
        field = aTableField.field;
        return *this;
    };
    std::string tableName;
    std::string field;
};


struct Join  {
//  Join(const std::string &aTable, Keywords aType, const std::string &aLHS, const std::string &aRHS)
//    : table(aTable), joinType(aType), lhs(aLHS), rhs(aRHS) {}
  Join(const std::string& aTable, Keywords aType) : table(aTable), joinType(aType) {};
  Join(const Join &aJoin) :joinType(aJoin.joinType),table(aJoin.table),lhs(aJoin.lhs),rhs(aJoin.rhs){};

  Keywords    joinType;
  std::string table;
  TableField  lhs;
  TableField  rhs;
};

class SelectStatement: public SQLStatement{
public:
    SelectStatement(SQLProcessor& aProcessor);

    StatusResult parse(Tokenizer& aTokenizer);
    StatusResult parseFiltersCommand(Tokenizer& aTokenizer, Schema &aSchema);
    StatusResult parseWhereFilters(Tokenizer &aTokenizer, Schema &aSchema);
    StatusResult parseLimit(Tokenizer &aTokenizer);
    StatusResult parseOrderBy(Tokenizer &aTokenizer, Schema &aSchema);
    StatusResult parseJoin(Tokenizer &aTokenizer);
    StatusResult run(std::ostream& aStream) const;
protected:
    std::vector<std::string> chosenAttrs;
    Filters                  theFilters;
    int                      limit = -1;
    std::string              orderKey = "";
    bool                     selectAll = false;
    std::vector<Join>        joins;
};

class UpdateStatement: public SQLStatement{
public:
    UpdateStatement(SQLProcessor &aProcessor);
    StatusResult parse(Tokenizer& aTokenizer);
    StatusResult run(std::ostream& aStream) const;
protected:
    bool        updateAll = true;
    KeyValues   keyValues;
    Filters     theFilters;
};
  
class ShowIndexStatement: public SQLStatement{
public:
    ShowIndexStatement(SQLProcessor &aProcessor);
    StatusResult parse(Tokenizer& aTokenizer);
    StatusResult run(std::ostream& aStream) const;
};

}

#endif /* SQLStatement_hpp */
