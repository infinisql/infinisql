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
 * @file   Operation.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:36:50 2013
 * 
 * @brief  Class for non-transactional, user and schema-related activities,
 * such as login, createuser, createtable, and so on.
 */

#include "Operation.h"
#include "Pg.h"
#line 32 "Operation.cc"

Operation::Operation(int typearg, class TransactionAgent *taarg, int64_t uid,
                     int64_t did) : type(typearg), taPtr(taarg), userid(uid),
                                    domainid(did)
{
    class TransactionAgent &taRef = *taPtr;
    operationid = ++taRef.operationidcounter;
    taRef.pendingOperations[operationid] = this;
    sockfd = taRef.sockfd;

    if (type==OP_SCHEMA)
    {
        schemaData.msgwaits =
            __sync_add_and_fetch(&nodeTopology.numtransactionagents, 0) +
            __sync_add_and_fetch(&nodeTopology.numengines, 0);
    }
}

Operation::~Operation()
{
}

int64_t Operation::getid()
{
    return operationid;
}

void Operation::setbuiltincmd(int cmd)
{
    schemaData.builtincmd = cmd;
}

void Operation::setDomainName(string name)
{
    domainName = name;
}

void Operation::handleOperation(class MessageUserSchema &msgrcvref)
{
    switch (msgrcvref.userschemaStruct.caller)
    {
    case 1: // Pg login
    {
        boost::unordered_map<int, class Pg *>::iterator it;
        it = taPtr->Pgs.find(sockfd);

        if (it != taPtr->Pgs.end())
        {
            class Pg &pgref = *it->second;
            pgref.continueLogin(msgrcvref.userschemaStruct.callerstate, msgrcvref);
            taPtr->pendingOperations.erase(operationid);
            delete this;
        }
        else
        {
            printf("%s %i anomaly Pg object not in Pgs, sockfd %i\n", __FILE__,
                   __LINE__, sockfd);
        }
    }
    break;

    default:
        printf("%s %i anomaly type %i\n", __FILE__, __LINE__, type);
    }
}
