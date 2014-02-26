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
    MetaDbOp<Catalog> mdbop;
    std::string ha("ho");
//    mdbop.populatemaps(catalogName2Id, catalogid2Catalog, ha, 50);
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
        
    case META_USER:
    {
        User meta;
        des(serval, meta);
        users[getkey(key)]=meta;
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
        
    default:
        LOG("can't handle type " << metatype);
    }
}

bool UserSchemaDb::load()
{
    std::map<metatypes_e, const char *>::iterator it;
    for (int n=(int)META_PARTITIONGROUP; n <= (int)META_INDEX; ++n)
    {
        if (dbi_open(dbTypeNames[(metatypes_e)n])==false)
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
            populatemap((metatypes_e)n, &key, &val);
            retval=mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        }
        cursor_close();
        txn_abort();
        dbi_close();
    }

    return true;
}

bool UserSchemaDb::stow()
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
        int retval=mdb_drop(txn, dbi, 0);
        if (retval)
        {
            LOG("mdb_drop problem " << retval);
            txn_abort();
            dbi_close();
            return false;
        }

        switch (it->first)
        {
        case META_PARTITIONGROUP:
        {
            std::map< std::vector<int16_t>, PartitionGroup >::iterator it2;
            for (it2 = partitiongroups.begin(); it2 != partitiongroups.end();
                ++it2)
            {
                std::vector<int16_t> vkey {it2->second.id};
                if (persist(vkey, it2->second)==false)
                {
                    return false;
                }
            }
        }
        break;

        case META_CATALOG:
        {
            std::map< std::vector<int16_t>, Catalog >::iterator it2;
            for (it2 = catalogs.begin(); it2 != catalogs.end();
                ++it2)
            {
                std::vector<int16_t> vkey {it2->second.id};
                if (persist(vkey, it2->second)==false)
                {
                    return false;
                }
            }
        }
        break;
        
        case META_SCHEMA:
        {
            std::map< std::vector<int16_t>, Schema >::iterator it2;
            for (it2 = schemata.begin(); it2 != schemata.end();
                ++it2)
            {
                std::vector<int16_t> vkey {it2->second.parentCatalog->id,
                        it2->second.id};
                if (persist(vkey, it2->second)==false)
                {
                    return false;
                }
            }
        }
        break;
        
        case META_TABLE:
        {
            std::map< std::vector<int16_t>, Table >::iterator it2;
            for (it2 = tables.begin(); it2 != tables.end();
                ++it2)
            {
                std::vector<int16_t> vkey {it2->second.parentCatalog->id,
                        it2->second.id, it2->second.versionid};
                if (persist(vkey, it2->second)==false)
                {
                    return false;
                }
            }
        }
        break;
        
        case META_FIELD:
        {
            std::map< std::vector<int16_t>, Field >::iterator it2;
            for (it2 = fields.begin(); it2 != fields.end();
                ++it2)
            {
                std::vector<int16_t> vkey {it2->second.parentCatalog->id,
                        it2->second.parentTable->id, it2->second.id,
                        it2->second.versionid};
                if (persist(vkey, it2->second)==false)
                {
                    return false;
                }
            }
        }
        break;
        
        case META_INDEX:
        {
            std::map< std::vector<int16_t>, Index >::iterator it2;
            for (it2 = indices.begin(); it2 != indices.end();
                ++it2)
            {
                std::vector<int16_t> vkey {it2->second.parentCatalog->id,
                        it2->second.id, it2->second.versionid};
                if (persist(vkey, it2->second)==false)
                {
                    return false;
                }
            }
        }
        break;
        
        case META_USER:
        {
            std::map< std::vector<int16_t>, User >::iterator it2;
            for (it2 = users.begin(); it2 != users.end();
                ++it2)
            {
                std::vector<int16_t> vkey {it2->second.parentCatalog->id,
                        it2->second.id};
                if (persist(vkey, it2->second)==false)
                {
                    return false;
                }
            }
        }
        break;

        default:
            LOG("can't handle " << it->first);
            txn_abort();
            dbi_close();
            return false;
        }
        
    }

    return true;
}

/*
    switch(args.metatype)
    {
    case META_PARTITIONGROUP:
        break;

    case META_CATALOG:
        break;

    case META_USER:
        break;

    case META_SCHEMA:
        break;

    case META_TABLE:
        break;
        
    case META_FIELD:
        break;

    case META_INDEX:
        break;

    default:
        LOG("no metatype " << args.metatype);
        return REASON_BADTYPE;
    }
*/

