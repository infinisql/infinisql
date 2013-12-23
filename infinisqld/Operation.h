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
 * @file   Operation.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:37:35 2013
 * 
 * @brief  Class for non-transactional, user and schema-related activities,
 * such as login, createuser, createtable, and so on.
 */

#ifndef INFINISQLOPERATION_H
#define INFINISQLOPERATION_H

#include "gch.h"
#include "TransactionAgent.h"

// usm: waiting for usm to reply, tasengines: waiting for replies
enum state_schema_e { usm, tasengines };
typedef struct
{
    int builtincmd;
    state_schema_e state;
    int64_t msgwaits;
} schemastruct;

/** 
 * @brief non-transactional user and schema-related activity
 *
 * @param typearg type of operation
 * @param taarg TransactionAgent
 * @param uid userid
 * @param did domainid
 */
class Operation
{
public:
    Operation(int typearg, class TransactionAgent *taarg, int64_t uid,
              int64_t did);
    virtual ~Operation();

    friend class TransactionAgent;

    //private:
    /** 
     * @brief "builtin" is the operation function to perform
     *
     * @param cmd command, one of BUILTIN* macros defined in defs.h
     */
    void setbuiltincmd(int cmd);
    /** 
     * @brief set domain name
     *
     * @param name domain name
     */
    void setDomainName(string name);
    /** 
     * @brief get operation identifier
     *
     *
     * @return operation identifier
     */
    int64_t getid();
    /** 
     * @brief perform operation (only for SQL logins)
     *
     * @param msgrcvref received MessageUserSchema
     */
    void handleOperation(class MessageUserSchema &msgrcvref);

    int type;
    class TransactionAgent *taPtr;
    int64_t operationid;
    int sockfd;
    int64_t userid;
    int64_t domainid;
    string domainName; // for login
    schemastruct schemaData;
};

#endif  /* INFINISQLOPERATION_H */
