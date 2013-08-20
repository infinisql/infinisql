/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL (tm). It is available either under the
 * GNU Affero Public License or under a commercial license. Contact the
 * copyright holder for information about a commercial license if terms
 * of the GNU Affero Public License do not suit you.
 *
 * This copy of InfiniSQL is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with InfiniSQL. It should be in the top level of the source
 * directory in a file entitled "COPYING".
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "infinisql_Topology.h"
#line 28 "Topology.cc"

Topology::Topology() : nodeid(0), numreplicas(1), activereplica(-1),
  numtransactionagents(0),
  numengines(0), numobgateways(0), numpartitions(0), userSchemaMgrNode(0),
  userSchemaMgrMbox(0), deadlockMgrNode(0), deadlockMgrMbox(0)
{
}

Topology::Topology(const Topology &orig)
{
}

Topology::~Topology()
{
}

Topology::partitionAddress *Topology::newActor(actortypes_e type,
    class Mbox *mbox, int epollfd, const string &argstring,
    int64_t actorid, const vector<string> &nodes,
    const vector<string> &services)
{
  partitionAddress *addr = new partitionAddress();
  partitionAddress &addrRef = *addr;
  addrRef.type = type;
  addrRef.mbox = mbox;
  addrRef.address.nodeid = nodeid;
  addrRef.address.actorid = actorid;
  addrRef.epollfd = epollfd;
  addrRef.nodes = nodes;
  addrRef.services = services;
  addrRef.argstring = argstring;

  if (addr==NULL)
  {
    printf("%s %i ERROR\n", __FILE__, __LINE__);
  }

  return addr;
}
