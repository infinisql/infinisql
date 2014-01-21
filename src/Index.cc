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
#include "Schema.h"
#include "Table.h"

Index::Index() : Metadata ()
{
    
}

Index::Index(Table *parentTablearg, std::string namearg)
{
    if (parentTablearg->parentCatalog->indexName2Id.count(namearg))
    {
        id=-1;
        return;
    }
    parentTable=parentTablearg;
    getparents();
    id=parentCatalog->getnextindexid();
    name=namearg;
    parentCatalog->indexName2Id[name]=id;
    parentCatalog->indexid2Index[id]=this;
    parentSchema->indexName2Id[name]=id;
    parentSchema->indexid2Index[id]=this;    
    parentTable->indexName2Id[name]=id;
    parentTable->indexid2Index[id]=this;    
}

Index::Index(const Index &orig) : Metadata(orig)
{
//    cp(orig);
}

Index &Index::operator= (const Index &orig)
{
    (Metadata)*this=Metadata(orig);
//    cp(orig);
    return *this;
}

/*
void Index::cp(const Index &orig)
{
}
*/

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

int Index::dbOpen()
{
    return Metadata::dbOpen(MDB_DUPSORT);
}
