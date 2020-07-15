#ifndef DBProcessor_hpp
#define DBProcessor_hpp

#include <stdio.h>
#include "CommandProcessor.hpp"
#include <fstream>
#include "Storage.hpp"
#include <vector>
#include <set>
#include "FolderReader.hpp"
#include "FolderView.hpp"
#include <regex>
#include "Database.hpp"
#include "keywords.hpp"
#include "Tokenizer.hpp"



namespace ECE141 {


    class DBCmdProcessor : public CommandProcessor {
    public:
        virtual Database*       getActiveDatabase(){return activeDB;};

        DBCmdProcessor(CommandProcessor* aNext = nullptr);
        virtual ~DBCmdProcessor();

        virtual Statement*      getStatement(Tokenizer& aTokenizer);
        virtual StatusResult    interpret(const Statement& aStatement);

        Database*               loadDatabase(const std::string& aName) const;
        DBCmdProcessor&         inactivateDB();

        StatusResult            createDatabase(const std::string& aName);
        StatusResult            dropDatabase(const std::string& aName);
        StatusResult            useDatabase(const std::string& aName);
        StatusResult            describeDatabase(const std::string& aName);
        StatusResult            showDatabase();

    protected:
        Database* activeDB;
    };

    
    
}

#endif /* DBProcessor_hpp */#pragma once
