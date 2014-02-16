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
    Schema(std::shared_ptr<Catalog> parentCatalogarg, const std::string& namearg);
    Schema(const Schema &orig);
    Schema &operator= (const Schema &orig);
    ~Schema();

    void ser(Serdes &output);
    size_t sersize();
    void des(Serdes &input);    
    /** 
     * @brief get metadata parent information from parentTable
     *
     */
    void getparents();
    
    std::unordered_map<std::string, int16_t> tableName2Id; /**< tableName2Id[name]=tableid */
    std::unordered_map<int16_t, Table *> tableid2Table; /**< tableid2Table[tableid]=Table* */
    std::unordered_map<std::string, int16_t> indexName2Id; /**< indexName2Id[name]=indexid */
    std::unordered_map<int16_t, Index *> indexid2Index; /**< indexid2Index[indexid]=Index* */
};

#endif // INFINISQLSCHEMA_H




