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
 * @file   TopologyMgr.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:59:59 2013
 * 
 * @brief  Actor which receives configuration commands over 0mq and MessagePack
 * from infinisqlmgr.py.
 *
 * Launches all other actors, updates Topology, and distributes configuration
 * changes to all other actors on each node. Facilitates dynamic
 * reconfiguration.
 */

#ifndef INFINISQLTOPOLOGYMGR_H
#define INFINISQLTOPOLOGYMGR_H

#include "gch.h"
#include "cfgenum.h"

/** 
 * @brief launch Topology Manager actor
 *
 * @param identity how to identify actor instance
 *
 * @return 
 */
void *topologyMgr(void *identity);

/** 
 * @brief execute Topology Manager actor
 *
 * @param myIdentityArg how to identify this 
 */
class TopologyMgr
{
public:
    TopologyMgr(Topology::actorIdentity *myIdentityArg);
    TopologyMgr(const TopologyMgr &orig);
    virtual ~TopologyMgr();
private:
    /** 
     * @brief local (node-specific) Topology update
     *
     * transmitted by manager process over 0mq serialized by msgpack
     *
     * @param pac msgpack unpacker object
     * @param result result of unpacking
     */
    void updateLocalConfig(msgpack::unpacker &pac, msgpack::unpacked &result);
    /** 
     * @brief global (cluster wide) Topology update
     *
     * transmitted by manager process over 0mq serialized by msgpack
     *
     * @param pac msgpack unpacker object
     * @param result result of unpacking
     */
    void updateGlobalConfig(msgpack::unpacker &pac, msgpack::unpacked &result);
    /** 
     * @brief tell all local actors to update their versions of Topology
     *
     */
    void broadcastConfig();

    Topology::actorIdentity myIdentity;
    class Mboxes mboxes;
    class Topology myTopology;
};

#endif  /* INFINISQLTOPOLOGYMGR_H */
