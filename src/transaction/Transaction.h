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
 * @file   Transaction.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Sat Feb 15 16:36:07 2014
 * 
 * @brief  base class for transactions and continuing operations
 */

#ifndef INFINISQLTRANSACTION_H
#define INFINISQLTRANSACTION_H

#include "../actors/Actor.h"

class Transaction
{
public:
    Transaction();
    virtual ~Transaction();
};

#endif // INFINISQLTRANSACTION_H
