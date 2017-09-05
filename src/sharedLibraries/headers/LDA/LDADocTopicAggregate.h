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
#ifndef LDA_DOC_TOPIC_AGGREGATE_H
#define LDA_DOC_TOPIC_AGGREGATE_H

//by Shangyu, May 2017

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "IntIntVectorPair.h"
#include "LDADocWordTopicAssignment.h"
#include "PDBVector.h"


using namespace pdb;


class LDADocTopicAggregate : public ClusterAggregateComp <DocAssignment, DocAssignment, unsigned, DocAssignment> {

public:

        ENABLE_DEEP_COPY

        // the key type must have == and size_t hash () defined
        Lambda <unsigned> getKeyProjection (Handle <DocAssignment> aggMe) override {
		return makeLambdaFromMethod (aggMe, getKey);
        }

        // the value type must have + defined
        Lambda <DocAssignment> getValueProjection (Handle <DocAssignment> aggMe) override {
		return makeLambdaFromMethod (aggMe, getValue);
        }

};


#endif
