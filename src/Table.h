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
    Table(Schema *parentSchemaarg, std::string namearg);
    Table(const Table &orig);
    Table &operator= (const Table &orig);
    /** 
     * @brief copy sufficient for reproduction elsewhere
     *
     * requires post-processing for destination actors' pointers to related
     * objects
     *
     * @param orig 
     */
    void cp(const Table &orig);
    ~Table();

    void ser(Serdes &output);
    size_t sersize();
    void des(Serdes &input);

    /** 
     * @brief fieldid generator
     *
     *
     * @return next fieldid
     */
    int16_t getnextfieldid();
    /** 
     * @brief get metadata parent information from parentSchema
     *
     */
    void getparents();
    /** 
     * @brief open table database
     *
     *
     * @return return value from mdb_dbi_open()
     */
    int dbOpen();

    int16_t nextfieldid;

    std::unordered_map<std::string, int16_t> fieldName2Id; /**< fieldName2Id[name]=fieldid */
    std::unordered_map<int16_t, Field *> fieldid2Field; /**< fieldid2Field[fieldid]=Field* */
    std::unordered_map<std::string, int16_t> indexName2Id; /**< indexName2Id[name]=indexid */
    std::unordered_map<int16_t, Index *> indexid2Index; /**< indexid2Index[indexid]=Index* */
};

#endif // INFINISQLTABLE_H
