//
//  Index.cpp
//  Assignment 8
//
//  Created by Ke Wang on 20/05/2020.
//  Copyright © 2020 Ke Wang. All rights reserved.
//

#include <stdio.h>
#include "Index.hpp"

namespace ECE141 {

Index& Index::addKeyValue(const ValueType &aKey, uint32_t aValue){
    list.insert(std::pair<ValueType, uint32_t>(aKey, aValue));
    return *this;
}

Index& Index::removeKeyValue(const ValueType &aKey){
    if (contains(aKey)){
        list.erase(aKey);
    }
    return *this;
}

bool Index::contains(const ValueType &aKey){
    for(auto &idx:list){
        if (idx.first == aKey){
            return true;
        }
    }
    return false;
}

uint32_t Index:: getValue(const ValueType &aKey){
    if (contains(aKey)){
        return list[aKey];
    }
    return 0;
}

bool Index:: each(BlockVisitor &aVisitor){
    uint32_t total = list.size();
    StatusResult theResult{noError};
    for (auto&item:list){
        
        StorageBlock aBlock;
        theStorage.readBlock(aBlock, item.second);
        theResult = aVisitor(theStorage, aBlock, item.second);
            
        }return true;
    }

bool Index::canIndexBy(const std::string &aField){
    if (aField == field) return true;
    return false;
}

StatusResult Index::encode(StorageBlock& aBlock) {
    aBlock.header.schemaHash = schemaId;
    std::stringstream    aWriter;
    std::string separator = ";";
    aWriter << field << separator
        << ECE141::Helpers::getStrForDataType(type) << separator
        << schemaId << separator
        << blockNum << separator
        << list.size() <<separator;
    
    for (auto& pair : list) {
        switch (pair.first.index()) {
        case 0:
            aWriter << std::get<uint32_t>(pair.first) << separator;
            break;
        case 1:
            aWriter << std::get<float>(pair.first) << separator;
            break;
        case 2:
            aWriter << std::get<bool>(pair.first) << separator;
            break;
        case 3:
            aWriter << std::get<std::string>(pair.first) << separator;
            break;
        }
        aWriter << pair.second << separator;
    }
    aWriter >> aBlock.data;
    //std::memcpy(theBlock.data, reinterpret_cast<const void*>(&aWriter), sizeof(Schema));
    return StatusResult{};

}


StatusResult Index:: decode(const StorageBlock& aBlock){
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

    field = info[0];
    type = ECE141::Helpers::getDataTypeForString(info[1]);
    schemaId = stoi(info[2]);
    blockNum = stoi(info[3]);
    size_t listSize = stoi(info[4]);
    size_t i;
    size_t curr = 5;
    for (i=0; i < listSize;++i) {
        std::pair<ValueType, uint32_t> tempPair;
        Value aValue(info[curr]);
        aValue.become(type);
//        list.emplace(std::make_pair(aValue.getValue(), stoi(info[curr + 1])), LessKey{});
        list.insert(std::pair<ValueType, uint32_t>(aValue.getValue(), stoi(info[curr + 1])));
        curr += 2;
    }
    return theResult;
}

}
