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
 * @file   Metadata.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 07:37:51 2014
 * 
 * @brief  base class for metadata definitions
 *
 * for things such as catalogs, schemata, tables, users, indices, fields
 */

#ifndef INFINISQLMETADATA_H
#define INFINISQLMETADATA_H

#include <memory>
#include "../mbox/Serdes.h"
#include <unordered_map>
#include <unordered_set>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <vector>
#include <limits>

using std::string;

class PartitionGroup;
class Catalog;
class Schema;
class Table;

class Metadata
{
public:
    /** 
     * @brief environment and database handles for LMDB instance
     *
     */
    struct lmdbinfo_s
    {
        MDB_env *env;
        MDB_txn *txn;
        MDB_cursor *cursor;
        MDB_dbi dbi;
    };
    
    Metadata();
    Metadata(const Metadata &orig);
    virtual ~Metadata();

    template < typename T, typename U >
    static int16_t getnextid(const T &map, U &nextid)
    {
        int16_t tmpid=nextid;
        int16_t startid=nextid;

        while (1)
        {
            if (tmpid++==0)
            {
                continue;
            }
            if (!map.count(tmpid))
            {
                nextid=tmpid;
                return nextid;
            }
            if (startid==tmpid) // have circled around to no avail
            {
                LOG("no available ID's " << startid);
                return 0;
            }
        }
    }

    virtual void getdbname(char *dbname) {dbname=nullptr; return;}
    template < typename T, typename U, typename V >
    void getdbname2(T dbtype, U catalogid, V schemaid, char *dbname)
    {
        dbname[0]=dbtype;
        int pos=1;
        memcpy(dbname+pos, &catalogid, sizeof(catalogid));
        pos+=sizeof(catalogid);    
        memcpy(dbname+pos, &schemaid, sizeof(schemaid));
        pos+=sizeof(schemaid);
        memcpy(dbname+pos, &id, sizeof(id));
        pos+=sizeof(id);
        memcpy(dbname+pos, &versionid, sizeof(versionid));
        dbname[pos]='\0';
    }
    /** 
     * @brief create new or open existing database for table
     *
     * called by Table and Index dbopens
     *
     * @param flags flags to append to mdb_dbi_open() flags
     *
     * @return return value from mdb_dbi_open(), or transaction problem
     */
    int dbOpen(unsigned int flags);
    /** 
     * @brief close database
     */
    void dbClose();
    /** 
     * @brief empty database
     *
     * @return return value from mdb_drop()
     */
    int dbEmpty();
    /** 
     * @brief drop database and close it
     *
     * @return return value from mdb_drop()
     */
    int dbDrop();
    
    int16_t id;
    std::string name;
    int16_t versionid;

    PartitionGroup *partitionGroup;
    Catalog *parentCatalog;
    Schema *parentSchema;
    Table *parentTable;
    lmdbinfo_s lmdbinfo;
};

void ser(const Metadata &d, Serdes &output);
size_t sersize(const Metadata &d);
void des(Serdes &input, Metadata &d);

#include "Catalog.h"

#endif // INFINISQLMETADATA_H
