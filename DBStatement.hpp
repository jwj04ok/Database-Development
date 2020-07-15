#pragma once
#ifndef DBStatement_hpp
#define DBStatement_hpp

#include "DBProcessor.hpp"

namespace ECE141 {

    //intermediate Statement class
    class DBStatement : public Statement {
    public:
        DBStatement(DBCmdProcessor& aDBCmdProcrssor, Keywords aType = Keywords::unknown_kw) 
            : Statement(aType), name(""), processor(aDBCmdProcrssor) {}

        virtual StatusResult parse(Tokenizer& aTokenizer) { return StatusResult{}; };
        virtual StatusResult run(std::ostream& aStream) const { return StatusResult{ noError }; };

        std::string     name;
        DBCmdProcessor& processor;
    };


    //------------------------------------------------------------------------------------------------
    class createDBStatement : public DBStatement {
    public:
        createDBStatement(DBCmdProcessor& aProcessor) :DBStatement(aProcessor, Keywords::create_kw) {}

        StatusResult parse(Tokenizer& aTokenizer) {
            if (aTokenizer.size() > 3) {
                while (aTokenizer.next(1)) {}
                return StatusResult{ illegalIdentifier };
            }
            aTokenizer.next(1);
            if (aTokenizer.tokenAt(1).keyword == Keywords::database_kw) {
                aTokenizer.next(1);
                if (aTokenizer.tokenAt(2).type == TokenType::identifier) {
                    name = aTokenizer.tokenAt(2).data;
                    aTokenizer.next(1);
                    return StatusResult{ noError };
                }
            }
            return StatusResult{ syntaxError };
        }

        virtual StatusResult run(std::ostream& aStream) const {
            if (!validName(name)) { return StatusResult{ illegalIdentifier }; }
            if (dataBaseExisted(name)) { return StatusResult{ databaseExists }; }
            return processor.createDatabase(name);
        }

    private:
        bool validName(const std::string& aName) const;
        bool dataBaseExisted(const std::string& aName) const;
    };

    //------------------------------------------------------------------------------------------------
    class dropDBStatement : public DBStatement {
    public:
        dropDBStatement(DBCmdProcessor& aProcessor) :DBStatement(aProcessor, Keywords::drop_kw) {}

        StatusResult parse(Tokenizer& aTokenizer) {
            if (aTokenizer.size() != 3) {
                while (aTokenizer.next(1)) {}
                return StatusResult{ syntaxError };
            }
            aTokenizer.next(1);
            if (aTokenizer.tokenAt(1).keyword == Keywords::database_kw) {
                aTokenizer.next(1);
                if (aTokenizer.tokenAt(2).type == TokenType::identifier) {
                    name = aTokenizer.tokenAt(2).data;
                    aTokenizer.next(1);
                    return StatusResult{ noError };
                }
            }
            return StatusResult{ syntaxError };
        }
        StatusResult run(std::ostream& aStream) const {
            return processor.dropDatabase(name);
        }
    };

    //------------------------------------------------------------------------------------------------
    class useDBStatement :public DBStatement {
    public:
        useDBStatement(DBCmdProcessor& aProcessor) :DBStatement(aProcessor, Keywords::use_kw) {}

        StatusResult  parse(Tokenizer& aTokenizer) {
            if (aTokenizer.size() != 3) {
                while (aTokenizer.next(1)) {}
                return StatusResult{ syntaxError };
            }
            aTokenizer.next(1);
            if (aTokenizer.tokenAt(1).keyword == Keywords::database_kw) {
                aTokenizer.next(1);
                if (aTokenizer.tokenAt(2).type == TokenType::identifier) {
                    name = aTokenizer.tokenAt(2).data;
                    aTokenizer.next(1);
                    return StatusResult{ noError };
                }
            }
            return StatusResult{ syntaxError };
        }

        StatusResult run(std::ostream& aStream) const {
            return processor.useDatabase(name);
        }
    };

    //------------------------------------------------------------------------------------------------
    class describeDBStatement :public DBStatement {
    public:
        describeDBStatement(DBCmdProcessor& aProcessor) : DBStatement(aProcessor, Keywords::describe_kw) {}

        StatusResult parse(Tokenizer& aTokenizer) {
            if (aTokenizer.size() > 3) {
                while (aTokenizer.next(1)) {}
                return StatusResult{ illegalIdentifier };
            }
            aTokenizer.next(1);
            if (aTokenizer.tokenAt(1).keyword == Keywords::database_kw) {
                aTokenizer.next(1);
                if (aTokenizer.tokenAt(2).type == TokenType::identifier) {
                    name = aTokenizer.tokenAt(2).data;
                    aTokenizer.next(1);
                    return StatusResult{ noError };
                }
            }
            return StatusResult{ syntaxError };
        }
        StatusResult run(std::ostream& aStream)const {
            return processor.describeDatabase(name);
        }

    };

    //------------------------------------------------------------------------------------------------
    class showDBStatament : public DBStatement {
    public:
        showDBStatament(DBCmdProcessor& aProcessor) : DBStatement(aProcessor, Keywords::show_kw) {}
        StatusResult  parse(Tokenizer& aTokenizer) {
            aTokenizer.next(1);
            if (aTokenizer.tokenAt(1).keyword == Keywords::databases_kw) {
                aTokenizer.next(1);
                return StatusResult{ noError };
            }
            return StatusResult{ syntaxError };
        }

        StatusResult run(std::ostream& aStream)const {
            return processor.showDatabase();
        }
    };

}



#endif /* DBStatement_hpp */
