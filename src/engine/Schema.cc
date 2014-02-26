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
 * @file   Schema.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 13:04:58 2014
 * 
 * @brief  schema is a collection of tables and indices
 */

#include "Schema.h"
#include "Catalog.h"

Schema::Schema()
{
    
}

Schema::Schema(const Schema &orig) : Metadata(orig)
{
    (Metadata)*this=Metadata(orig);
    parentcatalogid=orig.parentcatalogid;
}

Schema &Schema::operator= (const Schema &orig)
{
    (Metadata)*this=Metadata(orig);
    return *this;
}

Schema::~Schema()
{
    
}

void ser(const Schema &d, Serdes &output)
{
    ser((const Metadata &)d, output);
}

size_t sersize(const Schema &d)
{
    return sersize((const Metadata &)d);
}

void des(Serdes &input, Schema &d)
{
    des(input, (Metadata &)d);
}
