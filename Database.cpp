//
//  Database.cpp
//  Database1
//
//  Created by rick gessner on 4/12/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include <sstream>
#include "Database.hpp"
#include "View.hpp"


//this class represents the database object.
//This class should do actual database related work,
//we called upon by your db processor or commands

namespace ECE141 {
    
  Database::Database(const std::string aName, CreateNewStorage)
    : name(aName), storage(aName, CreateNewStorage{}) {
  }
  
  Database::Database(const std::string aName, OpenExistingStorage)
    : name(aName), storage(aName, OpenExistingStorage{}) {
  }
  
  Database::~Database() {
  }


 StatusResult Database::describe(std::ostream& anOutput) {
     if (DescribeDatabaseView* aView = new DescribeDatabaseView(storage)) {
         aView->show(anOutput);
         delete aView;
     }
     return StatusResult{ noError };
 }

StatusResult Database:: createTable(Schema* aSchema){
    const std::string &theName = aSchema->getName();
    schemas[theName] = aSchema;
    StorageBlock theBlock;
    theBlock = (*aSchema); //convert from schema...
    StatusResult theResult = storage.addSchema(theName, theBlock);
    Schema *theSchema = schemas[theName];
    theSchema->blockNum = theResult.value; //hang on to the blocknum...
    saveToc();
    //add index block
    Index *theIndex = new Index(theSchema->getPrimaryKeyName(), theSchema->getHash(), theSchema->getAttribute(theSchema->getPrimaryKeyName())->getType(), storage);
    indexs[theName] = theIndex;
    StorageBlock indexBlock{ECE141::BlockType::index_block};
    theResult = theIndex->encode(indexBlock);
    if (theResult) {
        theResult = storage.addBlock(indexBlock);
        if (theResult){
            uint32_t indexblockNum = theResult.value;
            theSchema->indexBlockNum = indexblockNum;
            theBlock = (*theSchema); //rewrite theBlock with update indexBlockNum
//            theBlock.header.id = indexblockNum;
            storage.writeBlock(theBlock, theSchema->blockNum);
            //rewrite schema block to the same blocknum
            
        }
        
    }
    else theResult.code = syntaxError;
    
    
    return theResult;
}

class ShowTablesView : public View {
public:
    ShowTablesView(StorageBlock &aTOC, std::string aName) : toc{aTOC}, name(aName) {}

    bool show(std::ostream &anOutput) {
        anOutput << "+----------------------+" << std::endl;
        anOutput << "| Tables_in_" << std::setw(11) << std::left << name << "|" << std::endl;
        anOutput << "+----------------------+" << std::endl;
        int tmp = 0;
        for (int i = 0; i < toc.schemas.header.count; i++) {
            tmp ++;
            anOutput << "| " << std::setw(21) << std::left << toc.schemas.items[i].name << "|" << std::endl;
        }
        anOutput << "+----------------------+" << std::endl;

        if (toc.schemas.header.count == 1) {
            anOutput << "1 row in set" << std::endl;
        }
        else {
            anOutput << tmp << " rows in set" << std::endl;
        }
        return true;
    }

protected:
    StorageBlock &toc;
    std::string   name;
};
 
StatusResult Database:: showTables(std::ostream& anOutput){
    if (View *theView = new ShowTablesView(storage.tempBlock, name)) {
        theView->show(anOutput);
        delete theView;
        return StatusResult{noError};
    }
    return StatusResult{readError};
};

Schema* Database::getSchema(const std::string &aName){
    if (schemas.count(aName)) {
           return schemas[aName];
       }
    
    else {
    //load schema from storage if not find
    SchemaSummary *theSchema = storage.findSchemaInTOC(aName);
    if (theSchema) { //not nullptr
        StorageBlock theBlock;
        StatusResult theResult = storage.readBlock(theBlock, theSchema->blocknum);
        if (theResult) {
            Schema *aSchema = new Schema(theBlock, theSchema->blocknum);
            schemas[aName] = aSchema; //store the loaded schema to schemas(member of database)
            return aSchema;
        }
    }
    }
       return nullptr;

}

Index* Database::getIndex(const std::string &tableName, const std::string &aField){
    if (indexs.count(tableName)){
        return indexs[tableName];
    }else{
        StatusResult theResult{noError};
        Schema *theSchema = getSchema(tableName);
        Index *aIndex = new Index(storage);
        StorageBlock aBlock;
        storage.readBlock(aBlock, theSchema->getIndexBlockNum());
        theResult = aIndex->decode(aBlock);
        indexs[aField] = aIndex;
        return indexs[aField];
    }
    
}
        

    


StatusResult Database:: dropTable(const std::string& aName){
    if (Schema *theSchema = getSchema(aName)) {
                storage.dropSchema(aName);
                if (schemas.count(aName)) {
                    schemas.erase(aName);
                }
                uint32_t count = storage.getTotalBlockCount();
                for (int i = 0; i < count; ++i) {
                    StorageBlock theBlock;
                    StatusResult theResult;
                    if (theResult = storage.readBlock(theBlock, i)) {
                        if (theSchema->getHash() == theBlock.header.schemaHash) {
                            storage.releaseBlock(i);
                            
                        }
                    }

                }
                return StatusResult{noError};
            }
     return StatusResult{unknownTable};
}


class SchemaDescriptionView : public View {
public:
    SchemaDescriptionView(Schema* theSchema): aSchema(*theSchema), stream(nullptr){};

