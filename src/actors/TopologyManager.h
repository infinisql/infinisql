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
 * @file   TopologyManager.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Wed Jan 22 09:11:59 2014
 * 
 * @brief  Actor which receives configuration commands over 0mq and MessagePack
 * from infinisqlmgr.py.
 * 
 * Launches all other actors, updates Topology, and distributes configuration
 * changes to all other actors on each node. Facilitates dynamic
 * reconfiguration.
 */

#ifndef INFINISQLTOPOLOGYMANAGER_H
#define INFINISQLTOPOLOGYMANAGER_H

#include "Actor.h"
#include "AdminListener.h"
#include "TransactionAgent.h"
#include "PartitionWriter.h"
#include "TransactionLogger.h"
#include "UserSchemaManager.h"
#include "ObGateway.h"
#include "IbGateway.h"
#include "Listener.h"

class TopologyManager : public Actor
{
public:
    TopologyManager(Actor::identity_s identity);
    void operator()();

    /** 
     * @brief start socket listener for Listener or IbGateway
     *
     * @param node node host/ip
     * @param service tcp port/service
     * @param isibgw set up socket for IbGateway
     * 
     * @return sockfd
     */
    int startSocket(std::string &node, std::string &service, bool isibgw);
    /** 
     * @brief open or create LMDB environment
     *
     * @param env environment handle
     * @param mapsize size in bytes (should be multiple of OS page size)
     * @param readers max reader threads
     * @param dbs number of db's (92bytes per db per thread memory usage)
     */
    void openPartition(MDB_env *env, size_t mapsize, unsigned int readers,
                       MDB_dbi dbs, std::string &path);
    /** 
     * @brief configure a few members of the identity structure for a
     * new actor
     *
     * @param actorid new actor's id
     * @param instance new actor's instance
     * @param newidentity new actor's identity structure
     */
    void setIdentity(int16_t actorid, int16_t instance,
                     identity_s &newidentity);

    /** 
     * @brief start actor thread
     *
     * @param actor functor with argument
     *
     * @return success (true) or failure (false)
     */
    template <typename T>
    bool startThread(T actor)
    {
        std::thread t;
        try
        {
            t=std::thread{actor};
        }
        catch(const std::system_error &e)
        {
            LOG("start thread problem code " << e.code() << " meaning " <<
                e.what());
            return false;
        }
        if (t.joinable())
        {
            t.detach();
            return true;
        }
        LOG("thread is not joinable");
        return false;            
    }

    /** 
     * @brief start TransactionAgent actor
     *
     * @param actorid actorid
     * @param instance instance
     *
     * @return success or failure
     */
    bool startTransactionAgent(int16_t actorid, int16_t instance);
    /** 
     * @brief start PartitionWriter actor
     *
     * @param actorid actorid
     * @param instance instance
     * @param mapsize size in bytes for the LMDB environment
     * @param readers number of concurrent readers for LMDB environment
     * @param dbs number of keystores in LMDB environment
     * @param path filesystem path for LMDB environment
     *
     * @return success or failure
     */
    bool startPartitionWriter(int16_t actorid, int16_t instance,
                              size_t mapsize, unsigned int readers,
                              MDB_dbi dbs, std::string &path);
    /** 
     * @brief start TransactionLogger actor
     *
     * @param actorid actorid
     * @param instance instance
     * @param path path to transaction log directory
     *
     * @return success or failure
     */
    bool startTransactionLogger(int16_t actorid, int16_t instance,
                                std::string &path);
    /** 
     * @brief start UserSchemaManager actor
     *
     * @param actorid actorid
     * @param path filesystem path for LMDB environment
     *
     * @return success or failure
     */
    bool startUserSchemaManager(int16_t actorid, std::string &path);
    /** 
     * @brief start AdminListener actor
     *
     * @param actorid actorid
     * @param zmqhostport 0mq host:port to listen on
     *
     * @return success or failure
     */
    bool startAdminListener(int16_t actorid, std::string &zmqhostport);
    /** 
     * @brief start ObGateway actor
     *
     * @param actorid actorid
     * @param instance instance
     *
     * @return success or failure
     */
    bool startObGateway(int16_t actorid, int16_t instance);
    /** 
     * @brief start IbGateway actor
     *
     * @param actorid actorid
     * @param instance instance
     * @param node host/IP node
     * @param service TCP port/service
     *
     * @return success or failure
     */
    bool startIbGateway(int16_t actorid, int16_t instance, std::string &node,
                        std::string &service);
    /** 
     * @brief start Listener actor
     *
     * @param actorid actorid
     * @param node host/IP node string
     * @param service TCP port/service
     *
     * @return 
     */
    bool startListener(int16_t actorid, std::string &node,
                        std::string &service);    
};

#endif // INFINISQLTOPOLOGYMANAGER_H
