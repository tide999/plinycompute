/*****************************************************************************
 *                                                                           *
 *  Copyright 2018 Rice University                                           *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/
#ifndef STORAGE_EXPORT_SET
#define STORAGE_EXPORT_SET


#include "Object.h"
#include "PDBString.h"

namespace pdb {

// PRELOAD %StorageExportSet%
class StorageExportSet : public Object {

public:
    StorageExportSet() {}

    StorageExportSet(std::string dbName,
                     std::string setName,
                     std::string outputFilePath,
                     std::string format) {

        this->dbName = dbName;
        this->setName = setName;
        this->outputFilePath = outputFilePath;
        this->format = format;
    }

    std::string getDbName() {
        return dbName;
    }

    std::string getSetName() {
        return setName;
    }

    std::string getOutputFilePath() {
        return outputFilePath;
    }

    std::string getFormat() {
        return format;
    }

    ENABLE_DEEP_COPY


private:
    String dbName;

    String setName;

    String outputFilePath;

    String format;
};
}
#endif
