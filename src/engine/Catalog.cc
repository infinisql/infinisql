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
 * @file   Catalog.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 08:14:02 2014
 * 
 * @brief  catalog is a collection of schemata and users
 * 
 * 
 */

#include "Catalog.h"

Catalog::Catalog() : Metadata (), nextuserid (0), nextschemaid (0),
                     nexttableid (0), nextindexid (0)
{
}

Catalog::Catalog(int16_t idarg, const std::string& namearg)
    : Metadata(idarg, namearg), nextuserid(0), nextschemaid(0),
      nexttableid(0), nextindexid(0)
{
}

Catalog::Catalog(const Catalog &orig) : Metadata (orig)
{
    cp(orig);
}

Catalog &Catalog::operator= (const Catalog &orig)
{
    (Metadata)*this=Metadata(orig);
    cp(orig);
    return *this;
}

void Catalog::cp(const Catalog &orig)
{
    nextuserid=orig.nextuserid;
    nextschemaid=orig.nextschemaid;
    nexttableid=orig.nexttableid;
    nextindexid=orig.nextindexid;
}

Catalog::~Catalog()
{
}

int16_t Catalog::getnextuserid()
{
    return ++nextuserid;
}

int16_t Catalog::getnextschemaid()
{
    return ++nextschemaid;
}

int16_t Catalog::getnexttableid()
{
    return ++nexttableid;
}

int16_t Catalog::getnextindexid()
{
    return ++nextindexid;
}

int Catalog::openEnvironment(std::string path)
{
    int retval=mdb_env_create(&lmdbinfo.env);
    if (retval)
    {
        return retval;
    }
    /* 96 bytes overhed per LMDB database per thread, according to
     * https://github.com/skydb/sky/pull/103
     * each table and index are a database
     */
    retval=mdb_env_set_maxdbs(lmdbinfo.env, 16384);
    if (retval)
    {
        mdb_env_close(lmdbinfo.env);
        lmdbinfo.env=nullptr;
        return retval;
    }
    retval=mdb_env_open(lmdbinfo.env, path.c_str(), MDB_WRITEMAP,
                        S_IRUSR|S_IWUSR);
    if (retval)
    {
        mdb_env_close(lmdbinfo.env);
        lmdbinfo.env=nullptr;
    }
    
    return retval;
}

void Catalog::closeEnvironment()
{
    mdb_env_close(lmdbinfo.env);
    lmdbinfo.env=nullptr;
}

int Catalog::deleteEnvironment(std::string path)
{
    string p=path;
    if (p.size() && p[p.size()-1] != '/')
    {
        p.append(1, '/');
    }
    string data=p + "data.mdb";
    int retval=remove(data.c_str());
    if (retval)
    {
        return retval;
    }
    string lock=p + "lock.mdb";
    return remove(lock.c_str());
}

void ser(const Catalog &d, Serdes &output)
{
    ser((const Metadata &)d, output);
    ser(d.nextuserid, output);
    ser(d.nextschemaid, output);
    ser(d.nexttableid, output);
    ser(d.nextindexid, output);
}

size_t sersize(const Catalog &d)
{
    return sersize((const Metadata &)d) + sersize(d.nextuserid) +
        sersize(d.nextschemaid) + sersize(d.nexttableid) +
        sersize(d.nextindexid);
}

void des(Serdes &input, Catalog &d)
{
    des(input, (Metadata &)d);
    des(input, d.nextuserid);
    des(input, d.nextschemaid);
    des(input, d.nexttableid);
    des(input, d.nextindexid);
}
