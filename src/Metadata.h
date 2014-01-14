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
 * @file   Metadata.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 07:37:51 2014
 * 
 * @brief  base class for metadata definitions
 *
 * for things such as catalogs, schemata, tables, users, indices, fields
 */

#ifndef INFINISQLMETADATA_H
#define INFINISQLMETADATA_H

#include "main.h"

class Catalog;
class Schema;
class Table;

class Metadata
{
public:
    Metadata();
    Metadata(int16_t idarg, std::string namearg, Catalog *parentCatalogarg,
             Schema *parentSchemaarg, Table *parentTablearg,
             int16_t parentcatalogid, int16_t parentschemaid,
        int16_t parenttableid);
    Metadata(const Metadata &orig);
    ~Metadata();

    void ser(Serdes &output);
    size_t sersize();
    void des(Serdes &input);
    
    int16_t id;
    std::string name;

    Catalog *parentCatalog;
    Schema *parentSchema;
    Table *parentTable;
    int16_t parentcatalogid;
    int16_t parentschemaid;
    int16_t parenttableid;
};

#include "Catalog.h"

#endif // INFINISQLMETADATA_H
