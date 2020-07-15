//
//  Statement.hpp
//  Database
//
//  Created by rick gessner on 3/20/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#ifndef Statement_hpp
#define Statement_hpp

#include "keywords.hpp"
#include <iostream>
#include <vector>

namespace ECE141 {
    static std::vector<std::pair<Keywords, std::string>> AppCmdDictionary = {
      std::make_pair(Keywords::help_kw,"help - shows this list of commands"),
      std::make_pair(Keywords::version_kw,"version -- shows the current version of this application"),
      std::make_pair(Keywords::quit_kw,"quit  -- terminates the execution of this DB application"),
      std::make_pair(Keywords::create_kw,"create database <name> -- creates a new database"),
      std::make_pair(Keywords::drop_kw,"drop database <name> -- drops the given database"),
      std::make_pair(Keywords::use_kw,"use database <name>  -- uses the given database"),
      std::make_pair(Keywords::describe_kw,"describe database <name>  -- describes the given database"),
      std::make_pair(Keywords::show_kw,"show databases   -- shows the list of databases available"),
      std::make_pair(Keywords::select_kw,"select - select everything after what is following"),
    };
  
  class Tokenizer;
  
  class Statement {
  public:
    Statement(Keywords aStatementType=Keywords::unknown_kw);
    Statement(const Statement &aCopy);
    
    virtual                 ~Statement();
    
    virtual   StatusResult  parse(Tokenizer &aTokenizer);
    
    Keywords                getType() const {return stmtType;}
    virtual   const char*   getStatementName() const {return "statement";}
    virtual   StatusResult  run(std::ostream &aStream) const;
    
  protected:
    Keywords   stmtType;    
  };

  class HelpStatement:public Statement {
  public:
      HelpStatement(Keywords aKeyword = Keywords::help_kw, Errors errorCode = Errors::keyExpected, uint32_t eValue = 100) :Statement(aKeyword) {
          code = errorCode;
          errorValue = eValue;
          hasContent = 0;
      }
      
      HelpStatement(Keywords aKeyword, Keywords contentKeyword, Errors errorCode = Errors::keyExpected, uint32_t eValue = 100):Statement(aKeyword){
          content = contentKeyword;
          code = errorCode;
          errorValue = eValue;
          hasContent = 1;
      }
      
      Keywords getContent() { return content; }
      StatusResult run(std::ostream &aStream) const;
      
  protected:
      Keywords content;
      Errors   code;
      uint32_t errorValue;
      bool     hasContent;
  };

  class VersionStatement :public Statement {
  public:
      VersionStatement(Keywords aKeyword = Keywords::version_kw, Errors errorCode = Errors::keyExpected, uint32_t eValue = 100) :Statement(Keywords::version_kw) {
          code = errorCode;
          errorValue = eValue;
      }
      
      StatusResult run(std::ostream &aStream) const;
      Keywords getContent() { return content; }
  
  protected:
      Keywords content=Keywords::version_kw;
      Errors   code;
      uint32_t errorValue;
  };

  class QuitStatement :public Statement {
  public:
      QuitStatement(Keywords aKeyword = Keywords::quit_kw, Errors errorCode = Errors::keyExpected, uint32_t eValue = 100) :Statement(Keywords::quit_kw){
          code = errorCode;
          errorValue = eValue;
      }
      StatusResult run(std::ostream &aStream) const;
      Keywords getContent() { return content; }
  protected:
      Keywords content = Keywords::quit_kw;
      Errors   code;
      uint32_t errorValue;
  };

}

#endif /* Statement_hpp */
