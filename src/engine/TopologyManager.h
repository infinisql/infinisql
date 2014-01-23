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

#include <thread>
#include "Actor.h"

class TopologyManager : public Actor
{
public:
    TopologyManager(Actor::identity_s identity);
    void operator()();
};

#endif // INFINISQLTOPOLOGYMANAGER_H
