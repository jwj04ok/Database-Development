//
//  Value.hpp
//  RGAssignment5
//
//  Created by rick gessner on 4/26/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Value_h
#define Value_h

#include <variant>
#include <optional>
#include <string>
#include <map>
#include <cstdint>
#include "Errors.hpp"
#include <iostream>

namespace ECE141 {


  struct Storable {
     virtual StatusResult  encode(std::ostream &aWriter)=0;
     virtual StatusResult  decode(std::istream &aReader)=0;
  };

  enum class DataType {
     no_type='N', bool_type='B', datetime_type='D', float_type='F', int_type='I', varchar_type='V',
   };

  static std::map<int, DataType> findType = {
    std::make_pair(0, DataType::int_type),
    std::make_pair(1, DataType::float_type),
    std::make_pair(2, DataType::bool_type),
    std::make_pair(3, DataType::varchar_type),
  };

  using ValueType = std::variant<uint32_t, float, bool, std::string>;
  using KeyValues = std::map<const std::string, ValueType>;

  class Value {
  public:
      Value();
      Value(ValueType aValue); 
      Value(const Value& aCopy);
      Value(std::string aStr);
      Value(const char* aChar);
      Value(DataType aType, std::string aString);

      ~Value();


      DataType          getType() const { return type; }
      ValueType         getValue() const { return value; }
      static ValueType  getDefault(DataType theType);

      Value& operator=(const Value& aCopy);
      Value& operator=(const std::string& aString);
      
      StatusResult become(DataType aType);

  protected:
      DataType type;
      ValueType value;

  };
}

#endif /* Value_h */