UserSchemaDb::reason_e UserSchemaDb::create(const createargs_s &args,
                                            int16_t &newid)
{
    newid=0;
    
    // set up ptrs of related objects
    Metadata *newObjectCurrentVersion=nullptr;
    Metadata *newObjectPendingVersion=nullptr;
    Catalog *parentCatalogptr=nullptr;
    Schema *parentSchemaptr=nullptr;
    Table *parentTableptrCurrentVersion=nullptr;
    Table *parentTableptrPendingVersion=nullptr;
    PartitionGroup *partitionGroupptr=nullptr;
    int16_t currentversionid=0;
    int16_t pendingversionid=0;
    
    if (args.partitiongroupid)
    {
        if (!partitiongroupid2Partitiongroup.count(args.partitiongroupid))
        {
            LOG("can't find partitiongroupid " << args.partitiongroupid);
            return REASON_NOPARTITIONGROUPID;
        }
        partitionGroupptr=
            partitiongroupid2Partitiongroup[args.partitiongroupid];
        currentversionid=partitionGroupptr->currentversionid;
        pendingversionid=partitionGroupptr->pendingversionid;
    }
    if (args.catalogid != 0)
    {
        if (!catalogid2Catalog.count(args.catalogid))
        {
            LOG("can't find catalogid " << args.catalogid);
            return REASON_NOCATALOGID;
        }
        parentCatalogptr=catalogid2Catalog[args.catalogid];
    }
    if (args.schemaid != 0)
    {
        if (!schemaid2Schema.count(args.schemaid))
        {
            LOG("can't find schemaid " << args.schemaid);
            return REASON_NOSCHEMAID;
        }
        parentSchemaptr=schemaid2Schema[args.schemaid];
    }
    if (args.tableid != 0)
    {
        if (!parentSchemaptr->tableid2Table.count(args.tableid))
        {
            LOG("can't find tableid " << args.tableid);
            return REASON_NOTABLEID;
        }
        parentTableptrCurrentVersion=parentSchemaptr->tableid2Table[args.tableid][currentversionid];
        if (pendingversionid)
        {
            parentTableptrPendingVersion=parentSchemaptr->tableid2Table[args.tableid][pendingversionid];            
        }
    }
    if (args.partitiongroupid)
    {
        if (!partitiongroupid2Partitiongroup.count(args.partitiongroupid))
        {
            LOG("can't find partitiongroupid " << args.partitiongroupid);
            return REASON_NOPARTITIONGROUPID;
        }
        partitionGroupptr=
            partitiongroupid2Partitiongroup[args.partitiongroupid];
    }

    // check namespace, get newid, create object and put in maps
    switch(args.metatype)
    {
    case META_PARTITIONGROUP:
        if (partitiongroupName2Id.count(args.name))
        {
            return REASON_DUPLICATENAME;
        }
        newid=Metadata::getnextid(partitiongroupid2Partitiongroup,
                                       nextpartitiongroupid);
        if (newid==0)
        {
            return REASON_NOPARTITIONGROUPIDS;
        }
        partitiongroupName2Id[args.name]=newid;
        partitiongroups[std::vector<int16_t>({newid})]=PartitionGroup();
        partitiongroupid2Partitiongroup[newid]=
            &partitiongroups[std::vector<int16_t>({newid})];
        newObjectCurrentVersion=&partitiongroups[std::vector<int16_t>({newid})];
        break;

    case META_CATALOG:
        if (catalogName2Id.count(args.name))
        {
            return REASON_DUPLICATENAME;
        }
        newid=Metadata::getnextid(catalogid2Catalog, nextcatalogid);
        if (newid==0)
        {
            return REASON_NOCATALOGIDS;
        }
        catalogName2Id[args.name]=newid;
        catalogs[std::vector<int16_t>({newid})]=Catalog();
        catalogid2Catalog[newid]=
            &catalogs[std::vector<int16_t>({newid})];
        newObjectCurrentVersion=&catalogs[std::vector<int16_t>({newid})];
        break;

    case META_USER:
        if (parentCatalogptr->userName2Id.count(args.name))
        {
            return REASON_DUPLICATENAME;
        }
        newid=Metadata::getnextid(parentCatalogptr->userid2User,
            parentCatalogptr->nextuserid);
        if (newid==0)
        {
            return REASON_NOUSERIDS;
        }
        parentCatalogptr->userName2Id[args.name]=newid;
        users[std::vector<int16_t>({args.catalogid, newid})]=User();
        parentCatalogptr->userid2User[newid]=
            &users[std::vector<int16_t>({args.catalogid, newid})];
        newObjectCurrentVersion=
            &users[std::vector<int16_t>({args.catalogid, newid})];
        break;

    case META_SCHEMA:
        if (parentCatalogptr->schemaName2Id.count(args.name))
        {
            return REASON_DUPLICATENAME;
        }
        newid=Metadata::getnextid(schemaid2Schema, nextschemaid);
        if (newid==0)
        {
            return REASON_NOSCHEMAIDS;
        }
        parentCatalogptr->schemaName2Id[args.name]=newid;
        schemata[std::vector<int16_t>({args.catalogid, newid})]=Schema();
        schemaid2Schema[newid]=
            &schemata[std::vector<int16_t>({args.catalogid, newid})];
        newObjectCurrentVersion=
            &schemata[std::vector<int16_t>({args.catalogid, newid})];
        schemaid2Catalogid[newid]=args.catalogid;
        break;

    case META_TABLE:
        if (parentSchemaptr->tableName2Id.count(args.name))
        {
            return REASON_DUPLICATENAME;
        }
        newid=Metadata::getnextid(parentSchemaptr->tableid2Table,
                                  parentSchemaptr->nexttableid);
        if (newid==0)
        {
            return REASON_NOSCHEMAIDS;
        }
        parentSchemaptr->tableName2Id[args.name]=newid;
        tables[std::vector<int16_t>({args.schemaid, newid,
                        currentversionid})]=Table();
        parentSchemaptr->tableid2Table[newid][currentversionid]=
            &tables[std::vector<int16_t>({args.schemaid, newid,
                        currentversionid})];
        newObjectCurrentVersion=&tables[std::vector<int16_t>({args.schemaid,
                        newid, currentversionid})];
        if (pendingversionid)
        {
            tables[std::vector<int16_t>({args.schemaid, newid,
                            pendingversionid})]=Table();
            parentSchemaptr->tableid2Table[newid][pendingversionid]=
                &tables[std::vector<int16_t>({args.schemaid, newid,
                            pendingversionid})];
            newObjectPendingVersion=
                &tables[std::vector<int16_t>({args.schemaid, newid,
                            pendingversionid})];
        }
        break;
        
    case META_FIELD:
        if (parentTableptrCurrentVersion->fieldName2Id.count(args.name))
        {
            return REASON_DUPLICATENAME;
        }
        if (parentTableptrCurrentVersion->fields.size() >
            std::numeric_limits<int16_t>::max())
        {
            newid=0;
            return REASON_NOFIELDIDS;
        }
        newid=parentTableptrCurrentVersion->fields.size();
        parentTableptrCurrentVersion->fieldName2Id[args.name]=newid;
        fields[std::vector<int16_t>({args.schemaid, args.tableid,
                        newid, currentversionid})]=Field();
        parentTableptrCurrentVersion->fields[newid]=
            &fields[std::vector<int16_t>({args.schemaid, args.tableid,
                        newid, currentversionid})];
        newObjectCurrentVersion=&fields[std::vector<int16_t>({args.schemaid,
                        args.tableid, newid, currentversionid})];
        if (pendingversionid)
        {
            fields[std::vector<int16_t>({args.schemaid, args.tableid,
                            newid, pendingversionid})]=Field();
            parentTableptrPendingVersion->fields[newid]=
                &fields[std::vector<int16_t>({args.schemaid, args.tableid,
                            newid, pendingversionid})];
            newObjectPendingVersion=&fields[std::vector<int16_t>({args.schemaid,
                        args.tableid, newid, pendingversionid})];
        }
        break;

    case META_INDEX:
        if (parentSchemaptr->indexName2Id.count(args.name))
        {
            return REASON_DUPLICATENAME;
        }
        newid=Metadata::getnextid(parentTableptrCurrentVersion->indexid2Index,
                                  parentTableptrCurrentVersion->nextindexid);
        if (newid==0)
        {
            return REASON_NOSCHEMAIDS;
        }
        parentSchemaptr->indexName2Id[args.name]=newid;
        indices[std::vector<int16_t>({args.schemaid, newid,
                        currentversionid})]=Index();
        parentTableptrCurrentVersion->indexid2Index[newid][currentversionid]=
            &indices[std::vector<int16_t>({args.schemaid, newid,
                        currentversionid})];
        newObjectCurrentVersion=&indices[std::vector<int16_t>({args.schemaid,
                        newid, currentversionid})];
        if (pendingversionid)
        {
            indices[std::vector<int16_t>({args.schemaid, newid,
                            pendingversionid})]=Index();
            parentTableptrPendingVersion->indexid2Index[newid][pendingversionid]=&indices[std::vector<int16_t>({args.schemaid, newid, pendingversionid})];
            newObjectPendingVersion=
                &indices[std::vector<int16_t>({args.schemaid, newid,
                            pendingversionid})];
        }
        break;

    default:
        LOG("no metatype " << args.metatype);
        return REASON_BADTYPE;
    }

    // set up objects
    newObjectCurrentVersion->name=args.name;
    newObjectCurrentVersion->id=newid;
    if (newObjectPendingVersion != nullptr)
    {
        *newObjectCurrentVersion=*newObjectPendingVersion;
        newObjectCurrentVersion->versionid=currentversionid;
    }

    switch(args.metatype)
    {
    case META_PARTITIONGROUP:
        break;

    case META_CATALOG:
        break;

    case META_USER:
        newObjectCurrentVersion->parentCatalog=parentCatalogptr;
        break;

    case META_SCHEMA:
        newObjectCurrentVersion->parentCatalog=parentCatalogptr;
        break;

    case META_TABLE:
        newObjectCurrentVersion->parentCatalog=parentCatalogptr;
        newObjectCurrentVersion->partitionGroup=partitionGroupptr;
        newObjectCurrentVersion->versionid=currentversionid;
        if (pendingversionid)
        {
            *newObjectPendingVersion=*newObjectCurrentVersion;
        }
        break;
        
    case META_FIELD:
    {
        Field &newField=*(Field *)newObjectCurrentVersion;
        newField.parentSchema=parentSchemaptr;
        newField.parentTable=parentTableptrCurrentVersion;
        newField.type=args.fieldtype;
        newField.size=args.size;
        newField.precision=args.precision;
        newField.scale=args.scale;
        newField.defaultValue=args.defaultValue;
        newField.nullconstraint=args.nullconstraint;
    }
    if (pendingversionid)
    {        
        Field &newField=*(Field *)newObjectPendingVersion;
        newField=*(Field *)newObjectCurrentVersion;
        newField.parentTable=parentTableptrPendingVersion;
    }
    break;

    case META_INDEX:
    {
        Index &newIndex=*(Index *)newObjectCurrentVersion;
        newIndex.parentSchema=parentSchemaptr;
        newIndex.parentTable=parentTableptrCurrentVersion;
        newIndex.fieldids=args.fieldids;
        newIndex.isunique=args.isunique;
    }
    if (pendingversionid)
    {
        Index &newIndex=*(Index *)newObjectPendingVersion;
        newIndex=*(Index *)newObjectCurrentVersion;
        newIndex.parentTable=parentTableptrPendingVersion;
    }
    break;

    default:
        LOG("no metatype " << args.metatype);
        return REASON_BADTYPE;
    }

    // persist new object(s)
    switch(args.metatype)
    {
    case META_PARTITIONGROUP:
        if (persist(std::vector<int16_t>({newid}),
                    *(PartitionGroup *)newObjectCurrentVersion)==false)
        {
            return REASON_CANTPERSIST;
        }
        break;

    case META_CATALOG:
        if (persist(std::vector<int16_t>({newid}),
                    *(Catalog *)newObjectCurrentVersion)==false)
        {
            return REASON_CANTPERSIST;
        }
        break;

    case META_USER:
        if (persist(std::vector<int16_t>({args.catalogid, newid}),
                    *(User *)newObjectCurrentVersion)==false)
        {
            return REASON_CANTPERSIST;
        }
        break;

    case META_SCHEMA:
        if (persist(std::vector<int16_t>({args.catalogid, newid}),
                    *(Schema *)newObjectCurrentVersion)==false)
        {
            return REASON_CANTPERSIST;
        }
        break;

    case META_TABLE:
        if (persist(std::vector<int16_t>({args.schemaid, newid,
                            currentversionid}),
                *(Table *)newObjectCurrentVersion)==false)
        {
            return REASON_CANTPERSIST;
        }
        if (pendingversionid)
        {
            if (persist(std::vector<int16_t>({args.schemaid, newid,
                                pendingversionid}),
                    *(Table *)newObjectPendingVersion)==false)
            {
                return REASON_CANTPERSIST;
            }            
        }
        break;
        
    case META_FIELD:
        if (persist(std::vector<int16_t>({args.schemaid, args.tableid, newid,
                            currentversionid}),
                *(Field *)newObjectCurrentVersion)==false)
        {
            return REASON_CANTPERSIST;
        }
        if (pendingversionid)
        {
            if (persist(std::vector<int16_t>({args.schemaid, args.tableid,
                                newid, pendingversionid}),
                    *(Field *)newObjectPendingVersion)==false)
            {
                return REASON_CANTPERSIST;
            }            
        }
        break;

    case META_INDEX:
        if (persist(std::vector<int16_t>({args.schemaid, newid,
                            currentversionid}),
                *(Index *)newObjectCurrentVersion)==false)
        {
            return REASON_CANTPERSIST;
        }
        if (pendingversionid)
        {
            if (persist(std::vector<int16_t>({args.schemaid, newid,
                                pendingversionid}),
                    *(Index *)newObjectPendingVersion)==false)
            {
                return REASON_CANTPERSIST;
            }            
        }
        break;

    default:
        LOG("no metatype " << args.metatype);
        return REASON_BADTYPE;
    }

    return REASON_OK;
}
