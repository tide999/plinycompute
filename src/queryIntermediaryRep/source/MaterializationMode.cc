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
#include "MaterializationMode.h"

namespace pdb_detail
{
    bool MaterializationModeNone::isNone()
    {
        return true;
    }

    void MaterializationModeNone::execute(MaterializationModeAlgo &algo)
    {
        algo.forNone(*this);
    }

    string MaterializationModeNone::tryGetDatabaseName(const string &noneValue)
    {
        return noneValue;
    }

    string MaterializationModeNone::tryGetSetName(const string &noneValue)
    {
        return noneValue;
    }

    MaterializationModeNamedSet::MaterializationModeNamedSet(string databaseName, string setName)
            : _databaseName(databaseName), _setName(setName)
    {
    }

    void MaterializationModeNamedSet::execute(MaterializationModeAlgo &algo)
    {
        algo.forNamedSet(*this);
    }

    bool MaterializationModeNamedSet::isNone()
    {
        return false;
    }

    string MaterializationModeNamedSet::tryGetDatabaseName(const string &defaultValue)
    {
        return getDatabaseName();
    }

    string MaterializationModeNamedSet::tryGetSetName(const string &defaultValue)
    {
        return getSetName();
    }

    string MaterializationModeNamedSet::getDatabaseName()
    {
        return _databaseName;
    }

    string MaterializationModeNamedSet::getSetName()
    {
        return _setName;
    }

}