    bool show(std::ostream &anOutput){
        stream = &anOutput; 
        const char *theSep = "+-----------+--------------+------+-----+---------+-----------------------------+";
        const char *theTitle = "| Field     | Type         | Null | Key | Default | Extra                       |";
        anOutput << theSep << std::endl<< theTitle << std::endl << theSep;

        AttributeList attributes = this->aSchema.getAttributes();

        for (auto &attr : attributes) {
            anOutput << "\n| " << attr.getName();
            size_t nSpace = 10 - attr.getName().length();
            anOutput << std::string(nSpace, ' ');
            switch (attr.getType()) {
                case DataType::int_type :anOutput << "| integer      ";
                    break;
                case DataType::bool_type :anOutput << "| boolean      ";
                    break;
                case DataType::varchar_type: {
                    int count = 0;
                    int opera = attr.getSize();
                    while (opera) {
                        opera /= 10;
                        ++count;
                    }
                    anOutput << "| varchar("<<std::setw(count)<<std::left<<attr.getSize()<< std::setw(5-count) << std::left<<")";
                    break;
                }
                case DataType::float_type:anOutput << "| float        ";
                    break;
                case DataType::datetime_type:anOutput << "| timestamp    ";
                    break;
                default:anOutput << "| n/a          ";
                    break;
                }

            if (attr.isNullable()) anOutput << "| YES  "; 
            else anOutput << "| NO   ";

            if (attr.isPrimaryKey()) anOutput << "| YES ";
            else anOutput << "|     ";

            if (attr.getDefaultValue() != "Null") anOutput << "| " << std::setw(8) << std::left << attr.getDefaultValue() << "|";
            else anOutput << "| NULL    |";

            if (attr.isAutoIncrement()) anOutput << " auto_increment primary key  |";
            else if(attr.isPrimaryKey()) anOutput<< " auto_increment primary key  |";
            else anOutput << "                             |";
            }
            anOutput << "\n";
            anOutput << theSep << std::endl;
            anOutput << aSchema.getAttributes().size() << " rows in set"<< std::endl;
            return true;
    };
protected:
    const Schema &aSchema;
    std::ostream *stream;
};

StatusResult Database:: describeTable(std::ostream& anOutput, Schema* aSchema){
    if (View* theView = new SchemaDescriptionView(aSchema)){
        theView -> show(anOutput);
        delete theView;
        return StatusResult{noError};
    }
    return StatusResult{readError};
}

StatusResult Database::insertRows(const Row& aRow, const std::string& aTableName) {
    if (Schema* theSchema = getSchema(aTableName)) {
        Row theRow = aRow;
        if (uint32_t autoincr = theSchema->getNextAutoIncrementValue()) {
            theRow.data[theSchema->getPrimaryKeyName()] = autoincr;
        }
        // validation of data
        if (theSchema->validate(theRow.data)) {
            StorageBlock theBlock(BlockType::data_block);
            StatusResult theResult = theSchema->encodeRows(theRow.data, theBlock);
            if (theResult) {
                theResult = storage.addBlock(theBlock);
                if (Index *theIndex = getIndex(aTableName, theSchema->getPrimaryKeyName())){
                    KeyValues KV = theRow.getColumns();
                    ValueType aValue = KV[theSchema->getPrimaryKeyName()];
                    theIndex->addKeyValue(aValue, theResult.value);
                    StorageBlock aBlock(BlockType::index_block);
                    theIndex->encode(aBlock);
                    storage.writeBlock(aBlock, theSchema->getIndexBlockNum());
                }
            }

            return StatusResult{ noError };
        }
        return StatusResult{ invalidAttribute };
    }
    return StatusResult{ unknownTable };
}

StatusResult Database::deleteRows(RowCollection& aCollection, const Schema& aSchema, const Filters &theFilters) {
    StatusResult theResult{noError};
    
    std::vector<Row*> &theRows = aCollection.getRows();
    for (auto &aRow: theRows){
        storage.releaseBlock(aRow->getBlockNum());
    }
    
    return theResult;
    /*
    uint32_t count = storage.getTotalBlockCount();
    for (int i = 0; i < count; ++i){
        StorageBlock theBlock;
        if (theResult = storage.readBlock(theBlock, i)){
            if (aSchema.getHash() == theBlock.header.schemaHash){
                storage.releaseBlock(i);
            }
        }
    }
    return StatusResult{ noError };
     */
}

StatusResult Database::selectRows(RowCollection& aCollection, const Schema& aSchema, const Filters& aFilters)
{
    
    StatusResult theResult{ noError };

    
    if (Index *theIndex = getIndex(aSchema.name, aSchema.getPrimaryKeyName())){
        for(auto &thePair: theIndex->getList()){
            uint32_t blockNum = thePair.second;
            StorageBlock aBlock;
            if (storage.readBlock(aBlock, blockNum)){
                KeyValues theKV;
                theResult = aSchema.decodeRows(aBlock, theKV);
                if (aFilters.matches(theKV)){
                    aCollection.add(new Row(theKV, blockNum));
                    
                }
            }
        }
        return theResult;
    }else{
    BlockVisitor aVisitor = BlockVisitor(aCollection, aSchema, aFilters);
    storage.eachBlock(aVisitor);
//    delete aVisitor;
    return theResult;
    }
    /*
    uint32_t count = storage.getTotalBlockCount();
    for (int i = 0; i < count; ++i) {
        StorageBlock theBlock;
        if (theResult = storage.readBlock(theBlock, i)) {
            KeyValues theValues;
            if (theBlock.header.type == 'D' && theBlock.header.schemaHash==aSchema.getHash()) {
                theResult = aSchema.decodeRows(theBlock, theValues);
                if (aFilters.matches(theValues)) {
                    aCollection.add(new Row(theValues, i));
                }
            }
        }

    }
    return StatusResult{noError};
     */
}

StatusResult Database::updateRow(Row& aRow, const KeyValues& aKVList,  Schema& aSchema) {
    KeyValues& newKV = aRow.getModifiableColumns();
    for (auto& aKV : aKVList) {
        Value tempValue(aKV.second);
        tempValue.become(findType[newKV[aKV.first].index()]);
        newKV[aKV.first] = tempValue.getValue();
        /*if (tempValue.become(findType[newKV[aKV.first].index])) {
            newKV[aKV.first] = tempValue.getValue();
        }*/
    }
    StorageBlock theBlock;
    storage.readBlock(theBlock,aRow.getBlockNum());
    aSchema.encodeRows(newKV, theBlock);
    storage.writeBlock(theBlock,aRow.getBlockNum());

    return StatusResult{ noError };
}

class ShowIndexsView : public View {
public:
    ShowIndexsView(std::map<std::string, Schema*>& aSchemas) : schemas{ aSchemas } {}

