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
#ifndef BASEQUERY_H
#define BASEQUERY_H

#include "Lambda.h"
#include "Object.h"
#include "PDBString.h"

// PRELOAD %BaseQuery%

namespace pdb {

class BaseQuery : public Object {
public:
    ENABLE_DEEP_COPY

    BaseQuery() {}
    ~BaseQuery() {}
    virtual void toMap(std::map<std::string, pdb::GenericLambdaObjectPtr>& fillMe,
                       int& identifier) {
        std::cout << "Not expected in BaseQuery" << std::endl;
    }
};
}

#endif