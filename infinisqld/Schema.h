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
 * @file   Schema.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:46:55 2013
 * 
 * @brief  Schema class. Contains tables (fields (indices)). One schemata per
 * domain. Corresponds to tablespace or database on other RDBMS platforms.
 */

#ifndef INFINISQLSCHEMA_H
#define INFINISQLSCHEMA_H

#include "gch.h"
#include "Table.h"

class Schema
{
public:
    Schema(int64_t);
    virtual ~Schema();

    friend class ApiInterface;
    friend class TransactionAgent;
    friend class Engine;
    friend class Transaction;
    friend class SubTransaction;
    friend class UserSchemaMgr;

    //private:
    int64_t getnexttableid();
    int createTable(int64_t id);

    int64_t domainid;
    int64_t nexttableid;
    boost::unordered_map< int64_t, class Table *> tables;
    boost::unordered_map< std::string, int64_t > tableNameToId;
    // fieldNameToId[tableid][fieldname] = fieldid
    boost::unordered_map< int64_t, boost::unordered_map<std::string, int64_t> >
        fieldNameToId;
};

template < typename T >
void createSchema(T servent)
{
    class MessageUserSchema &msgref =
        *(class MessageUserSchema *)servent->msgrcv;

    if (servent->domainidsToSchemata.count(msgref.userschemaStruct.domainid))
    {
        servent->status = BUILTIN_STATUS_NOTOK;
    }
    else
    {
        class Schema *sptr = new class Schema(msgref.userschemaStruct.domainid);
        servent->domainidsToSchemata[msgref.userschemaStruct.domainid] = sptr;
        servent->status = BUILTIN_STATUS_OK;
    }
}

#endif  /* INFINISQLSCHEMA_H */
