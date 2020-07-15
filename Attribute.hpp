//
//  Attribute.hpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "keywords.hpp"
#include "Value.hpp"

namespace ECE141 {
  

  class Attribute {
      
  protected:
    std::string   name;
    DataType      type;
    uint32_t      size : 24;
    uint32_t      autoIncrement : 1;
    uint32_t      primary : 1;
    uint32_t      nullable: 1;
    std::string   defaultValue = "Null";
    
    
  public:
          
    Attribute(DataType aType=DataType::no_type);
    Attribute(std::string aName, DataType aType, uint32_t aSize = 0, bool autoIncr = false, bool primary = false);
    Attribute(const Attribute &aCopy);
    ~Attribute();
    
    Attribute&          setName(std::string &aName);
    Attribute&          setType(DataType aType);
    Attribute&          setSize(int aSize);
    Attribute&          setAutoIncrement(bool anAuto);
    Attribute&          setPrimaryKey(bool aPrime);
    Attribute&          setNullable(bool aNullable);
    Attribute&          setDefault(std::string aDefualtValue);

    bool                isPrimaryKey() const { return primary; }
    bool                isNullable() const { return nullable; }
    bool                isAutoIncrement() const { return autoIncrement; }
    bool                isValid() { return true; }
    
    const std::string&  getName() const {return name;}
    DataType            getType() const {return type;}
    std::string         getDefaultValue() const {return defaultValue;}
    size_t              getSize() const;  

    friend std::ostream &operator <<(std::ostream &aWriter, Attribute &anAttribue);
    friend std::ostream &operator >>(std::iostream &aReader, Attribute &anAttribue);
    
  };
  

}


#endif /* Attribute_hpp */
