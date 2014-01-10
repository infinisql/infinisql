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
 * @file   Field.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:09:55 2013
 * 
 * @brief  Data Field class (INT, BOOL, VARCHAR, etc).
 */

#include "Field.h"
#line 30 "Field.cc"

Field::Field(fieldtype_e typearg, int64_t lengtharg,
             indextype_e indextypearg, string namearg) :
    type(typearg), length(lengtharg), indextype(indextypearg), name(namearg)
{
    index.makeindex(indextype, type);
}

Field::~Field()
{
}
