//
//  Row.hpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Row_hpp
#define Row_hpp

#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
//#include "Storage.hpp"
#include <algorithm>
#include "Value.hpp"

namespace ECE141 {


  class Row : public Storable {
  public:

    Row();
    Row(int aBlockNum);
    Row(const Row &aRow);
    Row(const KeyValues& aKVlist);
    Row(const KeyValues& aKVlist, int ablockNum);
    Row(Row& leftRow, Row& rightRow);
    Row(Row& leftRow, Row& rightRow,bool emptyside);//emptyside=0: all attrs in rightRow are NULL
                                                    //emptyside=1: all attrs in leftRow are NULL
    ~Row();

    Row& operator=(const Row &aRow);
    
    int                     getBlockNum() const { return blockNum; }
    const KeyValues&        getColumns() const { return data; }
    KeyValues&              getModifiableColumns()  { return data; }
    virtual StatusResult    encode(std::ostream& aWriter) {return StatusResult();};
    virtual StatusResult    decode(std::istream& aReader) {return StatusResult();};

    friend class Database;
    friend class RowCollection;
      
  protected:
    int       blockNum;
    KeyValues data;  
  };


class RowCollection {
public:
    RowCollection():rows(){};
    RowCollection(const RowCollection& aCopy);

    ~RowCollection() ;

    std::vector<Row*>& getRows() { return rows; }

    RowCollection& add(Row* aRow);
    RowCollection& order(std::string anOrderKey);
protected:
    std::vector<Row*> rows;
};

}

#endif /* Row_hpp */
