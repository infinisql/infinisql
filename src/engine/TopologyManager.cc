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
 * @file   TopologyManager.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Wed Jan 22 09:17:20 2014
 * 
 * @brief  Actor which receives configuration commands over 0mq and MessagePack
 * from infinisqlmgr.py.
 * 
 * Launches all other actors, updates Topology, and distributes configuration
 * changes to all other actors on each node. Facilitates dynamic
 * reconfiguration.
 */

#include "TopologyManager.h"

TopologyManager::TopologyManager(Actor::identity_s identity) : Actor(identity)
{
}

void TopologyManager::operator()()
{
    identity.mbox=new (std::nothrow) Mbox;
    if (identity.mbox==NULL)
    {
        LOG("can't create Mbox");
        exit(1);
    }

    newActor(ACTOR_ADMIN_LISTENER, 17, 0);

    while(1)
    {
        sleep(10);
    }
}

void TopologyManager::newActor(actortypes_e type, int16_t actorid,
                               int16_t instance)
{
    identity_s newidentity={};
    newidentity.address={identity.address.nodeid, actorid};
    newidentity.instance=instance;
    newidentity.mbox=new (std::nothrow) Mbox;
    if (newidentity.mbox==NULL)
    {
        LOG("can't create Mbox");
        exit(1);
    }
    newidentity.epollfd=identity.epollfd;
    newidentity.zmqhostport=identity.zmqhostport;

    std::thread t;

    switch (type)
    {
    case ACTOR_TRANSACTIONAGENT:
        t=std::thread{TransactionAgent(newidentity)};
        break;
        
    case ACTOR_PARTITION_WRITER:
        t=std::thread{PartitionWriter(newidentity)};
        break;
        
    case ACTOR_TRANSACTION_LOGGER:
        t=std::thread{TransactionLogger(newidentity)};
        break;
        
    case ACTOR_USERSCHEMAMGR:
        t=std::thread{UserSchemaManager(newidentity)};
        break;
        
    case ACTOR_OBGATEWAY:
        t=std::thread{ObGateway(newidentity)};
        break;
        
    case ACTOR_IBGATEWAY:
        t=std::thread{IbGateway(newidentity)};
        break;
        
    case ACTOR_LISTENER:
        t=std::thread{Listener(newidentity)};
        break;
        
    case ACTOR_ADMIN_LISTENER:
        t=std::thread{AdminListener(newidentity)};
        break;

    default:
        LOG("don't know how to start actor type " << type);
    }

    if (t.joinable())
    {
        t.detach();
    }
}
