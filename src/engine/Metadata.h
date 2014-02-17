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
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <vector>

using std::string;

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
    Metadata(int16_t id, std::string name);
    Metadata(const Metadata &orig);
    virtual ~Metadata();

    void ser(Serdes &output);
    size_t sersize();
    void des(Serdes &input);

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

    Catalog *parentCatalog;
    Schema *parentSchema;
    Table *parentTable;
    int16_t parentcatalogid;
    int16_t parentschemaid;
    int16_t parenttableid;

    lmdbinfo_s lmdbinfo;
};

void ser(const Metadata &d, Serdes &output);
size_t sersize(const Metadata &d);
void des(Serdes &input, Metadata &d);

#include "Catalog.h"

#endif // INFINISQLMETADATA_H
