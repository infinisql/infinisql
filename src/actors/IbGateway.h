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
 * @file   IbGateway.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Sat Jan 25 08:36:23 2014
 * 
 * @brief  Inbound Gateway actor. Counterpart to ObGateway. Receives messages
 * over the network from remote senders and distributes them to their
 * destination actors on the current node.
 */

#ifndef INFINISQLIBGATEWAY_H
#define INFINISQLIBGATEWAY_H

#include "Actor.h"

class IbGateway : public Actor
{
public:
    IbGateway(Actor::identity_s identity);
    ~IbGateway();
    void operator()();

    /** 
     * @brief decompress and distribute incoming messages
     *
     * @param buf bufer to read from
     * @param bufsize size of buffer
     */
    void inbufhandler(const char *buf, size_t bufsize);

    char *inbuf;
    char *dcstrsmall;
    std::unordered_map<int, std::string> pendingReads;
};

#endif // INFINISQLIBGATEWAY_H
