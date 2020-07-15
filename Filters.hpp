//
//  Filters.hpp
//  RGAssignment6
//
//  Created by rick gessner on 5/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Filters_h
#define Filters_h

#include <stdio.h>
#include <vector>
#include <string>
#include "Errors.hpp"
#include "Value.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {
  
  class Row;
  class Schema;
  
  struct Operand {
    Operand() {}
    Operand(std::string &aName, TokenType aType, ValueType &aValue, uint32_t anId=0)
      : name(aName), type(aType), entityId(anId), value(aValue) {}
    
    TokenType   type; //so we know if it's a field, a const (number, string)...
    std::string name; //for named attr. in schema
    ValueType   value;
    uint32_t    entityId;
  };
  
  //---------------------------------------------------

  struct Expression {
    Operand     lhs;
    Operand     rhs;
    Operators   op;
    
    Expression(Operand &aLHSOperand, Operators anOp, Operand &aRHSOperand)
      : lhs(aLHSOperand), op(anOp), rhs(aRHSOperand) {}
    
    bool operator()(KeyValues &aList);
  };
  
  //---------------------------------------------------

  using Expressions = std::vector<Expression*>;

  //---------------------------------------------------

  class Filters {
  public:
    
    Filters();
    Filters(const Filters &aFilters);
    ~Filters();
    
    size_t        getCount() const {return expressions.size();}
    bool          matches(KeyValues &aList) const;
    Filters&      add(Expression *anExpression);
    StatusResult  parseWhereFilters(Tokenizer &aTokenizer, Schema &aSchema);
    StatusResult  parseExpressions(Tokenizer &aTokenizer, Schema &aSchema);
    friend class Tokenizer;
    
  protected:
    Expressions  expressions;
    bool         andOr = true; //true: and; false: or
  };
   
}

#endif /* Filters_h */

