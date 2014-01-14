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
 * @file   Catalog.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 08:14:02 2014
 * 
 * @brief  catalog is a collection of schemata and users
 * 
 * 
 */

#include "Catalog.h"
#line 32 "Catalog.cc"

Catalog::Catalog() : Metadata (-1, "", NULL, NULL, NULL, -1, -1, -1),
                     nextuserid (0), nextschemaid (0), nexttableid (0),
                     nextindexid (0)
{
    
}

Catalog::~Catalog()
{
}

void Catalog::ser(Serdes &output)
{
    Metadata::ser(output);
    output.ser(nextuserid);
    output.ser(nextschemaid);
    output.ser(nexttableid);
    output.ser(nextindexid);
}

size_t Catalog::sersize()
{
    size_t retval=Metadata::sersize();
    retval+=Serdes::sersize(nextuserid);
    retval+=Serdes::sersize(nextschemaid);
    retval+=Serdes::sersize(nexttableid);
    retval+=Serdes::sersize(nextindexid);

    return retval;
}

void Catalog::des(Serdes &input)
{
    Metadata::des(input);
    input.des(nextuserid);
    input.des(nextschemaid);
    input.des(nexttableid);
    input.des(nextindexid);
}

int16_t Catalog::getnextuserid()
{
    return ++nextuserid;
}

int16_t Catalog::getnextschemaid()
{
    return ++nextschemaid;
}

int16_t Catalog::getnexttableid()
{
    return ++nexttableid;
}

int16_t Catalog::getnextindexid()
{
    return ++nextindexid;
}
