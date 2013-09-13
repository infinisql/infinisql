/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL (tm). It is available either under the
 * GNU Affero Public License or under a commercial license. Contact the
 * copyright holder for information about a commercial license if terms
 * of the GNU Affero Public License do not suit you.
 *
 * This copy of InfiniSQL is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with InfiniSQL. It should be in the top level of the source
 * directory in a file entitled "COPYING".
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <cstdlib>
#include <vector>
#include <stdint.h>
#include <pthread.h>
#include <string>
#include <stdio.h>
#include <boost/unordered_map.hpp>
#include "infinisql_cfgenum.h"
#include <map>

using std::string;
using std::map;

extern pthread_mutex_t nodeTopologyMutex;
using std::vector;

class Mbox;

/** topology definition and functions to set and query topology attributes
 */
class Topology
{
public:
  /** Address for each actor in the environment. This is embedded into
   * every message, as source and destination, so should be relatively
   * small.
   */
  struct addressStruct
  {
    int64_t nodeid; /**< node is a server process (not an OS instance) */
    int64_t actorid; /**< each actor has a unique id per node */
  };

  /** Actor address with pointer to Mbox and actor type. Mbox and
   * type are not necessary in message headers. This also contains
   * identification information for each actor instance's self */
  struct partitionAddress
  {
    addressStruct address;
    int64_t instance;
    class Mbox *mbox;
    actortypes_e type; /**< actor type */
    int epollfd;
    string argstring;
    string node;
    string service;
    vector<string> nodes;
    vector<string> services;
  };

  struct actor_s
  {
    actortypes_e type;
    int64_t instance;
    class Mbox *mbox;
  };

  Topology();
  Topology(const Topology &orig);
  virtual ~Topology();

  /** creates a new actor's addressing in the local node. Should be
   * followed by launching the thread */
  partitionAddress *newActor(actortypes_e, class Mbox *, int, const string &,
                             int64_t, const vector<string> &, const vector<string> &);

  int64_t nodeid; /**< this gets propagated to all addresses created by this
                   * class instance */
  int numreplicas;
  int activereplica;

  //local
  size_t numtransactionagents;
  size_t numengines;
  size_t numobgateways;
  size_t numconnectionhandlers;

  vector<actor_s> actorList;
  vector<class ConnectionHandler *> connectionHandlers;

  //global
  size_t numpartitions;
  int64_t userSchemaMgrNode;
  class Mbox *userSchemaMgrMbox;
  int64_t deadlockMgrNode;
  class Mbox *deadlockMgrMbox;
  // partitionList[replicaid][partition] = {nodeid, actorid}
  vector< vector<partitionAddress> > partitionList;
  vector<partitionAddress> partitionListThisReplica;
  map< int64_t, vector<string> > ibGateways;
  // allActors[nodeid][actorid] = type
  vector< vector<int> > allActors;
  boost::unordered_map< int64_t, vector<int> > allActorsThisReplica;
  // replicaMembers[replica][member] = nodeid
  vector< vector<int64_t> > replicaMembers;
  // tas[nodeid][tainstance] = actorid
  vector< vector<int64_t> > tas;
};

#endif  /* TOPOLOGY_H */
