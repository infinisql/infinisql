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
 * @date   Tue Dec 17 13:20:37 2013
 * 
 * @brief  Inbound Gateway actor. Counterpart to ObGateway. Receives messages
 * over the network from remote senders and distributes them to their
 * destination actors on the current node.
 */

#ifndef INFINISQLIBGATEWAY_H
#define INFINISQLIBGATEWAY_H

#include "gch.h"
#include "Actor.h"

/** 
 * @brief Inbound Gateway Actor
 */
class IbGateway : public Actor
{
public:
    /** 
     * @brief execute Inbound Gateway
     *
     * receives Message batches from other nodes, decompresses and
     * distributes to local actors.
     *
     * @param myIdentityArg how to identify this
     */
    IbGateway(Topology::actorIdentity *myIdentityArg);
    virtual ~IbGateway();

    /*
    Topology::actorIdentity myIdentity;
    class Mboxes mboxes;
    class Topology myTopology;
    */

private:
    /** 
     * @brief process compressed batch of incoming messages
     *
     * @param buf incoming message buffer
     * @param bufsize buffer size
     */
    void inbufhandler(const char *buf, size_t bufsize);
    /** 
     * @brief add file descriptor to remote ObGateway to poll structure
     *
     * @param newfd socket descriptor
     */
    void addtofds(int newfd);
    /** 
     * @brief remove file descriptors from poll struct
     *
     */
    void removefds();

    boost::unordered_map<int, std::string> pendingReads;
    boost::unordered_set<int> fdremoveset;
    struct pollfd *fds;
    nfds_t nfds;
    char *inbuf;
    char *dcstrsmall;
    bool ismultinode;
};

/** 
 * @brief launch IbGateway actor
 *
 * @param identity how to identify actor instance
 *
 * @return 
 */
void *ibGateway(void *identity);

#endif  /* INFINISQLIBGATEWAY_H */
