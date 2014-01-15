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

Table::Table() : Metadata (), nextfieldid (-1)
{
    
}

Table::Table(Schema *parentSchemaarg, std::string namearg) : nextfieldid (-1)
{
    if (parentSchemaarg->parentCatalog->tableName2Id.count(namearg))
    {
        id=-1;
        return;
    }
    parentSchema=parentSchemaarg;
    getparents();
    id=parentCatalog->getnexttableid();
    name=namearg;
    parentCatalog->tableName2Id[name]=id;
    parentCatalog->tableid2Table[id]=this;
    parentSchema->tableName2Id[name]=id;
    parentSchema->tableid2Table[id]=this;
}

Table::Table(const Table &orig) : Metadata (orig)
{
    cp(orig);
}

Table &Table::operator= (const Table &orig)
{
    (Metadata)*this=Metadata(orig);
    cp(orig);
    return *this;
}

void Table::cp(const Table &orig)
{
    nextfieldid=orig.nextfieldid;
}

Table::~Table()
{
}

void Table::ser(Serdes &output)
{
    Metadata::ser(output);
    output.ser(nextfieldid);
}

size_t Table::sersize()
{
    size_t retval=Metadata::sersize();
    retval+=Serdes::sersize(nextfieldid);

    return retval;
}

void Table::des(Serdes &input)
{
    Metadata::des(input);
    input.des(nextfieldid);
}

void Table::getparents()
{
    parentCatalog=parentSchema->parentCatalog;
    parentTable=NULL;
    parentcatalogid=parentSchema->parentcatalogid;
    parentschemaid=parentSchema->id;
    parenttableid=-1;
}

int16_t Table::getnextfieldid()
{
    return ++nextfieldid;
}

int Table::dbOpen()
{
    return Metadata::dbOpen(0);
}
