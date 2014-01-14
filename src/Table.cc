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

Table::Table() : Metadata (-1, "", NULL, NULL, NULL, -1, -1, -1),
                     nextfieldid (-1)
{
    
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
