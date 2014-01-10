/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL(tm).
 *
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
 * @file   Schema.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:45:59 2013
 * 
 * @brief  Schema class. Contains tables (fields (indices)). One schemata per
 * domain. Corresponds to tablespace or database on other RDBMS platforms.
 */

#include "Schema.h"
#line 31 "Schema.cc"

Schema::Schema(int64_t domainidarg) : domainid(domainidarg), nexttableid(0)
{
}

Schema::~Schema()
{
}

// take the table's id, check it already doesn't exist
// return status, let the caller reply to ta, or whatever
int Schema::createTable(int64_t id)
{
    if (tables.count(id))   // tableid exists
    {
        return BUILTIN_STATUS_NOTOK;
    }

    tables[id] = new Table(id);
    return BUILTIN_STATUS_OK;
}

int64_t Schema::getnexttableid()
{
    return ++nexttableid;
}
