//
//  CommandProcessor.cpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/18.
//  Copyright © 2018 rick gessner. All rights reserved.
//

#include <iostream>
#include "AppProcessor.hpp"
#include "Tokenizer.hpp"
#include <memory>

namespace ECE141 {


  //.....................................

  AppCmdProcessor::AppCmdProcessor(CommandProcessor *aNext) : CommandProcessor(aNext) {
  }
  
  AppCmdProcessor::~AppCmdProcessor() {}
  
  StatusResult AppCmdProcessor::interpret(const Statement &aStatement) {
    //STUDENT: write code related to given statement
      StatusResult theResult = aStatement.run(std::cout);
      return theResult;
  }
  
  // USE: factory to create statement based on given tokens...
  Statement* AppCmdProcessor::getStatement(Tokenizer &aTokenizer) {
    //STUDENT: Analyze tokens in tokenizer, see if they match one of the
    //         statements you are supposed to handle. If so, create a
    //         statement object of that type on heap and return it.
    
    //         If you recognize the tokens, consider using a factory
    //         to construct a custom statement object subclass.
      if (0==aTokenizer.size()) { return nullptr; }
      switch (aTokenizer.tokenAt(0).keyword) {
        case Keywords::help_kw:{
          if (1==aTokenizer.size()) {
              aTokenizer.next();
              return  new HelpStatement(Keywords::help_kw);
          }
          else if (2<aTokenizer.size()){
              while (aTokenizer.next()){}
              return new HelpStatement(Keywords::help_kw, Errors::syntaxError, 140);
          }
          else { 
              aTokenizer.next();
              aTokenizer.next();
              return new HelpStatement(Keywords::help_kw, aTokenizer.tokenAt(1).keyword);
          }
          break;
        }
      case Keywords::version_kw:{
          if(1==aTokenizer.size()){
              aTokenizer.next();
              return  new VersionStatement();
          }else if (1 < aTokenizer.size()){
              while (aTokenizer.next()){}
              return new VersionStatement(Keywords::version_kw, Errors::syntaxError, 140);
          }
          break;
      }
      case Keywords::quit_kw :{
        if (1==aTokenizer.size()){
            aTokenizer.next();
            return  new QuitStatement();
        }else{
            while (aTokenizer.next()){}
            return new QuitStatement(Keywords::quit_kw, Errors::syntaxError, 140);
            }
        break;
        }
      default: return nullptr;
  }
      return nullptr;
}
}
