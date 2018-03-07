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
#ifndef PDBCLIENT_H
#define PDBCLIENT_H

#include "CatalogClient.h"
#include "DispatcherClient.h"
#include "DistributedStorageManagerClient.h"
#include "QueryClient.h"
#include "ServerFunctionality.h"

#include "Handle.h"
#include "PDBObject.h"
#include "PDBVector.h"
#include "PartitionPolicy.h"

#include "SimpleRequest.h"

/**
 * This class provides functionality so users can connect and access
 * different Server functionalities of PDB, such as:
 *
 *  Catalog services
 *  Dispatcher services
 *  Distributed Storage services
 *  Query services
 *
 */

namespace pdb {

    class PDBClient : public ServerFunctionality {

    public:
      /* Creates PDBClient
       *
       *            portIn: the port number of the PDB master server
       *         addressIn: the IP address of the PDB master server
       *        myLoggerIn: the logger
       *         usePangea: true if Pangea is used
       * useQueryScheduler: true if Query Scheduler is used
       *
       */

      PDBClient(int portIn, std::string addressIn,
                bool usePangeaIn, bool useQuerySchedulerIn);

      PDBClient();

      ~PDBClient();

      void registerHandlers(PDBServer &forMe); // no - op

      /****
       * Methods for invoking DistributedStorageManager-related operations
       */

      /* Creates a database */
      void createDatabase(const std::string &databaseName);

      /* Creates a set with a given type for an existing database, no page_size
       * needed in arg. */
      void createSet(const std::string &databaseName, const std::string &setName,
                     const std::string &typeName);

      /* Creates a set with a given type (using a template) for an existing
       * database, no page_size needed in arg. */
      template <class DataType>
      void createSet(const std::string &databaseName, const std::string &setName);

      /* Creates a set with a given type for an existing database */
      void createSet(const std::string &databaseName, const std::string &setName,
                     const std::string &typeName,
                     size_t pageSize = DEFAULT_PAGE_SIZE);

      /* Creates a set with a given type (using a template) for an existing
       * database with page_size value. */
      template <class DataType>
      void createSet(const std::string &databaseName,
                                const std::string &setName, size_t pageSize);

      /* Creates a temporary set with a given type for an existing database (only
       * goes through storage) */
      void createTempSet(const std::string &databaseName,
                         const std::string &setName, const std::string &typeName,
                         size_t pageSize = DEFAULT_PAGE_SIZE);

      /* Flushes data currently in memory into disk. */
      void flushData();

      /* Removes a database from storage. */
      void removeDatabase(const std::string &databaseName);

      /* Removes a set for an existing database from storage. */
      void removeSet(const std::string &databaseName, const std::string &setName);

      /* Removes a set given a type from an existing database. */
      void clearSet(const std::string &databaseName, const std::string &setName,
                    const std::string &typeName);

      /* Removes a temporary set given a type from an existing database (only goes
       * through storage). */
      void removeTempSet(const std::string &databaseName,
                         const std::string &setName, const std::string &typeName);

      /* Exports the content of a set from a database into a file. Note that the
       * objects in
       * set must be instances of ExportableObject
       */
      void exportSet(const std::string &databaseName, const std::string &setName,
                     const std::string &outputFilePath, const std::string &format);

      /****
       * Methods for invoking Catalog-related operations
       */

      /* Sends a request to the Catalog Server to register a user-defined type
       * defined
       * in a shared library. */
      void registerType(std::string fileContainingSharedLib);

      /* Sends a request to the Catalog Server to register metadata about a node in
       * the cluster */
      void registerNode(string &localIP, int localPort, string &nodeName,
                        string &nodeType, int nodeStatus);

      /* Prints the content of the catalog. */
      void
      printCatalogMetadata(pdb::Handle<pdb::CatalogPrintMetadata> itemToSearch);

      /* Lists all metadata registered in the catalog. */
      void listAllRegisteredMetadata();

      /* Lists the Databases registered in the catalog. */
      void listRegisteredDatabases();

      /* Lists the Sets for a given database registered in the catalog. */
      void listRegisteredSetsForADatabase(std::string databaseName);

      /* Lists the Nodes registered in the catalog. */
      void listNodesInCluster();

      /* Lists user-defined types registered in the catalog. */
      void listUserDefinedTypes();

      /****
       * Methods for invoking Dispatcher-related operations
       *
       * @param setAndDatabase
       * @return
       */
      void registerSet(std::pair<std::string, std::string> setAndDatabase,
                       PartitionPolicy::Policy policy);

      /**
       *
       * @param setAndDatabase
       * @return
       */
      template <class DataType>
      void sendData(std::pair<std::string, std::string> setAndDatabase,
                    Handle<Vector<Handle<DataType>>> dataToSend);

      template <class DataType>
      void sendBytes(std::pair<std::string, std::string> setAndDatabase,
                     char *bytes, size_t numBytes);

      /****
       * Methods for invoking Query-related operations
       */

      /* Executes some computations */
      template <class... Types>
      void executeComputations(Handle<Computation> firstParam,
                               Handle<Types>... args);

      /* Deletes a set. */
      void deleteSet(std::string databaseName, std::string setName);

      /* Gets a set iterator. */
      template <class Type>
      SetIterator<Type> getSetIterator(std::string databaseName,
                                       std::string setName);

    private:
      std::shared_ptr<pdb::CatalogClient> catalogClient;
      pdb::DispatcherClient dispatcherClient;
      pdb::DistributedStorageManagerClient distributedStorageClient;
      pdb::QueryClient queryClient;

      std::function<bool(Handle<SimpleRequestResult>)>
      generateResponseHandler(std::string description, std::string &errMsg);

      int port;
      bool usePangea;
      bool useQueryScheduler;
      std::string address;
      PDBLoggerPtr logger;
    };
}

#include "PDBClientTemplate.cc"
#include "DispatcherClientTemplate.cc"
#include "StorageClientTemplate.cc"

#endif
