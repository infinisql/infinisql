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

#ifndef OPERATION_H
#define OPERATION_H

#include "infinisql_gch.h"
#include "infinisql_TransactionAgent.h"

// usm: waiting for usm to reply, tasengines: waiting for replies
enum state_schema_e { usm, tasengines };
typedef struct
{
    int builtincmd;
    state_schema_e state;
    int64_t msgwaits;
} schemastruct;

class Operation
{
public:
    Operation(int, class TransactionAgent *, int64_t, int64_t);
    virtual ~Operation();

    friend class TransactionAgent;

    //private:
    void setbuiltincmd(int);
    void setDomainName(string);
    int64_t getid(void);
    void handleOperation(class MessageUserSchema &);
    int type;
    class TransactionAgent *taPtr;
    int64_t operationid;
    int sockfd;
    int64_t userid;
    int64_t domainid;
    string domainName; // for login
    schemastruct schemaData;
};

#endif  /* OPERATION_H */
