//
//  SQLStatement.cpp
//  Datatabase4
//
//  Created by rick gessner on 4/13/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include "SQLStatement.hpp"
#include "SQLProcessor.hpp"
#include "Tokenizer.hpp"
#include "Schema.hpp"
#include "Database.hpp"


namespace ECE141 {

SQLStatement::SQLStatement(SQLProcessor &aProcessor, Keywords aStatementType)
  : Statement(aStatementType), name(""), processor(aProcessor) {}
  
StatusResult SQLStatement::parse(Tokenizer &aTokenizer) {return StatusResult{};}
  
StatusResult SQLStatement::run(std::ostream &aStream) {return StatusResult{noError};}

//-----------------------------------------------------------------------------------------
//Utility function to create table statement parse
  
  bool skipIf(Tokenizer &aTokenizer, Keywords aKeyword) {
    if (aTokenizer.more() && (aKeyword==Helpers::getKeywordId(aTokenizer.current().data))) {
      aTokenizer.next();
      return true;
    }
    return false;
  }
  
  bool skipIf(Tokenizer &aTokenizer, TokenType aTokenType) {
    if(aTokenizer.more() && (aTokenizer.current().type==aTokenType)) {
      aTokenizer.next();
      return true;
    }
    return false;
  }

//parse attribute options like "auto increment", "default value","not Null"...
  StatusResult parseAttributeProperty(Attribute &anAttribute, Tokenizer &aTokenizer) {
    bool options=true;
    StatusResult theResult{noError};
    
    while(theResult && options && aTokenizer.more()) {
      Token &theToken=aTokenizer.current();
      switch(theToken.type) {
        case TokenType::keyword:
          switch(theToken.keyword) {
            case Keywords::default_kw:
                  aTokenizer.next();
                  theToken=aTokenizer.current();
                  anAttribute.setDefault(theToken.data);
                  break;
            case Keywords::auto_increment_kw:
                  anAttribute.setAutoIncrement(true);
                  break;
            case Keywords::primary_kw:
                  anAttribute.setPrimaryKey(true);
                  break;
            case Keywords::not_kw:
                  aTokenizer.next();
                  theToken=aTokenizer.current();
                  if(Keywords::null_kw==theToken.keyword) {
                      anAttribute.setNullable(false);
                    }
                  else theResult.code=syntaxError;
                  break;
            case Keywords::unknown_kw:{
                std::string tmp = theToken.data;
                aTokenizer.next();
                theToken = aTokenizer.current();
                if (TokenType::identifier == theToken.type) {
                    anAttribute.setDefault(theToken.data);
                }
                else theResult.code = syntaxError;
                break;
            }
            default: break;
          }
          break;
          
        case TokenType::rparen:
        case TokenType::comma:
        case TokenType::semicolon:
          options=false;
          break;
          
        default:
          options=false;
          theResult.code=syntaxError;
      }
      aTokenizer.next(); 
    } 
    return theResult;
  }
  
  //handle "varchar(num)"
  StatusResult parseVarcharSize(Attribute &anAttribute, Tokenizer &aTokenizer) {
    Token &theToken=aTokenizer.current();
    if(TokenType::lparen==theToken.type) {
      aTokenizer.next();
      theToken=aTokenizer.current();
      if(TokenType::number==theToken.type) {
        int theSize = atoi(theToken.data.c_str());
        anAttribute.setSize(theSize);
        aTokenizer.next();
        if(skipIf(aTokenizer, TokenType::rparen)) {
          return StatusResult{noError};
        }
      }
    }
    return StatusResult{syntaxError};
  }
  
//parse a single attribue
  StatusResult parseAttribute(Attribute &anAttribute, Tokenizer &aTokenizer) {
    StatusResult theResult{noError};
    
    if(aTokenizer.more()) {
      Token &theToken=aTokenizer.current();
      if(Helpers::isDatatype(theToken.keyword)) {
        DataType theType = Helpers::getDataTypeForKeyword(theToken.keyword);
        anAttribute.setType(theType);
        aTokenizer.next(); //skip
        
        if(DataType::varchar_type==theType) {
          theResult=parseVarcharSize(anAttribute,aTokenizer);
        }
        
        if(theResult) {
          theResult=parseAttributeProperty(anAttribute, aTokenizer);
        }
        
      }
      else theResult.code=invalidAttribute;
    }
    return theResult;
  }
  
//--------------------------------------------------------------------

//------------------ CREATE TABLE  -----------------------------------

