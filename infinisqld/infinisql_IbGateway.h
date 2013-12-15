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

#ifndef INFINISQLIBGATEWAY_H
#define INFINISQLIBGATEWAY_H

#include "infinisql_gch.h"

class IbGateway
{
public:
    IbGateway(Topology::partitionAddress *);
    virtual ~IbGateway();

    Topology::partitionAddress myIdentity;
    class Mboxes mboxes;
    class Topology myTopology;

private:
    void inbufhandler(const char *, size_t);
    boost::unordered_map<int, std::string> pendingReads;
  
    void addtofds(int);
    void removefds();

    boost::unordered_set<int> fdremoveset;
    struct pollfd *fds;
    nfds_t nfds;
    char *inbuf;
    char *dcstrsmall;
    bool ismultinode;
};

void *ibGateway(void *);

#endif  /* INFINISQLIBGATEWAY_H */
