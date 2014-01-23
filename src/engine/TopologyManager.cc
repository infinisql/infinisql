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
    /* launch a thread by:
     * std::thread t{ClassName(Actor::identity_s)};
     * That will execute its constructor and its operator()
     */
    identity.mbox=new (std::nothrow) Mbox;
    std::cout << "Mbox: " << (void *)identity.mbox << std::endl;
    while(1)
    {
        sleep(10);
    }
}
