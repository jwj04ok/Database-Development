//
//  Filters.hpp
//  Assignement6
//
//  Created by rick gessner on 5/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//


#include <stdio.h>
#include "Filters.hpp"
#include "Row.hpp"
#include "Schema.hpp"
#include "Helpers.hpp"
#include "keywords.hpp"

namespace ECE141 {

Filters::Filters() {
}

Filters::Filters(const Filters &aCopy) {
    expressions = aCopy.expressions;
}

Filters::~Filters() {
    for (auto theExpr : expressions) {
        delete theExpr;
    }
    expressions.clear();
}

Filters &Filters::add(Expression *anExpression) {
    expressions.push_back(anExpression);
    return *this;
}

//for a keyValues pair, check if it satisfies the expression condition
bool Expression::operator()(KeyValues &aList){
    ValueType left = aList[lhs.name]; //get the corresponding valueType of given key = lhs.name
    ValueType right = rhs.value;
    switch (op) {
        case Operators::equal_op:
            return left == right;
            break;
        case Operators::notequal_op:
            return left != right;
            break;
        case Operators::lt_op:
            return left < right;
            break;
        case Operators::gt_op:
            return right < left;
            break;
        case Operators::lte_op:
            return left <= right;
            break;
        case Operators::gte_op:
            return right <= left;
            break;
        default:
            break;
    }
    
    return false;
}

//check whether keyvalues matches expressions
bool Filters::matches(KeyValues &aList) const {
    if (andOr==1){
        for (auto &theExpr : expressions) {
            if (!(*theExpr)(aList)) {
                return false;
            }
        }
        return true;
    }else{
        for (auto &theExpr : expressions) {
            if ((*theExpr)(aList)) {
                return true;
            }
        }
        return false;
    }
}

bool isValidTokenType(Token &aToken){
    if (TokenType::identifier == aToken.type || TokenType::number == aToken.type || TokenType::string == aToken.type){return true;}
    return false;
}

StatusResult parseLeftOperand(Token &aToken, Schema &aSchema, Operand &anOperand){
    if (std::nullopt != aSchema.getAttribute(aToken.data)){
        anOperand.name = aToken.data;
        anOperand.entityId = aSchema.getHash();
        anOperand.type = TokenType::identifier;
    }else return StatusResult{unknownAttribute};
    return StatusResult{noError};
}

StatusResult Filters:: parseWhereFilters(Tokenizer &aTokenizer, Schema &aSchema){
//    after "Where" keyword, parse the conditions followed after "where"
    StatusResult theResult{noError};
    while (theResult && aTokenizer.more()){
        Token aToken = aTokenizer.current();
        if (isValidTokenType(aToken)){ //identifier, string, number
            theResult = parseExpressions(aTokenizer, aSchema);
        }else if (TokenType::keyword ==  aToken.type && Keywords::and_kw == aToken.keyword){
            aTokenizer.next(); //skip "and"
            theResult = parseExpressions(aTokenizer, aSchema);
        }else if (TokenType::keyword ==  aToken.type && Keywords::or_kw == aToken.keyword){
            andOr = false;
            aTokenizer.next(); //skip "or"
            theResult = parseExpressions(aTokenizer, aSchema);
        }else if (TokenType::semicolon == aToken.type){
            //aTokenizer.next();
            return theResult;
        }
        
    }
    
    
    return theResult;
}

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

StatusResult Filters:: parseExpressions(Tokenizer &aTokenizer, Schema &aSchema){
    StatusResult theResult{syntaxError};
    Token &aToken = aTokenizer.current(); //should be 'identifier'
    Operand lhs;
    if (isValidTokenType(aToken)){
        if(parseLeftOperand(aToken, aSchema, lhs)){
            aTokenizer.next();//skip lhs
            aToken = aTokenizer.current();
            if (TokenType::operators == aToken.type) {
                Operators op = Helpers::toOperator(aToken.data);
                if (Operators::unknown_op != op) {
                    aToken = aTokenizer.peek(1);
                    if (isValidTokenType(aToken)) {
                        Operand rhs;
                        rhs.type = aToken.type;
                        rhs.value = aToken.data;
                        aTokenizer.next(1); //at rhs
                        if (validateOps(lhs, rhs, aSchema)) {
                            expressions.push_back(new Expression(lhs, op, rhs));
                            aTokenizer.next(); //at ";" or "ordered", "limit", "and", "or"
                            return StatusResult{ noError };
                        }
                    }
                }
            }
            
        }
    }

      return theResult;
}

}



