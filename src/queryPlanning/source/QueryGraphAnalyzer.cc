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
#ifndef QUERY_GRAPH_ANALYZER_SOURCE
#define QUERY_GRAPH_ANALYZER_SOURCE

#include "QueryGraphAnalyzer.h"
#include "InputTupleSetSpecifier.h"
#include <vector>
#include <string.h>


namespace pdb {


QueryGraphAnalyzer ::  QueryGraphAnalyzer ( std :: vector <Handle<Computation>> queryGraph ) {
   for (int i = 0; i < queryGraph.size(); i ++) {
       this->queryGraph.push_back(queryGraph[i]);
   }
}

QueryGraphAnalyzer :: QueryGraphAnalyzer ( Handle<Vector<Handle<Computation>>> queryGraph ) {
   for (int i = 0; i < queryGraph->size(); i ++) {
       this->queryGraph.push_back((*queryGraph)[i]);
   }
}

std :: string QueryGraphAnalyzer :: parseTCAPString () {

    Handle <Computation> curSink;
    int computationLabel = 0;
    std :: vector <std :: string> tcapStrings;
    for (int i = 0; i < this->queryGraph.size(); i++) {

        std :: vector <InputTupleSetSpecifier> inputTupleSets;
        InputTupleSetSpecifier inputTupleSet;
        inputTupleSets.push_back(inputTupleSet);
        curSink = queryGraph[i];
        std :: string outputTupleSetName = "";
        std :: vector<std :: string> outputColumnNames;
        std :: string addedOutputColumnName = "";
        traverse(tcapStrings, curSink, inputTupleSets, computationLabel, outputTupleSetName, outputColumnNames, addedOutputColumnName);
        
    }

    std :: string tcapStringToReturn = "";
    for (int i = 0; i < tcapStrings.size(); i++) {
        tcapStringToReturn += tcapStrings[i];
    }
    std :: cout << tcapStringToReturn << std :: endl;
    return tcapStringToReturn;

}


void QueryGraphAnalyzer :: traverse (std :: vector<std :: string> & tcapStrings, Handle<Computation> sink, std :: vector <InputTupleSetSpecifier>  inputTupleSets, int & computationLabel, std :: string & outputTupleSetName, std :: vector<std :: string> & outputColumnNames, std :: string & addedOutputColumnName) {
  
    int numInputs = sink->getNumInputs();
    std :: string computationName = sink->getComputationType();
    if (numInputs > 0) {
        std :: vector <InputTupleSetSpecifier> inputTupleSetsForMe;
        for (int i = 0; i < numInputs; i++) {
            Handle<Computation> curSink = sink->getIthInput(i);
            if(curSink->isTraversed() == false) {
                traverse(tcapStrings, curSink, inputTupleSets, computationLabel, outputTupleSetName, outputColumnNames, addedOutputColumnName);
                curSink->setTraversed(true);
            } else {
                //we met a materialized node
                outputTupleSetName = curSink->getOutputTupleSetName();
                addedOutputColumnName = curSink->getOutputColumnToApply();
                outputColumnNames.push_back(addedOutputColumnName);
            }
            std :: vector <std :: string> addedOutputColumns;
            addedOutputColumns.push_back(addedOutputColumnName);
            std :: cout << "outputTupleSetName: " << outputTupleSetName << std :: endl;
            std :: cout << "addedOutputColumns: " << addedOutputColumnName << std :: endl;
            InputTupleSetSpecifier curOutput (outputTupleSetName, outputColumnNames, addedOutputColumns);
            inputTupleSetsForMe.push_back(curOutput);
        }
        outputColumnNames.clear();
        addedOutputColumnName = ""; 
        std :: cout << "######################" << std :: endl;
        std :: cout << computationName << ": " << computationLabel << std :: endl;
        for (int i = 0; i < inputTupleSetsForMe.size(); i++) {
            InputTupleSetSpecifier curSet = inputTupleSetsForMe[i];
            std :: cout << "tupleSetName: " << curSet.getTupleSetName() << std :: endl;
            std :: vector < std :: string > columnsToApply = curSet.getColumnNamesToApply();
            for (int j = 0; j < columnsToApply.size(); j++) {
                std :: cout << j << ":" << columnsToApply[j] << std :: endl;
            }
        }
        std :: cout << "######################" << std :: endl;
        std :: string curTcapString = sink->toTCAPString(inputTupleSetsForMe, computationLabel, outputTupleSetName, outputColumnNames, addedOutputColumnName);
        std :: cout << computationName << ": " << computationLabel << ": outputTupleSetName: " << outputTupleSetName << std :: endl;
        std :: cout << computationName << ": " << computationLabel << ":  addedOutputColumns: " << addedOutputColumnName << std :: endl;
        tcapStrings.push_back(curTcapString);
        computationLabel ++;
    } else {
        outputColumnNames.clear();
        addedOutputColumnName = "";
        std :: string curTcapString = sink->toTCAPString(inputTupleSets, computationLabel, outputTupleSetName, outputColumnNames, addedOutputColumnName);
        std :: cout << curTcapString  << std :: endl;
        tcapStrings.push_back(curTcapString);
        computationLabel ++;
        
    }

}

void QueryGraphAnalyzer :: clearGraphMarks (Handle<Computation> sink) {

    sink->setTraversed(false);
    int numInputs = sink->getNumInputs();
    for (int i = 0; i < numInputs; i ++) {
        Handle<Computation> curNode = sink->getIthInput(i);
        clearGraphMarks(curNode);
    }

}


void QueryGraphAnalyzer :: clearGraphMarks() {

    for (int i = 0; i < this->queryGraph.size(); i++) {
        Handle<Computation> sink = this->queryGraph[i];
        clearGraphMarks(sink);
    }

}


void QueryGraphAnalyzer :: parseComputations ( std :: vector <Handle<Computation>> & computations, Handle<Computation> sink ) {

    int numInputs = sink->getNumInputs();
    for (int i = 0; i < numInputs; i ++) {
        Handle<Computation> curNode = sink->getIthInput(i);
        parseComputations(computations, curNode);
    }
    if(sink->isTraversed() == false) {
        computations.push_back(sink);
        sink->setTraversed(true);
    }

} 

void QueryGraphAnalyzer :: parseComputations ( std :: vector <Handle<Computation>> & computations) {
    this->clearGraphMarks();
    for (int i = 0; i < this->queryGraph.size(); i++) {
        Handle<Computation> sink = this->queryGraph[i];
        parseComputations(computations, sink);
    }

}


}

#endif