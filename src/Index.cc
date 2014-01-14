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
 * @file   Index.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 13:45:59 2014
 * 
 * @brief  index
 */

#include "Index.h"
#include "Catalog.h"
#include "Table.h"
#line 32 "Index.cc"

Index::Index() : Metadata (-1, "", NULL, NULL, NULL, -1, -1, -1)
{
    
}

Index::~Index()
{
}

void Index::ser(Serdes &output)
{
    Metadata::ser(output);
}

size_t Index::sersize()
{
    return Metadata::sersize();
}

void Index::des(Serdes &input)
{
    Metadata::des(input);
}

void Index::getparents()
{
    parentCatalog=parentTable->parentCatalog;
    parentSchema=parentTable->parentSchema;
    parentcatalogid=parentTable->parentcatalogid;
    parentschemaid=parentTable->parentschemaid;
    parenttableid=parentTable->id;
}
