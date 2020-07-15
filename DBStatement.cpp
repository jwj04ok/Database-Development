
#include "DBStatement.hpp"

namespace ECE141 {
    bool createDBStatement::dataBaseExisted(const std::string& aName) const {
        const char* address = StorageInfo::getDefaultStoragePath();
        std::string newaddress = address;
        for (auto& theItem : fs::directory_iterator(address)) {
            if (!theItem.is_directory()) {
                fs::path temp(theItem.path());
                std::string theName(temp.stem().generic_u8string());
                if (theName.compare(name) == 0) {
                    return true;
                }
            }
        }
        return false;
    }

    bool createDBStatement::validName(const std::string& aName) const {
        std::string pattern{ "[a-zA-Z][^\\s]*" };
        std::regex re(pattern);
        if (!regex_match(aName, re)) {
            std::cout << "Invalid Name" << std::endl;
            return false;
        }
        return true;
    }
}
