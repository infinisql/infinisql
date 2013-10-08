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

#include "infinisql_gch.h"
#include "infinisql_Applier.h"
#include "infinisql_TransactionAgent.h"
#line 30 "Applier.cc"

int64_t applierid;
class TransactionAgent *taPtr;
Topology::addressStruct sourceAddr;
int64_t partitioncount;

Applier::Applier(class TransactionAgent *taPtrarg, int64_t domainidarg,
                 Topology::addressStruct sourceAddrarg, int64_t partitioncountarg) :
  taPtr(taPtrarg), domainid(domainidarg), sourceAddr(sourceAddrarg),
  partitioncount(partitioncountarg)
{
  applierid = taPtr->getnextapplierid();
  taPtr->Appliers[applierid] = this;
}

Applier::~Applier()
{
}

void Applier::ackedApply(class MessageAckApply &msg)
{
  if (msg.ackapplyStruct.status != STATUS_OK)
  {
    printf("%s %i bad apply ack status %i %li,%li,%li\n", __FILE__, __LINE__,
           msg.ackapplyStruct.status, msg.ackapplyStruct.subtransactionid, msg.ackapplyStruct.applierid, msg.ackapplyStruct.partitionid);
    delete this;
    return;
  }

  if (!--partitioncount)
  {
    taPtr->Appliers.erase(applierid);
    delete this;
  }
}
