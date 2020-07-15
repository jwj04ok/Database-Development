//
//  SQLProcessor.cpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "SQLProcessor.hpp"

namespace ECE141 {

 SQLProcessor::SQLProcessor(CommandProcessor *aNext) : CommandProcessor(aNext) {}

 SQLProcessor::~SQLProcessor() {}

 //----------------------------------------------------------
 StatusResult SQLProcessor::createTable(Schema *aSchema) {
     if (Database *theDatabase = getActiveDatabase()) {
         if (aSchema) {
             for (auto exsitedSchema:theDatabase -> getStorage().getTOC().schemas.items){
                 if (aSchema->getName() == exsitedSchema.name){
                     return StatusResult{tableExists};
                 }
             }
             StatusResult theResult = theDatabase->createTable(aSchema);
             if (theResult) {
                 std::cout << "table successfully created!\n";
                 return theResult;
             }
             return StatusResult{invalidCommand};
         }
     }
     return StatusResult{noDatabaseSpecified};
 }

 //-------------------------------------------------------------
StatusResult SQLProcessor::describeTable(const std::string &aName) {
    if (Database *theDatabase = getActiveDatabase()) {
        if(Schema *theSchema = theDatabase->getSchema(aName)){
            return theDatabase->describeTable(std::cout, theSchema);
        }return StatusResult{unknownTable};
        
    }return StatusResult{unknownDatabase};
    
}

//  -----------------------------------------------------
StatusResult SQLProcessor::dropTable(const std::string &aName) {
    
    if (Database *theDatabase = getActiveDatabase()) {
        StatusResult theResult = theDatabase->dropTable(aName);
        if (theResult) {
            std::cout << "tabled dropped\n";
            return theResult;
        }
        else { return StatusResult{ unknownTable }; }
    }
    return StatusResult{unknownDatabase};
}

//-------------------------------------------------------------
StatusResult SQLProcessor::showTables(){
    
    if (Database *theDatabase = getActiveDatabase()) {
        return theDatabase->showTables(std::cout);
    }
    return StatusResult{noDatabaseSpecified};
}
//---------------------------------------------------------------
StatusResult SQLProcessor::insert(const Row& aRow, const std::string& aTableName) {
    if (Database* theDatabase = getActiveDatabase()) {
        return theDatabase->insertRows(aRow, aTableName);
    }
    return StatusResult{ unknownDatabase };
}

//---------------------------------------------------------------
StatusResult SQLProcessor::deleteRows(const std::string& aName, const Filters &theFilters) {
    Timer aTimer;
    if (Database* theDatabase = getActiveDatabase()) {
        if (Schema* theSchema = theDatabase->getSchema(aName)) {
            aTimer.start();
            RowCollection theCollection;
            StatusResult theResult = theDatabase -> selectRows(theCollection, *theSchema, theFilters);
            theDatabase->deleteRows(theCollection, *theSchema, theFilters);
            aTimer.stop();
            double time = aTimer.elapsed();
            std::cout << theCollection.getRows().size() << "rows affected(" << time << " ms.)"<<std::endl;
            return StatusResult{ noError };
        }
        return StatusResult{ unknownTable };
    }
    return StatusResult{ unknownDatabase };
}

//----------------------------------------------------------------

StatusResult SQLProcessor::selectRows(const std::string &tableName, const Filters &theFilters, const std::string &orderKey, const int &limit, const std::vector<Join> &joins){ 
        if (Database* theDatabase = getActiveDatabase()) {
            if (Schema* theSchema = theDatabase->getSchema(tableName)) {
                Timer aTimer;
                aTimer.start();
                RowCollection theCollection;
                StatusResult theResult = theDatabase->selectRows(theCollection, *theSchema, theFilters);
                if (!orderKey.empty()) {
                    theCollection.order(orderKey);
                }
                if (limit != 1) {
                    if (theCollection.getRows().size() > limit) {
                        theCollection.getRows().erase(theCollection.getRows().begin() + limit, theCollection.getRows().end());
                    }
                }
                if(joins.size() > 0){
                    
                    std::string anotherTable;
                    anotherTable = joins[0].rhs.tableName != tableName ? joins[0].rhs.tableName:joins[0].lhs.tableName;
                    
                    if(Schema *anotherSchema = theDatabase->getSchema(anotherTable)){
                        RowCollection anotherCollection;
                        StatusResult theResult = theDatabase->selectRows(anotherCollection, *anotherSchema, theFilters);
                        RowCollection mergedCollection;
                        theResult = mergeTwoTables(theCollection, anotherCollection, joins[0], mergedCollection);
                        Schema mergedSchema;
                        theResult = mergeTwoSchemas(*theSchema,*anotherSchema,mergedSchema);
                        if (View* theView = new TableView(mergedSchema, mergedCollection)) {
                            theView->show(std::cout);
                            delete theView;
                        }
                        return theResult;
                    }
                }
                
                aTimer.stop();
                double time = aTimer.elapsed();
                if (View* theView = new TableView(*theSchema, theCollection)) {
                    theView->show(std::cout);
                    std::cout<<"(" << time<<" ms.)" << std::endl;
                    delete theView;
                }
                return theResult;
            }
        }
    return StatusResult{unknownDatabase};
}

StatusResult SQLProcessor::mergeTwoTables(RowCollection& leftRows, RowCollection& rightRows, const Join& join, RowCollection& mergedRows) {
    switch (join.joinType) {
    case Keywords::left_kw :
        for (auto leftRow : leftRows.getRows()) {
            ValueType leftField = leftRow->getModifiableColumns()[join.lhs.field];
            bool matched = 0;
            for (auto rightRow : rightRows.getRows()) {
                if (rightRow->getModifiableColumns()[join.rhs.field] == leftField) {
                    Row *newRow=new Row(*leftRow,*rightRow);
                    mergedRows.add(newRow);
                    matched = 1;
                }
            }
            if (!matched) {
                Row* halfEmptyRow = new Row(*leftRow, *rightRows.getRows()[0],0);
                mergedRows.add(halfEmptyRow);
            }
        }
        break;
    case Keywords::right_kw:
        for (auto rightRow : rightRows.getRows()) {
            ValueType rightField = rightRow->getModifiableColumns()[join.rhs.field];
            bool matched = 0;
            for (auto leftRow : leftRows.getRows()) {
                if (leftRow->getModifiableColumns()[join.lhs.field] == rightField) {
                    Row* newRow = new Row(*leftRow, *rightRow);
                    mergedRows.add(newRow);
                    delete newRow;
                    matched = 1;
                }
            }
            if (!matched) {
                Row* halfEmptyRow = new Row(*leftRows.getRows()[0],*rightRow, 1);
                mergedRows.add(halfEmptyRow);
                delete halfEmptyRow;
            }
        }
        break;
    default:
        break;
    }
    return StatusResult();
};

StatusResult   SQLProcessor::mergeTwoSchemas(Schema& leftSchema, Schema& rightSchema, Schema& mergedSchema) {
    for (auto anAttr : leftSchema.getAttributes()) {
        mergedSchema.addAttribute(anAttr);
    }
    for (auto anAttr : rightSchema.getAttributes()) {
        if (anAttr.getName() != "id") {
            mergedSchema.addAttribute(anAttr);
        }
    }
    return StatusResult();
};

//-----------------------------------------------------------------------
StatusResult SQLProcessor::updateRows(const std::string& aName, const KeyValues& aKeyValues, const Filters& theFilters) { 
    StatusResult theResult{ noError };
    if (Database* theDatabase = getActiveDatabase()) {
        if (Schema* theSchema = theDatabase->getSchema(aName)) {
            Timer aTimer;
            aTimer.start();
            RowCollection theCollection;
            if ((theResult = theDatabase->selectRows(theCollection, *theSchema, theFilters))) {
                for (auto* theRow : theCollection.getRows()) {
                    theResult = theDatabase->updateRow(*theRow, aKeyValues, *theSchema); 
                }
            }
            aTimer.stop();
            double time = aTimer.elapsed();
            std::cout << theCollection.getRows().size() << " rows affected(" << time << " ms.)"<<std::endl;
        }
        else theResult.code = unknownTable;
    }
    else theResult.code = unknownDatabase;
    
    return theResult;
}

StatusResult SQLProcessor::showIndexs() {
    if (Database* theDatabase = getActiveDatabase()) {
        return theDatabase->showIndexs(std::cout);
    }
    return StatusResult{ noDatabaseSpecified };
}

