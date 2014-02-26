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
 * @file   Schema.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 13:02:02 2014
 * 
 * @brief  schema is a collection of tables and indices
 */

#ifndef INFINISQLSCHEMA_H
#define INFINISQLSCHEMA_H

#include "Metadata.h"

class Table;
class Index;

class Schema : public Metadata
{
public:
    Schema();
    Schema(const Schema &orig);
    Schema &operator= (const Schema &orig);
    ~Schema();

    int16_t parentcatalogid;

    int16_t nexttableid;

    std::unordered_map<std::string, int16_t> tableName2Id;
    std::unordered_map<std::string, int16_t> indexName2Id;
    // tableid2Table[tableid][versionid]=Table*
    std::unordered_map< int16_t, std::unordered_map<int16_t, Table *> >
        tableid2Table;
};

void ser(const Schema &d, Serdes &output);
size_t sersize(const Schema &d);
void des(Serdes &input, Schema &d);

#endif // INFINISQLSCHEMA_H
