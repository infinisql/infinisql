/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL(tm).
 
 * InfiniSQL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InfiniSQL. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   Topology.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:54:28 2013
 * 
 * @brief  Topology class has all of the actors, their types, and dynamic
 * configuration values. Each actor maintains a Topology object which gives
 * it a common view for the whole node and, as necessary, the whole cluster.
 */

#ifndef INFINISQLTOPOLOGY_H
#define INFINISQLTOPOLOGY_H

#include <cstdlib>
#include <vector>
#include <stdint.h>
#include <pthread.h>
#include <string>
#include <stdio.h>
#include <boost/unordered_map.hpp>
#include "cfgenum.h"
#include <map>

using std::string;
using std::map;

extern pthread_mutex_t nodeTopologyMutex;
extern FILE *logfile;
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
    struct __attribute__ ((__packed__)) addressStruct
    {
        int16_t nodeid; /**< node is a server process (not an OS instance) */
        int16_t actorid; /**< each actor has a unique id per node */
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
        std::string argstring;
        std::string node;
        std::string service;
        std::vector<std::string> nodes;
        std::vector<std::string> services;
    };

    /** 
     * @brief characteristics for each actor
     *
     */
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
    /** 
     * @brief creates a new actor's addressing in the local node
     *
     * should be followed by launching the thread
     *
     * @param type actor type
     * @param mbox Mbox
     * @param epollfd epoll descriptor
     * @param argstring argument field for some actors to use
     * @param actorid actorid
     * @param nodes 
     * @param services 
     *
     * @return 
     */
    partitionAddress *newActor(actortypes_e type, class Mbox *mbox, int epollfd,
                               const string &argstring, int64_t actorid,
                               const vector<string> &nodes,
                               const vector<string> &services);

    int16_t nodeid; /**< this gets propagated to all addresses created by this
                     * class instance */
    int numreplicas;
    int activereplica;

    //local
    size_t numtransactionagents;
    size_t numengines;
    size_t numobgateways;

    vector<actor_s> actorList;

    //global
    int16_t numpartitions;
    int16_t userSchemaMgrNode;
    class Mbox *userSchemaMgrMbox;
    int16_t deadlockMgrNode;
    class Mbox *deadlockMgrMbox;
    // partitionList[replicaid][partition] = {nodeid, actorid}
    std::vector< vector<partitionAddress> > partitionList;
    std::vector<partitionAddress> partitionListThisReplica;
    std::map< int64_t, vector<std::string> > ibGateways;
    // allActors[nodeid][actorid] = type
    std::vector< vector<int> > allActors;
    boost::unordered_map< int16_t, std::vector<int> > allActorsThisReplica;
    // replicaMembers[replica][member] = nodeid
    std::vector< vector<int16_t> > replicaMembers;
    // tas[nodeid][tainstance] = actorid
    std::vector< vector<int16_t> > tas;
};

#endif  /* INFINISQLTOPOLOGY_H */
