//
//  Attribute.cpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Attribute.hpp"
#include "Storage.hpp"


namespace ECE141 {

 Attribute::Attribute(std::string aName, DataType aType, uint32_t aSize, bool autoIncr, bool aPrimary) :
            name(aName), type(aType), size(aSize) {
        autoIncrement = autoIncr;
        primary = aPrimary;
        nullable = true;
    }

Attribute::Attribute(DataType aType) : type(aType) {
    autoIncrement = primary = false;
    nullable = true;
}

Attribute::Attribute(const Attribute &aCopy) : name(aCopy.name), type(aCopy.type), size(aCopy.size) {
    autoIncrement = aCopy.autoIncrement;
    nullable = aCopy.nullable;
    primary = aCopy.primary;
    defaultValue = aCopy.defaultValue;
}

Attribute::~Attribute() {
}

Attribute &Attribute::setName(std::string &aName) {
    name = aName;
    return *this;
}

Attribute &Attribute::setType(DataType aType) {
    type = aType;
    return *this;
}

Attribute &Attribute::setSize(int aSize) {
    size = aSize;
    return *this;
}

Attribute &Attribute::setAutoIncrement(bool anAuto) {
    autoIncrement = anAuto;
    return *this;
}

Attribute &Attribute::setPrimaryKey(bool aPrime) {
    primary = aPrime;
    return *this;
}

Attribute &Attribute::setNullable(bool aNullable) {
    nullable = aNullable;
    return *this;
}

Attribute& Attribute::setDefault(std::string aDefaultValue) {
    defaultValue = aDefaultValue;
    return *this;
}

size_t Attribute::getSize() const {
    switch (type) {
        case DataType::datetime_type:
        case DataType::int_type:return sizeof(int32_t);
        case DataType::float_type:return sizeof(float);
        case DataType::varchar_type:return size;
        default: break;
    }
    return 0;
}

//Output an Attribute directly to a stream
std::ostream &operator <<(std::ostream &aStream, Attribute &anAttribue){
    std::string separator = ";";
    aStream << anAttribue.name << separator;
    aStream << static_cast<char> (anAttribue.type) << separator;
    aStream << static_cast<uint32_t> (anAttribue.size) << separator;
    aStream << static_cast<bool>(anAttribue.autoIncrement) << separator;
    aStream << static_cast<bool> (anAttribue.primary) << separator;
    aStream << static_cast<bool>(anAttribue.nullable) << separator;
    return aStream;
}

std::ostream &operator >>(std::iostream &aStream, Attribute &anAttribue){
    
    std::string name;
    char type;
    uint32_t size;
    uint32_t autoIncre;
    bool primary;
    bool nullible;
    
    aStream >> name >> type >> size >> autoIncre >> primary >> nullible;
    anAttribue.setName(name);
    anAttribue.setSize(size);
    anAttribue.setAutoIncrement(autoIncre);
    anAttribue.setPrimaryKey(primary);
    anAttribue.setNullable(nullible);
    
    anAttribue.setType(DataType{type});

    return aStream;
}

}