  CreateTableStatement::CreateTableStatement(SQLProcessor &aProcessor)
    : SQLStatement(aProcessor, Keywords::create_kw)  {}
 

  StatusResult CreateTableStatement::parseAttributes(Tokenizer &aTokenizer) {
    StatusResult theResult{noError};
    
    while(theResult && aTokenizer.more()) {
      Token &theToken=aTokenizer.current();
      if(TokenType::identifier==theToken.type) {
        aTokenizer.next(); //skip identifier
        Attribute theAttribute(theToken.data, DataType::varchar_type);
        theResult=parseAttribute(theAttribute, aTokenizer);
        if(theResult) {
          attributes.push_back(theAttribute);
        }
      }
      else if(TokenType::semicolon==theToken.type) {
        aTokenizer.next();
        break;
      }
      else theResult.code=syntaxError;
    }
  
    return StatusResult{noError};
  }
  
  
  StatusResult CreateTableStatement::parse(Tokenizer &aTokenizer) {
    if(8<aTokenizer.remaining()) {
      Token &theToken = aTokenizer.peek(1); //"table"
      if(Keywords::table_kw==theToken.keyword) {
        Token &theNextToken = aTokenizer.peek(2); //name
        if(TokenType::identifier==theNextToken.type) {
          aTokenizer.next(3);
          name=theNextToken.data;
          if(skipIf(aTokenizer, TokenType::lparen)) { //('title', 'email', ...)
            return parseAttributes(aTokenizer);
          }
          return StatusResult{noError};
        }
      }
    }
    return StatusResult{Errors::syntaxError};
  }
  

  StatusResult CreateTableStatement::run(std::ostream &aStream) const {
    Schema *theSchema = new Schema(name); //first construct a schema with attributes
    for(auto theAttr : attributes) {
      theSchema->addAttribute(theAttr);
    }
    return processor.createTable(theSchema);
  }

  
  //------------------- Describe table ------------------------------
  
  DescribeStatement::DescribeStatement(SQLProcessor &aProcessor)
  : SQLStatement(aProcessor, Keywords::update_kw)  {}
  
  StatusResult DescribeStatement::parse(Tokenizer &aTokenizer) {
    if(1<aTokenizer.remaining()) {
      Token &theToken = aTokenizer.peek(1); //name
      if(TokenType::identifier==theToken.type) {
        name=theToken.data;
        aTokenizer.next(2);
        return StatusResult{noError};
      }      
    }
    return StatusResult{Errors::unknownCommand};
  }
  
  StatusResult DescribeStatement::run(std::ostream &aStream) const {
    return processor.describeTable(name);
  }
  
  //-------------------- Drop table ------------------------------
  
  DropTableStatement::DropTableStatement(SQLProcessor &aProcessor)
  : SQLStatement(aProcessor, Keywords::update_kw)  {}
  
  StatusResult DropTableStatement::parse(Tokenizer &aTokenizer) {
    if(2 < aTokenizer.remaining()) {
      Token &theToken = aTokenizer.peek(1); //"tables"
      if(Keywords::table_kw==theToken.keyword) {
        theToken = aTokenizer.peek(2);
        if(TokenType::identifier==theToken.type) {
          aTokenizer.next(3); //skip drop table name
          name=theToken.data;
          return StatusResult{noError}; //no error
        }
      }
    }
    return StatusResult{Errors::unknownCommand};
  }
  
  StatusResult DropTableStatement::run(std::ostream &aStream) const {
    return processor.dropTable(name);
  }

  //-------------------- Show table ------------------------------
  
  ShowTablesStatement::ShowTablesStatement(SQLProcessor &aProcessor)
  : SQLStatement(aProcessor, Keywords::update_kw)  {}
  
  StatusResult ShowTablesStatement::parse(Tokenizer &aTokenizer) {
    if(1<aTokenizer.remaining()) {
      Token &theToken = aTokenizer.peek(1); //"tables"
      if(Keywords::tables_kw==theToken.keyword) {
        aTokenizer.next(2); //skip identifier...
        return StatusResult{noError};
      }
    }
    return StatusResult{syntaxError};
  }
  
