//
//  Schema.cpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Schema.hpp"
#include "Storage.hpp"
#include "Value.hpp"

const int gMultiplier = 37;
const int base=100000;
namespace ECE141 {

uint32_t Schema::hashString(const char *str) {
    
    uint32_t h{0};
    unsigned char *p;
    for (p = (unsigned char *) str; *p != '\0'; p++)
        h = gMultiplier * h + *p;
    h=h/base;
    return h;
}
//-------------------------------------------------------------------
Schema::Schema() {}

Schema::Schema(std::string aName)
           : name(aName), attributes(), changed(false), count(0), blockNum(0), autoInc(0) {
       hash = hashString(aName.c_str());
   }

Schema::Schema(const Schema &aCopy) :
        name(aCopy.name), attributes(aCopy.attributes), changed(aCopy.changed), autoInc(aCopy.autoInc),
        count(aCopy.count), blockNum(aCopy.blockNum), hash(aCopy.hash) {
}

Schema::Schema(StorageBlock &aBlock, uint32_t aBlockId){

    std::stringstream aReader(aBlock.data);
    std::string separator = ";";
    char* currdata = const_cast<char*>(aBlock.data);
    char* tokenPtr = strtok(currdata, ";");
    std::vector<std::string> info;
    while (tokenPtr) {
        info.push_back(tokenPtr);
        tokenPtr = strtok(NULL, ";");
    }

    name = info[0];
    count = stoi(info[1]);
    autoInc = stoi(info[2]);
    blockNum = stoi(info[3]);
    hash = stoi(info[4]);
    indexBlockNum = stoi(info[5]);
    
    size_t size = stoi(info[6]);
    size_t currptr = 7;

    for (int i = 0; i < size; ++i) {
        Attribute tempAttri;
        tempAttri.setName(info[currptr]);
        tempAttri.setType(Helpers::getDataTypeForString(info[currptr+1]));
        tempAttri.setSize(stoi(info[currptr + 2]));
        tempAttri.setAutoIncrement(stoi(info[currptr + 3]));
        tempAttri.setPrimaryKey(stoi(info[currptr + 4]));
        tempAttri.setNullable(stoi(info[currptr + 5]));
        attributes.push_back(tempAttri);
        currptr += 6;
    }

}
Schema::~Schema(){}

//convert a schema to storage block
Schema::operator StorageBlock() {
    StorageBlock theBlock{ ECE141::BlockType::entity_block };
    theBlock.header.schemaHash = hash;
    std::stringstream    aWriter;
    std::string separator = ";";
    aWriter << name <<separator
        << count << separator
        << autoInc << separator
        << blockNum << separator
        << hash << separator
        << indexBlockNum << separator
        <<attributes.size() << separator;

    for (auto& attribute : attributes) {
        aWriter << attribute;
    }
    aWriter >> theBlock.data;
    //std::memcpy(theBlock.data, reinterpret_cast<const void*>(&aWriter), sizeof(Schema));
    return theBlock;
}

//get a specific attribute from schema
AttributeOpt Schema::getAttribute(const std::string &aName)const{
    for (auto theIter = attributes.begin(); theIter != attributes.end(); theIter++) {
        if (theIter->getName() == aName) {
            return *theIter;
        }
    }
    return std::nullopt;
}

std::string Schema::getPrimaryKeyName() const {
    for (auto& attr : attributes) {
        if (attr.isPrimaryKey()) return attr.getName();
    }
    return std::string("id"); 
}

//get what is the currect primary key value
uint32_t Schema::getNextAutoIncrementValue(){
    changed = true;
    return ++autoInc;
}

Schema& Schema::addAttribute(const Attribute &anAttribute){
    if (getAttribute(anAttribute.getName()) == std::nullopt) {
        attributes.push_back(anAttribute);
    }
    return *this;
}

//validata whether attributes type = KVlist type 
StatusResult Schema::validate(KeyValues& aList) {
    for (auto theKV : aList) {
        if (!validateAttribute(theKV.first, theKV.second)) {
            return StatusResult{ invalidAttribute };
        }
    }
    return StatusResult{ noError };
}

bool Schema::validateAttribute(const std::string& name, const Value& aValue) {
    bool comp = false;
    for (auto& attr : attributes) {
        if (attr.getName() == name) {
            comp = (attr.getType() == aValue.getType());
        }
    }
    return comp;
}

//encode a KVlist information into a Block
StatusResult Schema::encodeRows(KeyValues& aKeyValueList, StorageBlock& aBlock) {
    StatusResult theResult{ noError };
    std::fstream aWriter;
    aBlock.header.schemaHash = hashString(name.c_str());
    aBlock.header.id = autoInc;
    std::stringstream output; //test using a simple stringstream...
    std::string separator = ";";

    for (auto& attr : attributes) {
        ValueType aValueType = aKeyValueList[attr.getName()];
        switch (attr.getType()) {
        case DataType::int_type:
            output << std::get<uint32_t>(aValueType);
            break;
        case DataType::bool_type:
            output << std::get<bool>(aValueType);
            break;
        case DataType::varchar_type:
            output << std::get<std::string>(aValueType);
            break;
        case DataType::float_type:
            output << std::get<float>(aValueType);
            break;
        case DataType::datetime_type:
            break;
        default:
            break;
        }
        output << separator;
    }
    std::stringstream another;
    another << output.str();
    another >> aBlock.data;
    //output >> aBlock.data;
    return  theResult;
}

//decode a KVlist from a Blcok
StatusResult Schema::decodeRows(const StorageBlock& aBlock, KeyValues& aList)const {
    StatusResult theResult{ noError };
    StorageBlock theBlock(aBlock);

    std::stringstream anOutput(aBlock.data);
    std::string separator = ";";
    char* currdata = const_cast<char*>(aBlock.data);
    char* tokenPtr = strtok(currdata, ";");
    std::vector<std::string> info;
    while (tokenPtr) {
        info.push_back(tokenPtr);
        tokenPtr = strtok(NULL, ";");
    }

    int i = 0;
    for (auto& attr : attributes) {
        Value aValue(info[i]);
        aValue.become(attr.getType());
        aList.emplace(std::make_pair(attr.getName(), aValue.getValue()));
        i += 1;
    }

    return theResult;
    }


}
