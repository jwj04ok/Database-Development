#include "Value.hpp"

namespace ECE141 {


//------------------------------------------------------------
//--------------------------VALUE-----------------------------
    Value::Value() {
        type = DataType::no_type;
        value = "";
    }

    Value::Value(ValueType aValue) {
        type = findType[aValue.index()];
        value = aValue;
    };

    Value::Value(const Value& aCopy) {
        type = aCopy.type;
        value = aCopy.value;
    };

    Value::Value(std::string aStr) {
        type = DataType::varchar_type;
        value = aStr;
    }

    Value::Value(const char* aChar) { 
        type = DataType::varchar_type;
        value = aChar;
    }

    Value::Value(DataType aType, std::string aString){
        value=aString;
        switch (aType) {
        case DataType::int_type: value = (uint32_t)std::stoi(std::get<std::string>(value));
            type = DataType::int_type;
            break;

        case DataType::float_type:value = std::stof(std::get<std::string>(value));
            type = DataType::float_type;
            break;
        case DataType::datetime_type:        //not deal with datatime yet
            break;
        case DataType::bool_type:
                type = DataType::bool_type;
                if ("true" == aString || "TRUE" == aString||"True" == aString||"1" == aString){
                    value = true;
                }else value = false;
            break;
        case DataType::varchar_type:
            type = DataType::varchar_type;
            break;
        default:
            break;
        }
    }

    Value::~Value() {};

    Value& Value::operator=(const Value& other) {
        type = other.type;
        value = other.value;
        return *this;
    }

    Value& Value::operator=(const std::string& aString) {
        type = DataType::varchar_type;
        value = aString;
        return *this;
    }

StatusResult Value::become(DataType aType) {
        StatusResult theResult{ noError };

        if (type == aType) {
            return theResult;
        }

        try {
            switch (aType) {
            case DataType::int_type: value = (uint32_t)std::stoi(std::get<std::string>(value));
                type = DataType::int_type;
                break;

            case DataType::float_type:value = std::stof(std::get<std::string>(value));
                type = DataType::float_type;
                break;
            case DataType::datetime_type: //not deal with yet
                break;
            case DataType::bool_type:
                    if (std::get<std::string>(value) == "true"|| std::get<std::string>(value) == "TRUE"
                        || std::get<std::string>(value) == "True"|| std::get<std::string>(value) == "1"){
                        value = true;
                    }else{
                        value = false;
                    }
                type = DataType::bool_type;
                break;
            case DataType::varchar_type:
                type = DataType::varchar_type;
                break;
            case DataType::no_type:theResult.code = Errors::unknownType;
                break;
            default:theResult.code = Errors::invalidArguments;
                break;
            }
        }
        catch (...) {
            theResult.code = Errors::invalidArguments;
        }

        return theResult;
    }

    //get default value when not set by users
    ValueType Value::getDefault(DataType theType) {
        switch (theType) {
            case DataType::int_type: return ValueType((uint32_t) 0);

            case DataType::float_type: return ValueType((float) 0);

            case DataType::varchar_type: return ValueType("");

//            case DataType::datetime_type: return ValueType((unsigned int) std::time(nullptr));

            case DataType::bool_type: return ValueType(false);

            default:return ValueType(true);
        }
    }

}

