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

#ifndef STORAGE_ADD_SET_H
#define STORAGE_ADD_SET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "Configuration.h"

// PRELOAD %StorageAddSet%

namespace pdb {

// encapsulates a request to add a set in storage
class StorageAddSet : public Object {

public:
    StorageAddSet() {}
    ~StorageAddSet() {}

    StorageAddSet(std::string dataBase,
                  std::string setName,
                  std::string typeName,
                  size_t pageSize = DEFAULT_PAGE_SIZE)
        : dataBase(dataBase), setName(setName), typeName(typeName), pageSize(pageSize) {}

    std::string getDatabase() {
        return dataBase;
    }

    std::string getSetName() {
        return setName;
    }

    std::string getTypeName() {
        return typeName;
    }

    size_t getPageSize() {
        return pageSize;
    }


    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
    String typeName;
    size_t pageSize;
};
}

#endif
