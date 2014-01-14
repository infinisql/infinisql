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
 * @file   Index.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 13:43:16 2014
 * 
 * @brief  index
 */

#ifndef INFINISQLINDEX_H
#define INFINISQLINDEX_H

#include "Metadata.h"

class Index : public Metadata
{
public:
    Index();
    ~Index();

    void ser(Serdes &output);
    size_t sersize();
    void des(Serdes &input);
    /** 
     * @brief get metadata parent information from parentTable
     *
     */
    void getparents();    
};

#endif // INFINISQLINDEX_H
