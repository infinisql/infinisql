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
 * @file   Table.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 13:25:20 2014
 * 
 * @brief  table, contains fields and indices
 */

#include "Table.h"
#include "Catalog.h"
#include "Schema.h"
#line 32 "Table.cc"

Table::Table()
{
    
}

Table::Table(const Table &orig) : Metadata(orig)
{
}

Table &Table::operator= (const Table &orig)
{
    (Metadata)*this=Metadata(orig);
    return *this;
}

Table::~Table()
{
}

void Table::getdbname(char *dbname)
{
    getdbname2('t', parentCatalog->id, parentSchema->id, dbname);
}

int Table::dbOpen()
{
    return Metadata::dbOpen(0);
}

void ser(const Table &d, Serdes &output)
{
    ser((const Metadata &)d, output);
}

size_t sersize(const Table &d)
{
    return sersize((const Metadata &)d);
}

void des(Serdes &input, Table &d)
{
    des(input, (Metadata &)d);
}
