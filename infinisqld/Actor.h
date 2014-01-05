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
 * @date   Wed Jan  1 14:26:39 2014
 * 
 * @brief  base class for actors
 *
 * actors in InfiniSQL are long-running event-driven threads. Each
 * communicates with other actors through asynchronous messaging. Most
 * actors' main event loop process mailbox messages.
 */

#ifndef INFINISQLACTOR_H
#define INFINISQLACTOR_H

#include "gch.h"

/** 
 * @brief base class for actor types
 *
 * actors in InfiniSQL are long-running event-driven threads. Each
 * communicates with other actors through asynchronous messaging. Most
 * actors' main event loop process mailbox messages.
 */
class Actor
{
public:
    Actor();
    virtual ~Actor();

    /** 
     * @brief set up data for actors
     *
     * @param myIdentityArg actor identity passed upon instantiation
     */
    void init(Topology::actorIdentity *myIdentityArg);
    /** 
     * @brief get and deserialize next message from mbox
     *
     * deserialize messages from remote nodes
     *
     * @param timeout timeout parameter to MboxProducer::send
     */
    void getmsg(int timeout);
    
    class Message *msgrcv;
    class Mboxes mboxes;
    Topology::actorIdentity myIdentity;
    class Topology myTopology;

    class Message reuseMessage;
    class MessageSocket reuseMessageSocket;
    class MessageUserSchema reuseMessageUserSchema;
    class MessageDeadlock reuseMessageDeadlock;
    class MessageSubtransactionCmd reuseMessageSubtransactionCmd;
    class MessageCommitRollback reuseMessageCommitRollback;
    class MessageDispatch reuseMessageDispatch;
    class MessageAckDispatch reuseMessageAckDispatch;
    class MessageApply reuseMessageApply;
    class MessageAckApply reuseMessageAckApply;

    
};

#endif  /* INFINISQLACTOR_H */
