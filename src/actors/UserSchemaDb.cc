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
 * @file   UserSchemaDb.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Feb 17 13:01:41 2014
 * 
 * @brief  all metadata objects
 */

#include "UserSchemaDb.h"

UserSchemaDb::UserSchemaDb() : env(nullptr), txn(nullptr), dbi(0),
                               cursor(nullptr)
{
}

UserSchemaDb::UserSchemaDb(MDB_env *env) : env(env), txn(nullptr), dbi(0),
                                           cursor(nullptr)
{
    setTypeNames();
}

void UserSchemaDb::setTypeNames()
{
    dbTypeNames[META_PARTITIONGROUP]="partitiongroups";
    dbTypeNames[META_CATALOG]="catalogs";
    dbTypeNames[META_SCHEMA]="schemata";
    dbTypeNames[META_TABLE]="tables";
    dbTypeNames[META_FIELD]="fields";
    dbTypeNames[META_INDEX]="indices";
    dbTypeNames[META_USER]="users";
}

bool UserSchemaDb::txn_begin()
{
    int retval=mdb_txn_begin(env, nullptr, 0, &txn);
    if (retval)
    {
        LOG("problem mdb_txn_begin " << retval);
        return false;
    }

    return true;
}

void UserSchemaDb::txn_abort()
{
    mdb_txn_abort(txn);
}

bool UserSchemaDb::txn_commit()
{
    int retval=mdb_txn_commit(txn);
    if (retval)
    {
        LOG("problem mdb_txn_commit " << retval);
        mdb_txn_abort(txn);
        return false;
    }

    return true;
}

bool UserSchemaDb::dbi_create(const char *name)
{
    return dbi_open(name, MDB_CREATE);
}

bool UserSchemaDb::dbi_open(const char *name)
{
    return dbi_open(name, 0);
}

bool UserSchemaDb::dbi_open(const char *name, unsigned int openflags)
{
    
    if (txn_begin()==false)
    {
        return false;
    }

    int retval=mdb_dbi_open(txn, name, openflags, &dbi);
    if (retval)
    {
        LOG("mdb_dbi_open: " << name);
        txn_abort();
        return false;
    }

    if (txn_commit()==false)
    {
        return false;
    }

    return true;
}

void UserSchemaDb::dbi_close()
{
    mdb_dbi_close(env, dbi);
}

bool UserSchemaDb::cursor_open()
{
    int retval=mdb_cursor_open(txn, dbi, &cursor);
    if (retval)
    {
        LOG("mdb_cursor_open: " << retval);
        txn_abort();
        return false;
    }

    return true;
}

void UserSchemaDb::cursor_close()
{
    mdb_cursor_close(cursor);
}

bool UserSchemaDb::createdbs()
{
    std::map<metatypes_e, const char *>::iterator it;
    for (it = dbTypeNames.begin(); it != dbTypeNames.end(); ++it)
    {
        if (dbi_create(it->second)==false)
        {
            return false;
        }
    }

    return true;
}

std::vector<int16_t> UserSchemaDb::getkey(MDB_val *key)
{
    std::vector<int16_t> v;
    Serdes serkey(key);
    des(serkey, v);

    return v;
}


void UserSchemaDb::populatemap(metatypes_e metatype, MDB_val *key,
                                MDB_val *val)
{
    Serdes serval(val);
    
    switch(metatype)
    {
    case META_PARTITIONGROUP:
    {
        PartitionGroup meta;
        des(serval, meta);
        partitiongroups[getkey(key)]=meta;
    }
    break;
        
    case META_CATALOG:
    {
        Catalog meta;
        des(serval, meta);
        catalogs[getkey(key)]=meta;
    }
    break;
        
    case META_SCHEMA:
    {
        Schema meta;
        des(serval, meta);
        schemata[getkey(key)]=meta;
    }
    break;
        
    case META_TABLE:
    {
        Table meta;
        des(serval, meta);
        tables[getkey(key)]=meta;
    }
    break;
        
    case META_FIELD:
    {
        Field meta;
        des(serval, meta);
        fields[getkey(key)]=meta;
    }
    break;
        
    case META_INDEX:
    {
        Index meta;
        des(serval, meta);
        indices[getkey(key)]=meta;
    }
    break;
        
    case META_USER:
    {
        User meta;
        des(serval, meta);
        users[getkey(key)]=meta;
    }
    break;

    default:
        LOG("can't handle type " << metatype);
    }
}

bool UserSchemaDb::load()
{
    std::map<metatypes_e, const char *>::iterator it;
    for (it = dbTypeNames.begin(); it != dbTypeNames.end(); ++it)
    {
        if (dbi_open(it->second)==false)
        {
            return false;
        }

        if (txn_begin()==false)
        {
            return false;
        }
        if (cursor_open()==false)
        {
            return false;
        }
        MDB_val key;
        MDB_val val;
        int retval=mdb_cursor_get(cursor, &key, &val, MDB_FIRST);
        while(!retval)
        {
            populatemap(it->first, &key, &val);
            retval=mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        }
        cursor_close();
        txn_abort();
        dbi_close();
    }

    return true;
}
