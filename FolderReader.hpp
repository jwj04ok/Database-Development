//
//  FolderReader.hpp
//  Database5
//
//  Created by rick gessner on 4/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderReader_h
#define FolderReader_h

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace ECE141 {
  
  class FolderListener {
  public:
    virtual bool operator()(const std::string &aName)=0;
  };
  
  class FolderReader {
  public:
        
                  FolderReader(const char *aPath) : path(aPath) {}
    virtual       ~FolderReader() {}
    
    virtual bool  exists(const std::string &aPath) {
                    //STUDENT: add logic to see if FOLDER at given path exists.
                    if (FILE* file = fopen(aPath.c_str(), "r")) {
                        fclose(file);
                        return true;
                    }
                    return false;
                  }
    
    virtual void  each(FolderListener &aListener, const std::string &anExtension) const {
                    //STUDENT: iterate db's, pass the name of each to listener
                    fs::path thePath(path);
                    for (auto& theItem : fs::directory_iterator(path)) {
                        if (!theItem.is_directory()) {
                            fs::path temp(theItem.path());
                            std::string theExt(temp.extension().generic_u8string());
                            if (anExtension.size() == 0 || anExtension.compare(theExt)) {
                                if (!aListener(temp.stem().generic_u8string())) { break; }
                            }
                        }
                    }

                  };
      
    std::string path;
  };
   
}

#endif /* FolderReader_h */
