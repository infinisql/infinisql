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
 * @file   Actor.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 20 22:14:09 2014
 * 
 * @brief  base class for Actors
 */

#ifndef INFINISQLACTOR_H
#define INFINISQLACTOR_H

#include <thread>
#include "../mbox/Mbox.h"
#include "../engine/global.h"
#include "Topology.h"

#define GWBUFSIZE 16777216

extern std::atomic<int64_t> *socketAffinity;

class UserSchemaOperation;

class Actor
{
public:
    /** 
     * @brief identifying characteristics for an actor
     */
    struct identity_s
    {
        Message::address_s address;
        int16_t instance;
        Mbox *mbox;
        int epollfd;
        std::string zmqhostport;
        int sockfd;
        MDB_env *env;
        std::string transactionlogpath;
    };

    Actor(identity_s identity);
    void operator()() const;
    virtual ~Actor();

    int64_t getnextuserschemaoperationdid();
    /** 
     * @brief read next available message, deserialize it if from remote node
     *
     * places message in msgrcv
     *
     * @param timeout timeout to wait in microseconds
     */
    void getmsg(int timeout);
    /** 
     * @brief send MessageBatch to ObGateway, should be called between mbox
     * receive sets
     */
    void sendObBatch();
    /** 
     * @brief send message. if local, put in recipient's mailbox. if remote,
     * create and/or append to batch for ObGateway
     *
     * @param msg message to send
     */
    void sendMsg(Message &msg);
    /** 
     * @brief set sender and destination addresses on message header
     *
     * @param sourceAddress sender address (this actor)
     * @param destinationAddress destination address
     * @param msg message
     */
    void setEnvelope(const Message::address_s &sourceAddress,
                     const Message::address_s &destinationAddress,
                     Message &msg);
    /** 
     * @brief send message to specific actor
     *
     * also populates sender address
     *
     * @param destinationAddress destination actor address
     * @param msg message to send
     */
    void toActor(const Message::address_s &destinationAddress, Message &msg);
    /** 
     * @brief send message to UserSchemaManager, and populates sender address
     *
     * @param msg message to send
     */
    void toUserSchemaManager(Message &msg);

    identity_s identity;
    Message *msgrcv;
    TopologyDistinct myTopology;
    int64_t nextuserschemaoperationid;
    std::unordered_map<int64_t, UserSchemaOperation *> userSchemaOperations;

    Message reuseMessage;
    MessageSocket reuseMessageSocket;
    MessageUserSchema reuseMessageUserSchema;
};

#endif // INFINISQLACTOR_H