  StatusResult ShowTablesStatement::run(std::ostream &aStream) const {
    return processor.showTables();
  }
  
  //-------------------- Insert Row ------------------------------
  InsertRowStatement::InsertRowStatement(SQLProcessor& aProcessor):
      SQLStatement(aProcessor, Keywords::insert_kw) {}

  //parse keys of records like "name","id","email"
  StatusResult parsKeysList(Tokenizer& aTokenizer, std::vector<std::string>& keysList) {
      StatusResult theResult = StatusResult{ noError };
      bool moreNames = true;

      while (theResult && aTokenizer.more() && moreNames) {
          aTokenizer.skipIf(apostrophe);
          Token theCurrentToken = aTokenizer.peek(1);
          //can only have identifier(string), comma and rparen
          switch (theCurrentToken.type) {
          case TokenType::identifier:
              keysList.push_back(theCurrentToken.data);
              break;
          case TokenType::comma:
              break;
          case TokenType::rparen:
              moreNames = false;
              break;
          default:
              break;
              return StatusResult{ syntaxError };
          }
          aTokenizer.next(); 
      }
      if (keysList.size() == 0) { return StatusResult{ identifierExpected }; }
      return theResult;
  }

  //parse values of a record
  StatusResult parseValueList(Tokenizer& aTokenizer, std::vector<std::string>& valueList) {
      StatusResult theResult = StatusResult{ noError };
      while (theResult && aTokenizer.more()) {
          aTokenizer.skipIf(left_paren);
          Token& theToken = aTokenizer.current();
          if (TokenType::identifier == theToken.type || TokenType::number == theToken.type) {
              valueList.push_back(theToken.data);
              aTokenizer.next();
              if (!aTokenizer.more()) return StatusResult{syntaxError}; //miss rparen
              aTokenizer.skipIf(comma);
          }
          else if (skipIf(aTokenizer, TokenType::rparen)) {
              break;
          }
          else {
              return StatusResult{ syntaxError };
          }

      }
      return theResult;
  }

//check if the value is compatible with the attribute
StatusResult checkValidType(Attribute anAttr, Value aValue){
    StatusResult theResult{noError};
    if (DataType::varchar_type == anAttr.getType() && anAttr.getSize() > 0){
        size_t size = anAttr.getSize();
        if (size < std::get<std::string>(aValue.getValue()).size()) return StatusResult{invalidArguments};
    }
    
    if (DataType::int_type == anAttr.getType()){
        std::string theValue = std::get<std::string>(aValue.getValue());
        for (int i = 0; theValue[i] != 0; ++i){
            if (!isdigit(theValue[i])) return StatusResult{invalidArguments};
        }
    }
    
    return theResult;
}

//use keyValue list to create row object and add to rows.
  StatusResult createRow(InsertRowStatement& aStatement, std::map<std::string, std::string>& aKV, Schema& aSchema) {
      StatusResult theResult = StatusResult{ noError };
      KeyValues rowData;
      const AttributeList& theAttributes = aSchema.getAttributes();

      for (auto& anAttribute : theAttributes) {
          const  std::string& theName = anAttribute.getName();
          if (aKV.count(theName)) {
              Value theValue(aKV[theName]);
              theResult = checkValidType(anAttribute, theValue);
              if (!theResult) return theResult;
              theValue.become(anAttribute.getType());
              rowData[theName] = theValue.getValue();
          }
          else {
              DataType theType = anAttribute.getType();
              if("Null" == anAttribute.getDefaultValue()){
                  rowData[theName] = Value::getDefault(theType);
               
              }else{
                  Value emptyValue(theType, anAttribute.getDefaultValue());
                  rowData[theName] = emptyValue.getValue();
              }
          } 
  }
      Row aRow(rowData);
      aStatement.getRows().push_back(aRow);
      return theResult;
  }

