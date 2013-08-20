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

#ifndef SCHEMA_HPP
#define SCHEMA_HPP

#include "infinisql_gch.h"
#include "infinisql_Table.h"

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
  int64_t getnexttableid(void);
  int createTable(int64_t);

  int64_t domainid;
  int64_t nexttableid;
  boost::unordered_map< int64_t, class Table *> tables;
  boost::unordered_map< string, int64_t > tableNameToId;
  // fieldNameToId[tableid][fieldname] = fieldid
  boost::unordered_map< int64_t, boost::unordered_map<string, int64_t> >
  fieldNameToId;
};

template < typename T >
void createSchema(T servent)
{
  class MessageUserSchema &msgref =
        *(class MessageUserSchema *)servent->msgrcv;

  if (servent->domainidsToSchemata.count(msgref.domainid))
{
    servent->status = BUILTIN_STATUS_NOTOK;
  }
  else
  {
    class Schema *sptr = new class Schema(msgref.domainid);
    servent->domainidsToSchemata[msgref.domainid] = sptr;
    servent->status = BUILTIN_STATUS_OK;
  }
}

#endif  /* SCHEMA_HPP */
