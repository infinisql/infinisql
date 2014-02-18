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
 * @file   Metadata.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 08:47:01 2014
 * 
 * @brief  base class for metadata definitions
 *
 * for things such as catalogs, schemata, tables, users, indices, fields
 */

#include "Metadata.h"

Metadata::Metadata() : id(-1), parentCatalog(nullptr), parentSchema(nullptr),
                       parentTable(nullptr), parentcatalogid(-1),
                       parentschemaid(-1), parenttableid(-1), versionid(-1),
                       lmdbinfo({nullptr, nullptr, nullptr, 0})
{
    
}

Metadata::Metadata(int16_t id, std::string name)
    : id(id), name(name), parentCatalog(nullptr), parentSchema(nullptr),
      parentTable(nullptr), parentcatalogid(-1), parentschemaid(-1),
      parenttableid(-1), versionid(-1),
      lmdbinfo({nullptr, nullptr, nullptr, 0})
{
    
}

Metadata::Metadata(const Metadata &orig)
{
    id=orig.id;
    name=orig.name;
    parentCatalog=nullptr;
    parentSchema=nullptr;
    parentTable=nullptr;
    parentcatalogid=orig.parentcatalogid;
    parentschemaid=orig.parentschemaid;
    parenttableid=orig.parenttableid;
    versionid=orig.versionid;
}

Metadata::~Metadata() {
}

int Metadata::dbOpen(unsigned int flags)
{
    char dbname[sizeof(parentcatalogid)+sizeof(parentschemaid)+sizeof(id)+1]={};
    memcpy(dbname, &parentcatalogid, sizeof(parentcatalogid));
    memcpy(dbname+sizeof(parentcatalogid), &parentschemaid,
           sizeof(parentschemaid));
    memcpy(dbname+sizeof(parentcatalogid)+sizeof(parentschemaid), &id,
           sizeof(id));

    int retval=mdb_txn_begin(lmdbinfo.env, nullptr, 0, &lmdbinfo.txn);
    if (retval)
    {
        return retval;
    }
    retval=mdb_dbi_open(lmdbinfo.txn, dbname, MDB_CREATE | flags,
                        &lmdbinfo.dbi);
    if (retval)
    {
        return retval;
    }
    retval=mdb_txn_commit(lmdbinfo.txn);
    return retval;
}

void Metadata::dbClose()
{
    mdb_dbi_close(lmdbinfo.env, lmdbinfo.dbi);
}

int Metadata::dbEmpty()
{
    int retval=mdb_txn_begin(lmdbinfo.env, nullptr, 0, &lmdbinfo.txn);
    if (retval)
    {
        return retval;
    }
    retval=mdb_drop(lmdbinfo.txn, lmdbinfo.dbi, 0);
    if (retval)
    {
        return retval;
    }
    retval=mdb_txn_commit(lmdbinfo.txn);
    return retval;    
}

int Metadata::dbDrop()
{
    int retval=mdb_txn_begin(lmdbinfo.env, nullptr, 0, &lmdbinfo.txn);
    if (retval)
    {
        return retval;
    }
    retval=mdb_drop(lmdbinfo.txn, lmdbinfo.dbi, 1);
    if (retval)
    {
        return retval;
    }
    retval=mdb_txn_commit(lmdbinfo.txn);
    return retval;    
}

void ser(const Metadata &d, Serdes &output)
{
    ser(d.id, output);
    ser(d.name, output);
    ser(d.parentcatalogid, output);
    ser(d.parentschemaid, output);
    ser(d.parenttableid, output);
    ser(d.versionid, output);
}

size_t sersize(const Metadata &d)
{
    return sersize(d.id) + sersize(d.name) + sersize(d.parentcatalogid) +
        sersize(d.parentschemaid) + sersize(d.parenttableid) +
        sersize(d.versionid);
}

void des(Serdes &input, Metadata &d)
{
    des(input, d.id);
    des(input, d.name);
    des(input, d.parentcatalogid);
    des(input, d.parentschemaid);
    des(input, d.parenttableid);
    des(input, d.versionid);
}