  StatusResult InsertRowStatement::parseKeyValuePair(Tokenizer& aTokenizer, Schema& aSchema, std::vector<std::string>& keyList) {
      StatusResult theResult = StatusResult{ noError };
      //create a KV list anf then create a Row object
      while (theResult && aTokenizer.more() && (TokenType::lparen == aTokenizer.current().type)) {
          aTokenizer.next();
          std::vector<std::string> valueList;
          theResult = parseValueList(aTokenizer, valueList);
          aTokenizer.skipIf(comma);
          if (theResult) {
              if (keyList.size() == valueList.size()) {
                  std::map<std::string, std::string> aKV;
                  for (int i = 0; i < valueList.size(); i++) {
                      aKV[keyList[i]] = valueList[i];
                  }
                  theResult = createRow(*this, aKV, aSchema);
              }
              else { return StatusResult{ keyValueMismatch }; }

          }
      }
      return theResult;
  }

  StatusResult InsertRowStatement::parse(Tokenizer& aTokenizer) {
      StatusResult theResult = StatusResult{ syntaxError };
      if (2 < aTokenizer.remaining()) {
          Token& theToken = aTokenizer.peek(1); //"into"
          if (Keywords::into_kw == theToken.keyword) {
              theToken = aTokenizer.peek(2);
              if (TokenType::identifier == theToken.type) {
                  name = theToken.data;
                  if (Database* activeDatabase = processor.getActiveDatabase()) {
                      if (Schema* theSchema = activeDatabase->getSchema(name)) {
                          aTokenizer.next(2); //skip "into table_name"
                          Token& nextToken = aTokenizer.peek(1);
                          if (TokenType::lparen == nextToken.type) {
                              aTokenizer.next(); //skip '('
                              std::vector<std::string> keysList; //parse the attribute names
                              if (parsKeysList(aTokenizer, keysList)) {
                                  aTokenizer.next();
                                  Token theNextToken = aTokenizer.current();
                                  if (skipIf(aTokenizer, Keywords::values_kw)) {
                                      theResult = parseKeyValuePair(aTokenizer, *theSchema, keysList);
                                      return theResult;
                                  }
                              }
                              else { return StatusResult{ syntaxError }; } //error in parsing attributes name(keys)
                          }
                      }
                      else { return StatusResult{ unknownTable}; }
                  }
                  else { return StatusResult{noDatabaseSpecified}; }
              }

          }

      }

      return StatusResult{ syntaxError };
  }

  StatusResult InsertRowStatement::run(std::ostream& aStream) const {
      StatusResult theResult{ noError };
      for (auto& theRow : rows) {
          theResult = processor.insert(theRow, name);
          if (!theResult) break;
      }
      return theResult;
  }

  //-------------------- Delete record ------------------------------

  DeleteStatement::DeleteStatement(SQLProcessor& aProcessor)
      : SQLStatement(aProcessor, Keywords::delete_kw) {}

  StatusResult DeleteStatement::parse(Tokenizer& aTokenizer) {
      StatusResult theResult{ noError };
      if (2 < aTokenizer.remaining()) {
          Token& theToken = aTokenizer.peek(1); //should be 'from'...
          if (Keywords::from_kw == theToken.keyword) {
              theToken = aTokenizer.peek(2);
              if (TokenType::identifier == theToken.type) {
                  name = theToken.data; //name of the table

                  if (Schema* theSchema = processor.getActiveDatabase()->getSchema(name)) {
                      aTokenizer.next(3); //skip "delete from tablename"
                      aTokenizer.skipIf(semicolon);
                      if (0 < aTokenizer.remaining()){ //where filters
                          Token &aToken = aTokenizer.current();
                          if (TokenType::keyword == aToken.type && Keywords::where_kw == aToken.keyword){
                              aTokenizer.next(); //skip where
                              theResult = theFilters.parseWhereFilters(aTokenizer, *theSchema);
                          }
                      }else{
                          return StatusResult{noError};
                      }
                    }
                  else return StatusResult{unknownTable};

              }
          }
      }
      return theResult;
  }

