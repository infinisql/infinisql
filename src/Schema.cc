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

Schema::Schema() : Metadata()
{
    
}

Schema::Schema(Catalog *parentCatalogarg, std::string namearg)
{
    if (parentCatalogarg->schemaName2Id.count(namearg))
    {
        id=-1;
        return;
    }
    parentCatalog=parentCatalogarg;
    getparents();
    id=parentCatalog->getnextschemaid();
    name=namearg;
    parentCatalog->schemaName2Id[name]=id;
    parentCatalog->schemaid2Schema[id]=this;    
}

Schema::Schema(const Schema &orig) : Metadata (orig)
{
}

Schema &Schema::operator= (const Schema &orig)
{
    (Metadata)*this=Metadata(orig);
    return *this;
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
    parentSchema=nullptr;
    parentTable=nullptr;
    parentcatalogid=parentCatalog->id;
    parentschemaid=-1;
    parenttableid=-1;
}
