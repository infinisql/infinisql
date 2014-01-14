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
 * @file   Metadata.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 08:47:01 2014
 * 
 * @brief  base class for metadata definitions
 *
 * for things such as catalogs, schemata, tables, users, indices, fields
 */

#include "Metadata.h"
#line 32 "Metadata.cc"

Metadata::Metadata() : id (0), parentCatalog (NULL), parentSchema (NULL),
                       parentTable (NULL), parentcatalogid (-1),
                       parentschemaid (-1), parenttableid (-1)
{
    
}

Metadata::Metadata(int16_t idarg, std::string namearg, Catalog *parentCatalogarg,
                   Schema *parentSchemaarg, Table *parentTablearg,
    int16_t parentcatalogidarg, int16_t parentschemaidarg,
                   int16_t parenttableidarg)
    : id (idarg), name (namearg), parentCatalog (parentCatalogarg),
      parentSchema (parentSchemaarg), parentTable (parentTablearg),
      parentcatalogid (parentcatalogidarg), parentschemaid (parentschemaidarg),
      parenttableid (parenttableidarg)
{
    
}

Metadata::~Metadata()
{
    
}

void Metadata::ser(Serdes &output)
{
    output.ser(id);
    output.ser(name);
}

size_t Metadata::sersize()
{
    return Serdes::sersize(id) + Serdes::sersize(name);
}

void Metadata::des(Serdes &input)
{
    input.des(id);
    input.des(name);
}