  StatusResult DeleteStatement::run(std::ostream& aStream) const {
      return processor.deleteRows(name, theFilters);
  }



//-------------------- Select rows ------------------------------
SelectStatement::SelectStatement (SQLProcessor& aProcessor)
    :SQLStatement(aProcessor, Keywords::select_kw){}

//get what attribues selected (include *)
StatusResult parseAttrNames(Tokenizer& aTokenizer, std::vector<std::string>& nameList){
    StatusResult theResult{noError};
    Token aToken = aTokenizer.current(); //attribute name or "*"
    while (Keywords::from_kw != aToken.keyword){
        if (TokenType::identifier == aToken.type || TokenType::operators == aToken.type){
            std::string attrName = aToken.data;
            if (attrName == "*"){
                //do something?
                nameList.push_back(attrName);
                Token tmpToken = aTokenizer.peek(1);
                if (Keywords::from_kw != tmpToken.keyword) return StatusResult{syntaxError}; //*should followed directly by "from"

            }else nameList.push_back(attrName);
            aTokenizer.next(); //skip over one attribute name or "*"
            aTokenizer.skipIf(comma);
            aToken = aTokenizer.current();
        }
        
    }//while
    return theResult;
}
/*
StatusResult parseLeftOperand(Token &aToken, Schema &aSchema, Operand &anOperand){
    if (std::nullopt != aSchema.getAttribute(aToken.data)){
        anOperand.name = aToken.data;
        anOperand.entityId = aSchema.getHash();
        anOperand.type = TokenType::identifier;
    }else return StatusResult{unknownAttribute};
    return StatusResult{noError};
}

//validate expressions
StatusResult validateOps(Operand &lhs, Operand &rhs, Schema &aSchema){
    if (TokenType::identifier == lhs.type){
        auto anAttrs = aSchema.getAttribute(lhs.name);
        Value aValue(rhs.value);
        aValue.become(anAttrs->getType());
        rhs.value = aValue.getValue();
        return StatusResult{noError};
    }
    return StatusResult{syntaxError};
}

StatusResult SelectStatement::parseWhereFilters(Tokenizer& aTokenizer, Schema& aSchema) {
    StatusResult theResult{ syntaxError };
    Token aToken = aTokenizer.peek(1); //get lhs
    if (TokenType::identifier == aToken.type) {
        Operand lhs;
        if (parseLeftOperand(aToken, aSchema, lhs)) {
            aTokenizer.next(); //at lhs
            aToken = aTokenizer.peek(1);
            if (TokenType::operators == aToken.type) {
                Operators op = Helpers::toOperator(aToken.data);
                if (Operators::unknown_op != op) {
                    aTokenizer.next(); //at operator
                    aToken = aTokenizer.peek(1);
                    if (TokenType::identifier == aToken.type || TokenType::number == aToken.type || TokenType::number == aToken.type) {
                        Operand rhs;
                        rhs.type = aToken.type;
                        rhs.value = aToken.data;
                        aTokenizer.next(2); //skip rhs
                        if (validateOps(lhs, rhs, aSchema)) {
                            theFilters.add(new Expression(lhs, op, rhs));
                            aTokenizer.next(); //at ";" or "ordered", "limit"
                            return StatusResult{ noError };
                        }
                    }
                }
            }
        }
        else theResult.code = unknownAttribute;
    }
    return theResult;
}
*/
StatusResult SelectStatement:: parseLimit(Tokenizer &aTokenizer){
    if (aTokenizer.more()){
        aTokenizer.next(); //skip limit
        Token aToken = aTokenizer.current(); //identifier
        if (TokenType::number == aToken.type){
            limit = std::stoi(aToken.data);
            aTokenizer.next();
            return StatusResult{noError};
        }
    }
    return StatusResult{syntaxError};
}

//check whether order key is in attribute names
bool validateOrderKey(std::string orderKey, Schema &aSchema){
    if (std::nullopt!= aSchema.getAttribute(orderKey)){
        return true;
    }
    return false;
}

StatusResult SelectStatement::parseOrderBy(Tokenizer &aTokenizer, Schema &aSchema){
    if (2 <= aTokenizer.remaining()){
        aTokenizer.next(); //skip "order"
        Token aToken = aTokenizer.current();
        if (TokenType::keyword == aToken.type && Keywords::by_kw == aToken.keyword){
            aTokenizer.next(); //skip "by"
            aToken = aTokenizer.current(); //identifier
            if(TokenType::identifier == aToken.type){
                orderKey = aToken.data;
                aTokenizer.next();//skip identifier
                if (validateOrderKey(orderKey, aSchema)){ 
                    return StatusResult{noError};
                }
                
            }
        }
    }
    return  StatusResult{syntaxError};
}

//deal with "where ...", "order by...", "limit ..."
StatusResult SelectStatement:: parseFiltersCommand(Tokenizer& aTokenizer, Schema &aSchema){
    StatusResult theResult{noError};
    while (aTokenizer.more()){
        if (!aTokenizer.more()) return StatusResult{ noError };
        Token theNextToken = aTokenizer.current(); //where/ordered/limit/;
        switch (theNextToken.type) {
            case TokenType::keyword:
                switch (theNextToken.keyword) {
                    case Keywords::where_kw:
                        aTokenizer.next(); //skip "where"
                        theResult = theFilters.parseWhereFilters(aTokenizer, aSchema);
                        //theResult = parseWhereFilters(aTokenizer, aSchema);
                        if (!theResult) return theResult;
                        break;
                    case Keywords::order_kw:
                        theResult = parseOrderBy(aTokenizer, aSchema);
                        break;
                    case Keywords::limit_kw:
                        theResult = parseLimit(aTokenizer);
                        
                    default:
                        break;
                }
                break;
            case TokenType::semicolon:
                return theResult;
                break;
            default:
                
                break;
        }
        

    }
    return theResult;
}

StatusResult validateAttrbutes(std::vector<std::string>& nameList, Schema aSchema) {
    StatusResult theResult{ noError };

    if (nameList.size() == 1 && nameList[0] == "*") {
        return theResult;
    }

    for (auto& name : nameList) {
        bool tmp = false;
        for (auto& attr : aSchema.getAttributes()) {
            if (name == attr.getName()) {
                tmp = true;
                break;
            }
        }
        if (tmp == false) return StatusResult{ unknownAttribute };
    }
    return theResult;
}

StatusResult parseTableName(Tokenizer &aTokenizer, std::string &aTableName){
    Token &aToken = aTokenizer.current();
    aTableName = aToken.data;
    return StatusResult{noError};
}


StatusResult parseTableField(Tokenizer &aTokenizer, TableField &aTableField){
    StatusResult theResult{noError};
    Token &aToken = aTokenizer.current();
    aTableField.tableName = aToken.data;
    aTokenizer.next(1);
    aToken = aTokenizer.current();
    if (TokenType::operators == aToken.type && "." == aToken.data){
        aToken = aTokenizer.peek(1);
        aTableField.field = aToken.data;
        aTokenizer.next(2);
    }else theResult.code = keywordExpected;
    return theResult;
}

//jointype JOIN tablename ON table1.field=table2.field
StatusResult SelectStatement::parseJoin(Tokenizer &aTokenizer) {
  Token &theToken = aTokenizer.current();
  StatusResult theResult{joinTypeExpected}; //add joinTypeExpected to your errors file if missing...

  Keywords theJoinType{Keywords::join_kw}; //could just be a plain join
  if(in_array<Keywords>(gJoinTypes, theToken.keyword)) {
    theResult.code=noError;
    theJoinType=theToken.keyword;
    aTokenizer.next(1); //yank the 'join-type' token (e.g. left, right)
    if(aTokenizer.skipIf(Keywords::join_kw)) {
      std::string theTable;
      if((theResult=parseTableName(aTokenizer, theTable))) {
          aTokenizer.next(1);
        Join theJoin(theTable, theJoinType);
        theResult.code=keywordExpected; //on...
        if(aTokenizer.skipIf(Keywords::on_kw)) { //LHS field = RHS field
          if((theResult=parseTableField(aTokenizer, theJoin.lhs))) {
            theToken = aTokenizer.current();
            if(TokenType::operators == theToken.type && "=" == theToken.data) {
                aTokenizer.next(1);
              if((theResult=parseTableField(aTokenizer, theJoin.rhs))) {
                  if(theJoin.lhs.tableName == theTable){
                      TableField tmp(theJoin.lhs);
                      theJoin.lhs(theJoin.rhs);
                      theJoin.rhs(tmp);
                  }
                joins.push_back(theJoin);
                theResult.code=noError;
              }
            }
          }
        }
      }
    }
  }
  return theResult;
}

StatusResult SelectStatement:: parse(Tokenizer& aTokenizer){
    StatusResult theResult{syntaxError};
    if (4 <= aTokenizer.remaining()){
        aTokenizer.next(); //skip "select"
        if (parseAttrNames(aTokenizer, chosenAttrs)){
            if ("*" == chosenAttrs[0]) selectAll = true;
            Token theToken = aTokenizer.current();//"from"
            if (Keywords::from_kw == theToken.keyword && aTokenizer.more()){
                theToken = aTokenizer.peek(1);
                if (TokenType::identifier == theToken.type){
                    name = theToken.data; //get table name
                    if(Database *theDatabase = processor.getActiveDatabase()){
                        if (Schema *theSchema = theDatabase->getSchema(name)){
                            theResult = validateAttrbutes(chosenAttrs, *theSchema);
                            if (!theResult) return theResult;
                            theResult.code = noError;
                            aTokenizer.next(2); //skip "from", table name
                            if (aTokenizer.remaining()){
                                Token &aToken = aTokenizer.peek(1);
                                if (in_array<Keywords>(gJoinTypes, aToken.keyword)){
                                    theResult = parseJoin(aTokenizer);
                                }else theResult = parseFiltersCommand(aTokenizer, *theSchema);
                            }
                        }else return StatusResult{unknownTable};
                    }else return StatusResult{unknownDatabase};
                    
                }
            }
        }else return theResult;
    }
    return  theResult;
}

StatusResult SelectStatement:: run(std::ostream& aStream) const{
    return processor.selectRows(name, theFilters, orderKey, limit, joins);
    //return StatusResult{noError};
};

//-------------------- Update rows ------------------------------
UpdateStatement::UpdateStatement (SQLProcessor& aProcessor)
:SQLStatement(aProcessor, Keywords::update_kw){}

StatusResult parseKeyValuePair(Tokenizer &aTokenizer, KeyValues &keyValues){
    StatusResult theResult{noError};
    while (theResult && aTokenizer.more()){
        Token aToken = aTokenizer.current();
        if (TokenType::identifier == aToken.type){
            std::string key = aToken.data;
            aTokenizer.next(); //skip name to update
            aToken = aTokenizer.current();
            if (TokenType::operators == aToken.type && "=" == aToken.data){
                aToken = aTokenizer.peek(1);
                if (TokenType::identifier == aToken.type || TokenType::string == aToken.type){
                    std::string value = aToken.data;
                    keyValues[key] = value;
                    aTokenizer.next(); //skip value
                    aTokenizer.skipIf(comma);
                    aTokenizer.next();
                
                }
            }
        }
        else if (TokenType::keyword == aToken.type){
            break;
        }else theResult.code = syntaxError;
    }
    return theResult;
}

StatusResult UpdateStatement:: parse(Tokenizer& aTokenizer){
    StatusResult theResult{syntaxError};
    if (5 <= aTokenizer.remaining()){
        aTokenizer.next(); //skip "update"
        Token aToken = aTokenizer.current();
        if (TokenType::identifier == aToken.type || TokenType::string == aToken.type){
            name = aToken.data;
            if (Database *theDB = processor.getActiveDatabase()){
                if (Schema *theSchema = theDB->getSchema(name)){
                    aToken = aTokenizer.peek(1);
                    if (Keywords::set_kw == aToken.keyword){
                        aTokenizer.next(2); //skip "set"
                        theResult = parseKeyValuePair(aTokenizer, keyValues);
                        if (theResult){
                            aToken = aTokenizer.current();
                            if(Keywords::where_kw == aToken.keyword){
                                aTokenizer.next(); //skip where
                                theResult = theFilters.parseWhereFilters(aTokenizer, *theSchema);
                            }
                        }
                    }
                }else return StatusResult{unknownTable};
            
            }
            
        }
        
    }
    
    return theResult;
}

StatusResult UpdateStatement::run(std::ostream& aStream)  const {
    return processor.updateRows(name, keyValues, theFilters); 
}

ShowIndexStatement::ShowIndexStatement (SQLProcessor& aProcessor)
:SQLStatement(aProcessor, Keywords::show_kw){}

StatusResult ShowIndexStatement::parse(Tokenizer &aTokenizer){
    StatusResult theResult{noError};
    if (aTokenizer.more()){
        Token theToken = aTokenizer.peek(1);
        if (Keywords::indexes_kw == theToken.keyword){
            aTokenizer.next(2);
            if (aTokenizer.more()) theResult.code = syntaxError;
        }
    }
    
    return theResult;
}

StatusResult ShowIndexStatement::run(std::ostream& aStream) const {
    return processor.showIndexs();
}
}

