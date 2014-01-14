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
 * @file   Schema.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 13:04:58 2014
 * 
 * @brief  schema is a collection of tables and indices
 */

#include "Schema.h"
#include "Catalog.h"
#line 31 "Schema.cc"

Schema::Schema() : Metadata(-1, "", NULL, NULL, NULL, -1, -1, -1)
{
    
}

Schema::~Schema()
{
    
}

void Schema::ser(Serdes &output)
{
    Metadata::ser(output);
}

size_t Schema::sersize()
{
    return Metadata::sersize();
}

void Schema::des(Serdes &input)
{
    Metadata::des(input);
}

void Schema::getparents()
{
    parentSchema=NULL;
    parentTable=NULL;
    parentcatalogid=parentCatalog->id;
    parentschemaid=-1;
    parenttableid=-1;
}
