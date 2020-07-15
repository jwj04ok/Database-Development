//
//  Row.cpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Row.hpp"

namespace ECE141 {
  
  //STUDENT: You need to fully implement these methods...

  Row::Row() {}

  Row::Row(int aBlockNum):blockNum(aBlockNum),data() {};

  Row::Row(const Row& aRow) :blockNum(aRow.blockNum), data(aRow.data) {};

  Row::Row(const KeyValues& aKVlist) {
      for (auto& pair : aKVlist) {
          data.emplace(pair);
      }
  }

  Row::Row(const KeyValues& aKVlist, int ablockNum) : blockNum(ablockNum) {
      for (auto& pair : aKVlist) {
          data.emplace(pair);
      }
  }

  Row::Row(Row& leftRow, Row& rightRow) {
      for (auto anAttr : leftRow.data) {
          data.emplace(anAttr);
      }
      for (auto anAttr : rightRow.data) {
          data.emplace(anAttr);
      }
  };

  Row::Row(Row& leftRow, Row& rightRow, bool emptyside) {
      switch (emptyside) {
      case 0:
          for (auto anAttr : leftRow.data) {
              data.emplace(anAttr);
          }
          for (auto anAttr : rightRow.data) {
              std::pair<const std::string, ValueType> tempPair(anAttr.first, "NULL");
              data.emplace(tempPair);
          }
          break;
      case 1:
          for (auto anAttr : leftRow.data) {
              std::pair<const std::string, ValueType> tempPair(anAttr.first, "NULL");
              data.emplace(tempPair);
          }
          for (auto anAttr : rightRow.data) {
              data.emplace(anAttr);
          }
          break;
      default:
          break;
      }
      
  };

  Row::~Row() {}

  Row& Row::operator=(const Row &aRow) {return *this;}

  //sort rows in collection 
  RowCollection& RowCollection::order(std::string anOrderKey) {

    std::sort(this->rows.begin(), this->rows.end(), [&](Row* lhs, Row* rhs) {return lhs->data[anOrderKey] <  rhs->data[anOrderKey]; });

    return *this;
}

   RowCollection::RowCollection(const RowCollection& aCopy): rows(aCopy.rows){};

   RowCollection::~RowCollection() {
    for (auto theRow : rows) {
        delete theRow;
    }
   }

   RowCollection &RowCollection::add(Row *aRow) {
        rows.push_back(aRow);
        return *this;
   }

}
