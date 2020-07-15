//
//  Statement.cpp
//  Database
//
//  Created by rick gessner on 3/20/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include "Statement.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {
  
  Statement::Statement(Keywords aStatementType) : stmtType(aStatementType)  {
  }
  
  Statement::Statement(const Statement &aCopy) : stmtType(aCopy.stmtType) {
  }
  
  Statement::~Statement() {
  }
  
  //USE: -------------------
  StatusResult Statement::parse(Tokenizer &aTokenizer) {
    return StatusResult{};
  }
  
  StatusResult Statement::run(std::ostream &aStream) const {
    //return commandable.runStatement(*this);
    return StatusResult();
  }

StatusResult HelpStatement::run(std::ostream &aStream)const{
    if (errorValue == 100){
        if (hasContent){
            int count = 0;
            for (auto item: AppCmdDictionary){
                if (item.first == content){
                    std::cout <<"     -- " << item.second<<std::endl;
                    count = 1;
                    break;
                }
            }
            if (count == 0){
                return StatusResult(Errors::syntaxError, 140);
            }
        }else{
            std::cout << "help -- the available list of commands shown below:" << std::endl;
            for (auto item : AppCmdDictionary) {
                std::cout << "     -- " << item.second<<std::endl;
        }
        }
    }else{
        return StatusResult(code, errorValue);
    }
    return StatusResult();
}

StatusResult VersionStatement::run(std::ostream &aStream) const{
    if (errorValue == 100){
        std::cout << "version ECE141b-1" << std::endl;
            return StatusResult();
    }else{
        return StatusResult(code, errorValue);
    }
    }

StatusResult QuitStatement::run(std::ostream &aStream) const{
    if (errorValue == 100){
        exit(0);
        return StatusResult();
    }
    else{
        return StatusResult(code, errorValue);
    }
    
}
}
