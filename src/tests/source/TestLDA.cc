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
#ifndef TEST_LDA_CC
#define TEST_LDA_CC


// By Shangyu, June 2017
// LDA using Gibbs Sampling;

#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "QueryClient.h"
#include "DistributedStorageManagerClient.h"

#include "DispatcherClient.h"
#include "Set.h"
#include "DataTypes.h"
#include "DoubleVector.h"
#include "SumResult.h"
#include "WriteSumResultSet.h"

#include "LDADocument.h"
#include "LDADocIDAggregate.h"
#include "ScanLDADocumentSet.h"
#include "LDAInitialTopicProbSelection.h"
#include "WriteIntDoubleVectorPairSet.h"
#include "IntDoubleVectorPair.h"
#include "ScanIntSet.h"
#include "WriteIntSet.h"
#include "LDA/LDAInitialWordTopicProbMultiSelection.h"

#include <ctime>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <math.h>
#include <random>



using namespace pdb;
int main (int argc, char * argv[]) {
    bool printResult = true;
    bool clusterMode = false;
   // freopen("output.txt","w",stdout);
    std::ofstream term("/dev/tty", std::ios_base::out);

//    std::ofstream out("output.txt");
//    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
//    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

    const std::string red("\033[0;31m");
    const std::string green("\033[1;32m");
    const std::string yellow("\033[1;33m");
    const std::string blue("\033[1;34m");
    const std::string cyan("\033[0;36m");
    const std::string magenta("\033[0;35m");
    const std::string reset("\033[0m");


    std :: cout << "Usage: #printResult[Y/N] #clusterMode[Y/N] #dataSize[MB] #masterIp #addData[Y/N]" << std :: endl;        
    if (argc > 1) {
        if (strcmp(argv[1],"N") == 0) {
            printResult = false;
            std :: cout << "You successfully disabled printing result." << std::endl;
        }else {
            printResult = true;
            std :: cout << "Will print result." << std :: endl;
        }
    } else {
        std :: cout << "Will print result. If you don't want to print result, you can add N as the first parameter to disable result printing." << std :: endl;
    }

    if (argc > 2) {
        if (strcmp(argv[2],"Y") == 0) {
            clusterMode = true;
            std :: cout << "You successfully set the test to run on cluster." << std :: endl;
        } else {
            clusterMode = false;
        }
    } else {
        std :: cout << "Will run on local node. If you want to run on cluster, you can add any character as the second parameter to run on the cluster configured by $PDB_HOME/conf/serverlist." << std :: endl;
    }

    int numOfMb = 64; //by default we add 64MB data
    if (argc > 3) {
        numOfMb = atoi(argv[3]);
    }
    numOfMb = 64; //Force it to be 64 by now.


    std :: cout << "To add data with size: " << numOfMb << "MB" << std :: endl;

    std :: string masterIp = "localhost";
    if (argc > 4) {
        masterIp = argv[4];
    }
    std :: cout << "Master IP Address is " << masterIp << std :: endl;

    bool whetherToAddData = true;
    if (argc > 5) {
        if (strcmp(argv[5],"N") == 0) {
            whetherToAddData = false;
        }
    }


    term << blue << std :: endl;
    term << "*****************************************" << std :: endl;
    term << "LDA starts : " << std :: endl;
    term << "*****************************************" << std :: endl;
    term << reset << std :: endl;

    
    term << blue << "The LDA paramers are: " << std :: endl;
    term << reset << std :: endl;

    // Basic parameters for LDA
    int iter = 1;
    int numDoc = 10;
    int numWord = 10;
    int numTopic = 4;

    if (argc > 6) {
	iter = std::stoi(argv[6]);
    }
    std :: cout << "The number of iterations: " << iter << std :: endl;

    if (argc > 7) {
	numDoc = std::stoi(argv[7]);
    }
    std :: cout << "The number of documents: " << numDoc << std :: endl;

    if (argc > 8) {
	numWord = std::stoi(argv[8]);
    }
    std :: cout << "The dictionary size: " << numWord << std :: endl;

    if (argc > 9) {
	numTopic = std::stoi(argv[9]);
    }
    std :: cout << "The number of topics: " << numTopic << std :: endl;


    std :: cout << std :: endl;


    pdb :: PDBLoggerPtr clientLogger = make_shared<pdb :: PDBLogger>("clientLog");

    pdb :: DistributedStorageManagerClient temp (8108, masterIp, clientLogger);

    pdb :: CatalogClient catalogClient (8108, masterIp, clientLogger);

    string errMsg;

    // Some meta data
    pdb :: makeObjectAllocatorBlock(1 * 1024 * 1024, true);
    pdb::Handle<pdb::Vector<double>> alpha = pdb::makeObject<pdb::Vector<double>> (numTopic, numTopic);

    // For the random number generator
    std::random_device rd;
    std::mt19937 randomGen(rd());
	
    alpha->fill(1.0);


    if (whetherToAddData == true) {
        //Step 1. Create Database and Set
        // now, register a type for user data

        // now, create a new database
        if (!temp.createDatabase ("LDA_db", errMsg)) {
            cout << "Not able to create database: " + errMsg;
            exit (-1);
        } else {
            cout << "Created database.\n";
        }

        // now, create a new set in that database
        if (!temp.createSet<LDADocument> ("LDA_db", "LDA_input_set", errMsg)) {
            cout << "Not able to create set: " + errMsg;
            exit (-1);
        } else {
            cout << "Created set.\n";
        }

        if (!temp.createSet<int> ("LDA_db", "LDA_meta_data_set", errMsg)) {
            cout << "Not able to create set: " + errMsg;
            exit (-1);
        } else {
            cout << "Created set.\n";
        }

        //Step 2. Add data
        DispatcherClient dispatcherClient = DispatcherClient(8108, masterIp, clientLogger);


        if (numOfMb > 0) {
            int numIterations = numOfMb/64;
            std:: cout << "Number of MB: " << numOfMb << " Number of Iterations: " << numIterations << std::endl;
            int remainder = numOfMb - 64*numIterations;
            if (remainder > 0) { 
                numIterations = numIterations + 1; 
            }
            for (int num = 0; num < numIterations; num++) {
                std::cout << "Iterations: "<< num << std::endl;
                int blockSize = 64;
                if ((num == numIterations - 1) && (remainder > 0)){
                    blockSize = remainder;
                }
                pdb :: makeObjectAllocatorBlock(blockSize * 1024 * 1024, true);
		pdb::Handle<pdb::Vector<pdb::Handle<LDADocument>>> storeMe = 
			pdb::makeObject<pdb::Vector<pdb::Handle<LDADocument>>> ();
                try {


                    for (int i = 0; i < numDoc; i++) {

			int length = storeMe->size();
			std::uniform_int_distribution<> int_unif(1, 3);
                        for (int j = 0; j < numWord; j++){
			    std::uniform_real_distribution<> real_unif(0, 1);
			    double ifWord = real_unif(randomGen);
			    if (ifWord > 0.3) {
                                pdb :: Handle <LDADocument> myData = pdb::makeObject<LDADocument>();
				myData->setDoc(i);
				myData->setWord(j);
				myData->setCount(int_unif(randomGen));
      			//        myData->push_back(i);
                        //    	myData->push_back(j);
			//	myData->push_back(int_unif(randomGen));

                        	storeMe->push_back (myData);
			    }
                        }
			if (storeMe->size() == length) {
				term << "We do not get any words for this document: " << i << std::endl;
				std::uniform_int_distribution<> int_unif2(0, numWord - 1);
                                pdb :: Handle <LDADocument> myData = pdb::makeObject<LDADocument>();
				myData->setDoc(i);
				myData->setWord(int_unif2(randomGen));
				myData->setCount(int_unif(randomGen));

      			      //  myData->push_back(i);
			      //myData->push_back(int_unif2(randomGen));
				//myData->push_back(int_unif(randomGen));
                        	storeMe->push_back (myData);
			}
                    }
		    
		    term << std :: endl;
		    term << green << "input data: " << reset << std :: endl;
                    for (int i=0; i<storeMe->size();i++){
                        (*storeMe)[i]->print();
                    }
		    
                } catch (pdb :: NotEnoughSpace &n) {
                    if (!dispatcherClient.sendData<LDADocument>(std::pair<std::string, std::string>("LDA_input_set", "LDA_db"), storeMe, errMsg)) {
                        std :: cout << "Failed to send data to dispatcher server" << std :: endl;
                        return -1;
                    }
                }
                PDB_COUT << blockSize << "MB data sent to dispatcher server~~" << std :: endl;
            }

            //to write back all buffered records        
            temp.flushData( errMsg );
        }

    // add meta data
        if (numOfMb > 0) {
            int numIterations = numOfMb/64;
            std:: cout << "Number of MB: " << numOfMb << " Number of Iterations: " << numIterations << std::endl;
            int remainder = numOfMb - 64*numIterations;
            if (remainder > 0) { 
                numIterations = numIterations + 1; 
            }
            for (int num = 0; num < numIterations; num++) {
                std::cout << "Iterations: "<< num << std::endl;
                int blockSize = 64;
                if ((num == numIterations - 1) && (remainder > 0)){
                    blockSize = remainder;
                }
                pdb :: makeObjectAllocatorBlock(blockSize * 1024 * 1024, true);
		pdb::Handle<pdb::Vector<pdb::Handle<int>>> storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<int>>> ();
                try {
		    Handle <int> myData = makeObject <int> (numWord);
		    storeMe->push_back(myData);
		  //  storeMe->push_back(&numWord);

		    term << std :: endl;
		    term << green << "Dictionary size: " << *((*storeMe)[0]) << reset << std :: endl;
		    term << std :: endl;
        		    
                } catch (pdb :: NotEnoughSpace &n) {
                    if (!dispatcherClient.sendData<int>(std::pair<std::string, std::string>("LDA_meta_data_set", "LDA_db"), storeMe, errMsg)) {
                        std :: cout << "Failed to send data to dispatcher server" << std :: endl;
                        return -1;
                    }
                }
                PDB_COUT << blockSize << "MB data sent to dispatcher server~~" << std :: endl;
            }

            //to write back all buffered records        
            temp.flushData( errMsg );
        }
    }




    // now, create a new set in that database to store output data
    
    PDB_COUT << "to create a new set to store the data count" << std :: endl;
    if (!temp.createSet<int> ("LDA_db", "LDA_init_test_set", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit (-1);
    } else {
        cout << "Created set LDA_init_test_set.\n";
    }


    PDB_COUT << "to create a new set to store the initial model" << std :: endl;
    if (!temp.createSet<DoubleVector> ("LDA_db", "LDA_initial_model_set", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit (-1);
    } else {
        cout << "Created set LDA_initial_model_set.\n";
    }
    
    
    PDB_COUT << "to create a new set for storing output data" << std :: endl;
    if (!temp.createSet<DoubleVector> ("LDA_db", "LDA_output_set", errMsg)) {
        cout << "Not able to create set: " + errMsg;
        exit (-1);
    } else {
        cout << "Created set LDA_output_set.\n";
    }


    
    //Step 3. To execute a Query
	// for allocations

	// register this query class
    
    catalogClient.registerType ("libraries/libLDADocIDAggregate.so", errMsg);
    catalogClient.registerType ("libraries/libScanLDADocumentSet.so", errMsg);
    catalogClient.registerType ("libraries/libLDAInitialTopicProbSelection.so", errMsg);
    catalogClient.registerType ("libraries/libWriteIntDoubleVectorPairSet.so", errMsg);
    catalogClient.registerType ("libraries/libIntDoubleVectorPair.so", errMsg);
    catalogClient.registerType ("libraries/libScanIntSet.so", errMsg);
    catalogClient.registerType ("libraries/libWriteIntSet.so", errMsg);
    catalogClient.registerType ("libraries/libLDAInitialWordTopicProbMultiSelection.so", errMsg);
    


	// connect to the query client
    QueryClient myClient (8108, "localhost", clientLogger, true);
//    const UseTemporaryAllocationBlock tempBlock {1024 * 1024 * 128};
    
    // Initialization for LDA
       
    // Initialize the topic mixture probabilities for each doc
    
    /*
    Handle<Computation> myInitialScanSet = makeObject<ScanLDADocumentSet>("LDA_db", "LDA_input_set");
    Handle<Computation> myDocID = makeObject<LDADocIDAggregate>();
    myDocID->setInput(myInitialScanSet);
    Handle<Computation> myDocTopicProb = makeObject<LDAInitialTopicProbSelection>(*alpha);
    myDocTopicProb->setInput(myDocID);
    */

    // Initialize the (wordID, topic prob vector)
    Handle<Computation> myMetaScanSet = makeObject<ScanIntSet>("LDA_db", "LDA_meta_data_set");
//    Handle<Computation> myWordTopicProb = makeObject<LDAInitialWordTopicProbMultiSelection>(numWord, numTopic);
//    myWordTopicProb->setInput(myMetaScanSet);
//    Handle<Computation> myDocID = makeObject<LDADocIDAggregate>();
//    myDocID->setInput(myMetaScanSet);

//    Handle <Computation> myWriter = makeObject<WriteIntDoubleVectorPairSet>("LDA_db", "LDA_init_test_set");
//    Handle<Computation> myDocTopicProb = makeObject<LDAInitialTopicProbSelection>(*alpha);
//    myDocTopicProb->setInput(myMetaScanSet);
    Handle <Computation> myWriter = makeObject<WriteIntSet>("LDA_db", "LDA_init_test_set");
    myWriter->setInput(myMetaScanSet);

    if (!myClient.executeComputations(errMsg, myWriter)) {
        std :: cout << "Query failed. Message was: " << errMsg << "\n";
        return 1;
    }

    /*
    SetIterator <IntDoubleVectorPair> initTopicProbResult = 
				myClient.getSetIterator <IntDoubleVectorPair> ("LDA_db", "LDA_init_test_set");
    for (Handle<IntDoubleVectorPair> a : initTopicProbResult) {
	term << "Word ID: " << a->getInt() << " Topic Probability: ";
	a->getVector()->print(); 
	term << std::endl;	
    }*/
    
	
    SetIterator <int> initTopicProbResult = 
				myClient.getSetIterator <int> ("LDA_db", "LDA_init_test_set");
    for (auto a : initTopicProbResult) {
	std::cout << "Word ID: " << *a << " Topic Probability: ";
	std::cout << std::endl;	
    }

//    dataCount = 10;
//    std :: cout << "The number of data points: " << dataCount << std :: endl;
//    std :: cout << std :: endl;
   	 
     
    // Start LDA iterations
    /*	
    for (int n = 0; n < iter; n++) {


                const UseTemporaryAllocationBlock tempBlock {1024 * 1024 * 24};

		term << "*****************************************" << std :: endl;
		term << "I am in iteration : " << n << std :: endl;
		term << "*****************************************" << std :: endl;
    }
    */
		/*
    		pdb::Handle<pdb::Vector<Handle<DoubleVector>>> my_model = pdb::makeObject<pdb::Vector<Handle<DoubleVector>>> ();

		for (int i = 0; i < k; i++) {
			Handle<DoubleVector> tmp = pdb::makeObject<DoubleVector>(dim);
			my_model->push_back(tmp);
			for (int j = 0; j < dim; j++) {
				(*my_model)[i]->setDouble(j, model[i][j]);
			}
		}

		
                std :: cout << "The std model I have is: " << std :: endl;
                for (int i = 0; i < k; i++) {
                     for (int j = 0; j < dim; j++) {
                         std :: cout << "model[" << i << "][" << j << "]=" << model[i][j] << std :: endl;
                     }
                }
		

	    	std :: cout << "The model I have is: " << std :: endl;
		for (int i = 0; i < k; i++) {
			(*my_model)[i]->print();
	    	}		    
		
	
			
    		Handle<Computation> myScanSet = makeObject<ScanLDADocumentSet>("LDA_db", "LDA_input_set");
		Handle <Computation> myDocWordTopicJoin = makeObject <LDADocWordTopicJoin> ();

		Handle<Computation> myQuery = makeObject<KMeansAggregate>(my_model);
		myQuery->setInput(myScanSet);
                //myQuery->setAllocatorPolicy(noReuseAllocator);
                //myQuery->setOutput("LDA_db", "LDA_output_set");
		//Handle<Computation> myWriteSet = makeObject<WriteKMeansSet>("LDA_db", "LDA_output_set");
		//myWriteSet->setAllocatorPolicy(noReuseAllocator);
		//myWriteSet->setInput(myQuery);


		auto begin = std :: chrono :: high_resolution_clock :: now();

		if (!myClient.executeComputations(errMsg, myQuery)) {
			std :: cout << "Query failed. Message was: " << errMsg << "\n";
			return 1;
		}

		std :: cout << "The query is executed successfully!" << std :: endl;

		// update the model
		SetIterator <KMeansAggregateOutputType> result = myClient.getSetIterator <KMeansAggregateOutputType> ("LDA_db", "LDA_output_set");
		kk = 0;

		if (ifFirst) {
			for (Handle<KMeansAggregateOutputType> a : result) {
				if (kk >= k)
						break;
				std :: cout << "The cluster index I got is " << (*a).getKey() << std :: endl;
				std :: cout << "The cluster count sum I got is " << (*a).getValue().getCount() << std :: endl;
				std :: cout << "The cluster mean sum I got is " << std :: endl;
				(*a).getValue().getMean().print();
				DoubleVector tmpModel = (*a).getValue().getMean() / (*a).getValue().getCount();
				for (int i = 0; i < dim; i++) {
					model[kk][i] = tmpModel.getDouble(i);
				}

				for (int i = 0; i < dim; i++) {
					avgData[i] += (*a).getValue().getMean().getDouble(i);
				}

				std :: cout << "I am updating the model in position: " << kk << std :: endl;
				for(int i = 0; i < dim; i++)
					std::cout << i << ": " << model[kk][i] << ' ';
				std :: cout << std :: endl;
				std :: cout << std :: endl;
				kk++;
			}
			for (int i = 0; i < dim; i++) {
				avgData[i] = avgData[i] / dataCount;
			}
			std :: cout << "The average of data points is : \n";
			for (int i = 0; i < dim; i++)
				std::cout << i << ": " << avgData[i] << ' ';
			std :: cout << std :: endl;
			std :: cout << std :: endl;
			ifFirst = false;
		}
		else {
			for (Handle<KMeansAggregateOutputType> a : result) {
				if (kk >= k)
						break;
				std :: cout << "The cluster index I got is " << (*a).getKey() << std :: endl;
				std :: cout << "The cluster count sum I got is " << (*a).getValue().getCount() << std :: endl;
				std :: cout << "The cluster mean sum I got is " << std :: endl;
				(*a).getValue().getMean().print();
		//		(*model)[kk] = (*a).getValue().getMean() / (*a).getValue().getCount();
				DoubleVector tmpModel = (*a).getValue().getMean() / (*a).getValue().getCount();
				for (int i = 0; i < dim; i++) {
					model[kk][i] = tmpModel.getDouble(i);
				}
				std :: cout << "I am updating the model in position: " << kk << std :: endl;
				for(int i = 0; i < dim; i++)
					std::cout << i << ": " << model[kk][i] << ' ';
				std :: cout << std :: endl;
				std :: cout << std :: endl;
				kk++;
			}
		}
		if (kk < k) {
			std :: cout << "These clusters do not have data: "  << std :: endl;
			for (int i = kk; i < k; i++) {
				std :: cout << i << ", ";
				for (int j = 0; j < dim; j++) {
					double bias = ((double) rand() / (RAND_MAX));
					model[i][j] = avgData[j] + bias;
				}
			}
		}
		std :: cout << std :: endl;
		std :: cout << std :: endl;

		temp.clearSet("LDA_db", "LDA_output_set", "pdb::KMeansAggregateOutputType", errMsg);

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Time Duration: " <<
		std::chrono::duration_cast<std::chrono::duration<float>>(end-begin).count() << " secs." << std::endl;
    }

    std::cout << std::endl;

    //QueryClient myClient (8108, "localhost", clientLogger, true);

	// print the resuts
    if (printResult == true) {
//        std :: cout << "to print result..." << std :: endl;

//	std :: cout << std :: endl;

	*
        SetIterator <DoubleVector> input = myClient.getSetIterator <DoubleVector> ("LDA_db", "LDA_input_set");
        std :: cout << "Query input: "<< std :: endl;
        int countIn = 0;
        for (auto a : input) {
            countIn ++;
            std :: cout << countIn << ":";
            a->print();
            std :: cout << std::endl;
        }
	*

        
        SetIterator <KMeansAggregateOutputType> result = myClient.getSetIterator <KMeansAggregateOutputType> ("LDA_db", "LDA_output_set");


	std :: cout << std :: endl;
	std :: cout << blue << "*****************************************" << reset << std :: endl;
	std :: cout << blue << "K-means resultss : " << reset << std :: endl;
	std :: cout << blue << "*****************************************" << reset << std :: endl;
	std :: cout << std :: endl;

//                std :: cout << "The std model I have is: " << std :: endl;
	for (int i = 0; i < k; i++) {
	     std :: cout << "Cluster index: " << i << std::endl;
	     for (int j = 0; j < dim - 1; j++) {
		 std :: cout << blue << model[i][j] << ", " << reset;
	     }
		 std :: cout << blue << model[i][dim - 1] << reset << std :: endl;
	}

    }
    */


    std :: cout << std :: endl;

    if (clusterMode == false) {
	    // and delete the sets
        myClient.deleteSet ("LDA_db", "LDA_output_set");
        myClient.deleteSet ("LDA_db", "LDA_initial_model_set");
        myClient.deleteSet ("LDA_db", "LDA_init_test_set");
    } else {
        if (!temp.removeSet ("LDA_db", "LDA_output_set", errMsg)) {
            cout << "Not able to remove set: " + errMsg;
            exit (-1);
        }
        else if (!temp.removeSet ("LDA_db", "LDA_initial_model_set", errMsg)) {
            cout << "Not able to remove set: " + errMsg;
            exit (-1);
        }
        else if (!temp.removeSet ("LDA_db", "LDA_init_test_set", errMsg)) {
            cout << "Not able to remove set: " + errMsg;
            exit (-1);
        }

	else {
            cout << "Removed set.\n";
        }
    }
    int code = system ("scripts/cleanupSoFiles.sh");
    if (code < 0) {
        std :: cout << "Can't cleanup so files" << std :: endl;
    }
//    std::cout << "Time Duration: " <<
//    std::chrono::duration_cast<std::chrono::duration<float>>(end-begin).count() << " secs." << std::endl;
}

#endif