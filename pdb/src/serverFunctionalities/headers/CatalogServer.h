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
#ifndef CATALOG_SERVER_H
#define CATALOG_SERVER_H

#include <mutex>
#include <CatDeleteDatabaseRequest.h>

#include "CatalogClient.h"
#include "PDBCatalog.h"
#include "PDBDebug.h"
#include "PDBServer.h"
#include "CatCreateDatabaseRequest.h"
#include "CatDeleteSetRequest.h"
#include "ServerFunctionality.h"

/**
 * Class for handling requests regarding the Catalog Server functionality.
 * This can be used either by the Catalog Manager Server or a Catalog in any
 * Worker Node in the cluster. All metadata is stored and retrieved via
 * the pdbCatalog class, which has an SQLite database as the underlying
 * persistent storage.
 *
 * If this is the manager catalog server, it will receive a metadata
 * registration request and perform the following operations:
 *
 *    1) update metadata in the local manager catalog SQLite database
 *    2) iterate over all registered nodes in the cluster and send the metadata
 * object
 *    3) update catalog version
 *
 *  if this is the catalog instance of a worker node, it will receive a metadata
 * registration
 *  request from the manager catalog server and perform the following operations:
 *
 *    1) update metadata in the local catalog SQLite database
 *    2) update catalog version
 *    3) send acknowledgement to manager catalog server
 */

namespace pdb {

class CatalogServer : public ServerFunctionality {

public:

  /**
   * Creates a Catalog Server
   * @param catalogDirectory : the path of the location of the catalog
   * @param isManagerCatalogServer : true if this is the Manager Catalog Server, workers nodes have this parameter set to false
   * @param managerIP : the IP address of the Manager Catalog
   * @param managerPort : the port number of the Manager Catalog
   */
  CatalogServer(const string &catalogDirectory, bool isManagerCatalogServer,
                const string &managerIP, int managerPort);

  /**
   * Default destructor
   */
  ~CatalogServer() = default;

  /**
   * From the ServerFunctionality interface
   * @param forMe
   */
  void registerHandlers(PDBServer &forMe) override;

  bool databaseExists(const std::string &name);

  bool setExists(const std::string &dbName, const std::string &setName);

  bool registerNode(PDBCatalogNodePtr node, std::string &error);

  bool registerSet(PDBCatalogSetPtr set, std::string &error);

  bool registerDatabase(PDBCatalogDatabasePtr db, std::string &error);

  bool removeDatabase(const std::string &dbName, std::string &error);

  bool removeSet(const std::string &dbName, const std::string &setName, std::string &error);

  PDBCatalogSetPtr getSet(const std::string &dbName, const std::string &setName);

  PDBCatalogDatabasePtr getDatabase(const std::string &dbName);

  PDBCatalogTypePtr getTypeWithoutLibrary(const std::string &name);

  std::vector<PDBCatalogNode> getNodesWithSet(const std::string &dbName, const std::string &setName);

  std::vector<PDBCatalogNode> getNodesWithDatabase(const std::string &dbName);

private:

  /**
   * Interface to a persistent catalog storage for storing and retrieving PDB
   * metadata.
   * All metadata is stored in an SQLite database.
   */
  PDBCatalogPtr pdbCatalog;

  /**
   * Path where the catalog file is located
   */
  std::string catalogDirectory;

  /**
   * Path where we store the .so files
   */
  std::string tempPath;

  /**
   * True if this is the Manager Catalog Server
   */
  bool isManagerCatalogServer;

  /**
   * The ip address of the manager
   */
  string managerIP;

  /**
   * Default port of the Catalog Server, can be changed in the constructor
   */
  int managerPort;

  /**
   * Logger to capture debug information for the Catalog Server
   */
  PDBLoggerPtr catServerLogger;

  /**
   * To ensure serialized access
   */
  std::mutex serverMutex;

  /**
   * Makes the directories of the catalog if needed
   */
  void initDirectories() const;

  /**
   * Initializes the catalog with the built in types if needed
   */
  void initBuiltInTypes();

  bool broadcastRegisterSet(Handle<CatCreateSetRequest> &request, std::string &error);

  bool broadcastRegisterDatabase(Handle<CatCreateDatabaseRequest> &request, std::string &error);

  bool broadcastDeleteSet(Handle<CatDeleteSetRequest> &request, std::string &error);

  bool broadcastDeleteDatabase(Handle<CatDeleteDatabaseRequest> &request, std::string &error);

  /**
   * Adds a new object type... return -1 on failure, this is done on a worker node catalog
   * the typeID is given by the manager catalog
   * @param typeID
   * @param soFile
   * @param errMsg
   * @return
   */
  int16_t loadAndRegisterType(int16_t typeID, const char* &soFile, size_t soFileSize, string &errMsg);

  /**
   * Broadcasts a request to all available nodes in a cluster, when an update has occurred
   * returns a map with the results from updating each node in the cluster
   * @tparam Type - The type of the request we are broadcasting
   * @param request - the request we are broadcasting
   * @param broadcastResults - the results of the broadcast
   * @param errMsg - the error message generated if any
   * @return - true if we succeed false otherwise
   */
  template <class Type>
  bool broadcastRequest(Handle<Type> &request, map<string, pair<bool, string>> &broadcastResults, string &errMsg){

    // go through each node
    for (auto &node : pdbCatalog->getNodes()) {

      // grab the address and the port of the node
      std::string nodeIP = node.address;
      int nodePort = node.port;

      // is this node the manager if so skip it
      if (node.nodeType != "manager") {

        // sends the request to a node in the cluster
        bool res = forwardRequest(request, nodeIP, nodePort, errMsg);

        // adds the result of the update
        broadcastResults.insert(make_pair(nodeIP, make_pair(res, errMsg)));
      }
    }

    return true;
  }


  /**
   * Templated method to forward a request to the Catalog Server on another node
   * @tparam Type - is the type of the request we want to forward
   * @param request - is the request we want to forward
   * @param address - is the address of the node with the catalog server
   * @param port - is the port of the node with the catalog server
   * @param errMsg - the generated error message if any
   * @return - true if we succeed, false otherwise
   */
  template <class Type>
  bool forwardRequest(pdb::Handle<Type> &request, const std::string &address, int port, std::string &errMsg) {

    return simpleRequest<Type, SimpleRequestResult, bool>(
        this->catServerLogger, port, address, false, 1024 * 1024,
        [&](Handle<SimpleRequestResult> result) {

          // if the result is something else null we got a response
          if (result != nullptr) {

            // check if we failed
            if (!result->getRes().first) {

              // we failed set the error and return false
              errMsg = "Error failed request of type : "; +  + " "  + result->getRes().second;
              errMsg.append(typeid(Type).name());
              errMsg.append(", with error : " + result->getRes().second);

              // log the error
              this->catServerLogger->error("Error registering node metadata: " + result->getRes().second);

              // return false
              return false;
            }

            // we are good return true
            return true;
          }

          // set an error and return false
          errMsg = "Error failed request of type : "; +  + " "  + result->getRes().second;
          errMsg.append(typeid(Type).name());
          errMsg.append(", with error : " + result->getRes().second);

          return false;
        },
        request);
  }

};
}

#endif
