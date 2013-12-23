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
 * @file   Field.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:10:40 2013
 * 
 * @brief  Data Field class (INT, BOOL, VARCHAR, etc).
 */

#ifndef INFINISQLFIELD_H
#define INFINISQLFIELD_H

#include "gch.h"
#include "Index.h"

/** 
 * @brief Field object.
 *
 * @param typearg type of field, such as INT, FLOAT, CHAR
 * @param lengtharg length of field--set to 0 unless CHARX field
 * @param indextypearg  index type
 * @param namearg  field name 
 */
class Field
{
public:
    /** 
     * @ create field
     *
     * also create index object associated with field
     *
     * @param typearg field type
     * @param lengtharg length (for CHARX)
     * @param indextypearg index type
     * @param namearg field name
     */
    Field(fieldtype_e typearg, int64_t lengtharg, indextype_e indextypearg,
          std::string namearg);
    virtual ~Field();

    friend class Transaction;
    friend class ApiInterface;
    friend class Table;
    friend class SubTransaction;

    //private:
    fieldtype_e type;
    size_t length;
    indextype_e indextype;
    class Index index;
    std::string name;
};

#endif  /* INFINISQLFIELD_H */
