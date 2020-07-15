#include "DBProcessor.hpp"
#include "Tokenizer.hpp"
#include <memory>
#include "DBStatement.hpp"

namespace ECE141 {
  


  DBCmdProcessor::DBCmdProcessor(CommandProcessor *aNext) : CommandProcessor(aNext) {
      activeDB = nullptr;
  }
  
  DBCmdProcessor::~DBCmdProcessor() {}
  
  // USE: -----------------------------------------------------

  StatusResult DBCmdProcessor::interpret(const Statement& aStatement) {
      return aStatement.run(std::cout);
  }

  // USE: factory to create statement based on given tokens...
    Statement* DBCmdProcessor::getStatement(Tokenizer & aTokenizer) {
        Statement* theResult = nullptr;
        if (aTokenizer.size()) {
            Token theToken = aTokenizer.current();
            switch (theToken.keyword) {
            case Keywords::create_kw: theResult = new createDBStatement(*this);
                break;
            case Keywords::drop_kw: theResult = new dropDBStatement(*this);
                break;
            case Keywords::show_kw: theResult = new showDBStatament(*this);
                break;
            case Keywords::describe_kw: theResult = new describeDBStatement(*this);
                break;
            case Keywords::use_kw: theResult = new useDBStatement(*this);
                break;
            default: break;
            }
            if (theResult) {
                StatusResult theError = theResult->parse(aTokenizer);
                if (!theError) {
                    delete theResult;
                    theResult = nullptr;
                }
            }
        }
        return theResult;
  }
  
    Database* DBCmdProcessor::loadDatabase(const std::string& aName) const {
        Database* theResult = nullptr;
        const char* path = StorageInfo::getDefaultStoragePath();
        if (FolderReader* theReader=new FolderReader(path)) {
            if (theReader->exists(Storage::getPath(aName))) {
                theResult = new Database(aName, OpenExistingStorage{});
            }
            delete theReader;
        }
        return theResult;
    }

    DBCmdProcessor& DBCmdProcessor:: inactivateDB(){
        if(activeDB){
            StatusResult theResult{ noError };
            theResult = activeDB->saveToc();
            delete activeDB;
            activeDB = nullptr;
        }
        return *this;
    }

    StatusResult DBCmdProcessor::createDatabase(const std::string& aName) {
        inactivateDB();
        activeDB = new Database(aName, CreateNewStorage{});
        std::cout << "created database " << aName << std::endl;
        inactivateDB();
        return StatusResult{ noError };
    }

    StatusResult DBCmdProcessor::dropDatabase(const std::string& aName) {
        inactivateDB();
        std::string DBName = Storage::getPath(aName);
        if (std::remove(DBName.c_str()) == 0) {
            std::cout << "dropped database " + aName + " (ok) \n";
        }
        return StatusResult();
    }

    StatusResult DBCmdProcessor::useDatabase(const std::string& aName) {
        inactivateDB();
        if (Database* loadedDB = loadDatabase(aName)) {
            activeDB = loadedDB;
            std::cout << "using database " << aName << std::endl;
            return StatusResult{ noError };
        }
        return StatusResult{ unknownDatabase };
    }

    StatusResult DBCmdProcessor::describeDatabase(const std::string& aName) {
        std::cout << "Blk#  Type    Other" << std::endl;
        std::cout << "----------------------------" << std::endl;
        if (Database* theDB = loadDatabase(aName)) {
            theDB->describe(std::cout);
            delete theDB;
            return StatusResult{ noError };
        }
        return StatusResult{unknownCommand};
    }

    StatusResult DBCmdProcessor::showDatabase() {
        FolderReader theReader(StorageInfo::getDefaultStoragePath());
        FolderView   theView(theReader, ".db");
        theView.show(std::cout);
        return StatusResult();
    }

}
