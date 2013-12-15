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

#include "infinisql_Schema.h"
#line 22 "Schema.cc"

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

int64_t Schema::getnexttableid(void)
{
    return ++nexttableid;
}
