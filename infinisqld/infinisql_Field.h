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

#ifndef INFINISQLFIELD_H
#define INFINISQLFIELD_H

#include "infinisql_gch.h"
#include "infinisql_Index.h"

class Field
{
public:
    Field(fieldtype_e, int64_t, indextype_e, string);
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
    string name;
};

#endif  /* INFINISQLFIELD_H */