    bool show(std::ostream& anOutput) {
        anOutput << "+--------------+--------------+" << std::endl;
        anOutput << "| table        | field        | "<< std::endl;
        anOutput << "+--------------+--------------+" << std::endl;
        int tmp = 0;
        for (auto item:schemas) {
            tmp++;
            anOutput << "| " << std::setw(13) << std::left << item.second->getName() << "| ";
            std::string indexName="None";
            for (auto attr : item.second->getAttributes()) {
                if (attr.isPrimaryKey()) {
                    indexName = attr.getName();
                    break;
                }
            }
            anOutput << std::setw(13) << std::left << indexName << "|"<<std::endl;
        }
        anOutput << "+--------------+--------------+" << std::endl;

        anOutput << schemas.size() << " rows in set" << std::endl;
        return true;
    }

protected:
    std::map<std::string, Schema*>& schemas;
};

StatusResult Database::showIndexs(std::ostream& anOutput) {
    if (View* theView = new ShowIndexsView(schemas)) {
        theView->show(anOutput);
        delete theView;
        return StatusResult{ noError };
    }
    return StatusResult{ readError };
};

//save meta block to storage(db file) 
StatusResult Database::saveToc() {
    StatusResult theResult{ noError };
    std::string thePath = storage.getPath(name);
    storage.stream.clear();
    storage.stream.open(thePath.c_str(), std::fstream::in | std::fstream::out);
    storage.stream.close();
    storage.stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out);
    theResult = storage.writeBlock(storage.getTOC(), 0);
    return theResult;
}

}