   StatusResult SQLProcessor::interpret(const Statement &aStatement) {
       return aStatement.run(std::cout);
   }

   Statement *SQLProcessor::getStatement(Tokenizer &aTokenizer) {
       Statement *theResult = nullptr;
       if (aTokenizer.remaining()) {
           Token theToken = aTokenizer.current();
           switch (theToken.keyword) {
               case Keywords::create_kw  : theResult = new CreateTableStatement(*this);
                   break;
               case Keywords::drop_kw    : theResult = new DropTableStatement(*this);
                   break;
               case Keywords::describe_kw: theResult = new DescribeStatement(*this);
                   break;
               case Keywords::show_kw    :{
                   Token &nextToken = aTokenizer.peek(1);
                   if (Keywords::table_kw == nextToken.keyword){
                       theResult = new ShowTablesStatement(*this);
                   }else if (Keywords::indexes_kw == nextToken.keyword){
                       theResult = new ShowIndexStatement(*this);
                   }
                   break;
               }
               case Keywords::insert_kw  : theResult = new InsertRowStatement(*this);
                   break;
               case Keywords::delete_kw  : theResult = new DeleteStatement(*this);
                   break;
               case Keywords::select_kw  : theResult = new SelectStatement(*this);
                   break;
               case Keywords::update_kw  : theResult = new UpdateStatement(*this);
               default: break;
           }
           if (theResult) {
               StatusResult theError = theResult->parse(aTokenizer);
               if (!theError) {
                   std::cout << "Error (" << theError.code << ") "<< "\n";
                   delete theResult;
                   theResult = nullptr;
               }
           }
       }
       return theResult;
   }


}
