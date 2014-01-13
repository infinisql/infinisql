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
 * @file   Lightning.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Jan  7 23:41:14 2014
 * 
 * @brief  interface to Symas' Lightning MDB
 * 
 */

#ifndef INFINISQLLIGHTNING_H
#define INFINISQLLIGHTNING_H

#include <lmdb.h>

class Lightning
{
public:
    Lightning();
    ~Lightning();
};

#endif // INFINISQLLIGHTNING_H
