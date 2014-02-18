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
 * @file   UserSchemaDb.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Feb 17 12:55:31 2014
 * 
 * @brief  all metadata objects
 */

#ifndef INFINISQL_USERSCHEMADB_H
#define INFINISQL_USERSCHEMADB_H

#include "../engine/Metadata.h"
#include "../engine/Catalog.h"
#include "../engine/Schema.h"
#include "../engine/Table.h"
#include "../engine/Field.h"
#include "../engine/Index.h"
#include "../engine/User.h"
#include "../engine/PartitionGroup.h"

#include <map>
#include <cstdint>
#include <vector>
#include <lmdb.h>

/* Format of DB on LMDB environment:
 *
 * db names: key format (no commas)
 * partitiongroups: partitiongroupid
 * catalogs: catalogid
 * schemata: catalogid,schemaid
 * tables: catalogid,tableid,versionid
 * fields: catalogid,tableid,fieldid
 * indices: catalogid,indexid,versionid
 * users: catalogid,userid
 *
 * all id's are int16_t, binary, host-endian
 * value of each entry for metadata objects is created by ser() for that
 * object
 */


class UserSchemaDb
{
public:
    enum metatypes_e
    {
        META_NONE = 0,
        META_PARTITIONGROUP,
        META_CATALOG,
        META_SCHEMA,
        META_TABLE,
        META_FIELD,
        META_INDEX,
        META_USER
    };
    UserSchemaDb();
    // belongs to UserSchemaManager:
    UserSchemaDb(MDB_env *env);

    void setTypeNames();
    bool txn_begin();
    void txn_abort();
    bool txn_commit();
    bool dbi_create(const char *name);
    bool dbi_open(const char *name);
    bool dbi_open(const char *name, unsigned int openflags);
    void dbi_close();
    bool cursor_open();
    void cursor_close();
    bool createdbs();
    std::vector<int16_t> getkey(MDB_val *key);
    void populatemap(metatypes_e metatype, MDB_val *key, MDB_val *val);
    bool load();

    MDB_env *env;
    MDB_txn *txn;
    MDB_dbi dbi;
    MDB_cursor *cursor;

    std::map< std::vector<int16_t>, PartitionGroup > partitiongroups;
    std::map< std::vector<int16_t>, Catalog > catalogs;
    std::map< std::vector<int16_t>, Schema > schemata;
    std::map< std::vector<int16_t>, Table > tables;
    std::map< std::vector<int16_t>, Field > fields;
    std::map< std::vector<int16_t>, Index > indices;
    std::map< std::vector<int16_t>, User > users;

    std::map<metatypes_e, const char *> dbTypeNames;
};

#endif // INFINISQL_USERSCHEMADB_H
