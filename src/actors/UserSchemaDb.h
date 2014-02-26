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
 * users: catalogid,userid
 * schemata: catalogid,schemaid
 * tables: schemaid,tableid,versionid
 * fields: schemaid,tableid,fieldid,versionid
 * indices: schemaid,indexid,versionid
 *
 * all id's are int16_t, binary, host-endian
 * value of each entry for metadata objects is created by ser() for that
 * object
 *
 * bring up in this order, based on dependencies:
 * 1) partitiongroup
 * 2) catalog
 * 3) user
 * 4) schema
 * 5) table
 * 6) field
 * 7) index
 *
 * name & idspace hierarchy:
 * OBJECT           NAMESPACE           IDSPACE
 * partitiongroup   . (UserSchemaDb)    . (UserSchemaDb)
 * catalog          . (UserSchemaDb)    . (UserSchemaDb)
 * user             catalog             catalog
 * schema           catalog             . (UserSchemaDb)
 * table            schema              schema
 * field            table               table
 * index            schema              table
 *
 * schemaid2Catalog id maps each schema to its parent catalog
 */

template < typename T >
class MetaDbOp
{
public:
    MetaDbOp();

    template < typename U, typename V >
    void populatemaps (U &namemap, V &idmap, const std::string &newname,
        int16_t newid);
};

template < typename T >
MetaDbOp< T >::MetaDbOp() {}

template < typename T >
template < typename U, typename V >
void MetaDbOp< T >::populatemaps(U &namemap, V &idmap,
                                 const std::string &newname, int16_t newid)
{
    namemap[newname]=newid;
    idmap[newid]=new T;    
}

class UserSchemaDb
{
public:
    enum metatypes_e
    {
        META_NONE = 0,
        META_PARTITIONGROUP = 1,
        META_CATALOG = 2,
        META_USER = 3,
        META_SCHEMA = 4,
        META_TABLE = 5,
        META_FIELD = 6,
        META_INDEX = 7
    };
    enum reason_e
    {
        REASON_OK = 0,
        REASON_NAME = 1,
        REASON_NOCATALOGID = 2,
        REASON_NOSCHEMAID = 3,
        REASON_NOTABLEID = 4,
        REASON_NOPARTITIONGROUPID = 5,
        REASON_BADTYPE = 6,
        REASON_DUPLICATENAME = 7,
        REASON_NOPARTITIONGROUPIDS = 8,
        REASON_NOCATALOGIDS = 9,
        REASON_NOUSERIDS = 10,
        REASON_NOSCHEMAIDS = 11,
        REASON_NOTABLEIDS = 12,
        REASON_NOFIELDIDS = 13,
        REASON_NOINDEXIDS = 14,
        REASON_CANTPERSIST = 15
    };
    struct createargs_s
    {
        metatypes_e metatype;
        std::string name;
        int16_t catalogid;
        int16_t schemaid;
        int16_t tableid;
        int16_t partitiongroupid;
        Field::type_e fieldtype;
        ssize_t size;
        ssize_t precision;
        ssize_t scale;
        FieldValue defaultValue;
        bool nullconstraint;
        std::vector<int16_t> fieldids;
        bool isunique;
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

    template < typename T >
    bool persist(const std::vector<int16_t> &key, const T &meta)
    {
        Serdes serkey(sersize(key));
        ser(key, serkey);
        Serdes serval(sersize(meta));
        ser(meta, serval);
        int retval=mdb_put(txn, dbi, &serkey.val, &serval.val, 0);
        if (retval)
        {
            LOG("mdb_put problem " << retval);
            txn_abort();
            dbi_close();
            return false;
        }

        return true;
    }

    bool stow();

    reason_e create(const createargs_s &createargs, int16_t &newid);

    MDB_env *env;
    MDB_txn *txn;
    MDB_dbi dbi;
    MDB_cursor *cursor;

    std::map<metatypes_e, const char *> dbTypeNames;

    int16_t nextpartitiongroupid;
    int16_t nextcatalogid;
    int16_t nextschemaid;
        
    std::map< std::vector<int16_t>, PartitionGroup > partitiongroups;
    std::map< std::vector<int16_t>, Catalog > catalogs;
    std::map< std::vector<int16_t>, Schema > schemata;
    std::map< std::vector<int16_t>, Table > tables;
    std::map< std::vector<int16_t>, Field > fields;
    std::map< std::vector<int16_t>, Index > indices;
    std::map< std::vector<int16_t>, User > users;

    std::unordered_map<int16_t, int16_t> schemaid2Catalogid;
    
    std::unordered_map<std::string, int16_t> partitiongroupName2Id;
    std::unordered_map<std::string, int16_t> catalogName2Id;

    std::unordered_map<int16_t, PartitionGroup *>
        partitiongroupid2Partitiongroup;
    std::unordered_map<int16_t, Catalog *> catalogid2Catalog;
    /* schema id space is in UserSchemaDb, but namespace in
     * Catalog. schemaidToCatalogid maps the id's.
     * This is to allow authentication state to be passed as schemaid
     * and userid, and not require catalogid as well.
     */     
    std::unordered_map<int16_t, Schema *> schemaid2Schema;        
};

#endif // INFINISQL_USERSCHEMADB_H
