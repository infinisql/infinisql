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
 * @file   Table.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 13:24:09 2014
 * 
 * @brief  table, contains fields and indices
 */

#ifndef INFINISQLTABLE_H
#define INFINISQLTABLE_H

#include "Metadata.h"

class Field;
class Index;

class Table : public Metadata
{
public:
    Table();
    Table(const Table &orig);
    Table &operator= (const Table &orig);
    ~Table();

    void getdbname(char *dbname);
    /** 
     * @brief open table database
     *
     *
     * @return return value from mdb_dbi_open()
     */
    int dbOpen();

    int16_t nextindexid;

    std::unordered_map<std::string, int16_t> fieldName2Id;
    std::vector<Field *> fields;
    // indexid2Index[indexid][versionid]=Index*
    std::unordered_map< int16_t, std::unordered_map<int16_t, Index *> >
        indexid2Index;
};

void ser(const Table &d, Serdes &output);
size_t sersize(const Table &d);
void des(Serdes &input, Table &d);

#endif // INFINISQLTABLE_H
