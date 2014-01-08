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
 * @file   Serdes.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Jan  7 17:16:03 2014
 * 
 * @brief  serialization and deserialization for POD and generic data types
 */

#include "Serdes.h"
#line 30 "Serdes.cc"

Serdes::Serdes()
{
    
}

Serdes::~Serdes()
{
    
}

size_t Serdes::ser(int16_t d, char *output)
{
    memcpy(output, &d, sizeof(d));
    return sizeof(d);
}

size_t Serdes::sersize(int16_t d)
{
    return sizeof(d);
}

size_t Serdes::des(char *input, int16_t *d)
{
    memcpy(d, input, sizeof(*d));
    return sizeof(*d);
}

size_t Serdes::ser(int32_t d, char *output)
{
    memcpy(output, &d, sizeof(d));
    return sizeof(d);
}

size_t Serdes::sersize(int32_t d)
{
    return sizeof(d);
}

size_t Serdes::des(char *input, int32_t *d)
{
    memcpy(d, input, sizeof(*d));
    return sizeof(*d);
}

size_t Serdes::ser(int64_t d, char *output)
{
    memcpy(output, &d, sizeof(d));
    return sizeof(d);
}

size_t Serdes::sersize(int64_t d)
{
    return sizeof(d);
}

size_t Serdes::des(char *input, int64_t *d)
{
    memcpy(d, input, sizeof(*d));
    return sizeof(*d);
}

size_t Serdes::ser(char d, char *output)
{
    memcpy(output, &d, sizeof(d));
    return sizeof(d);
}

size_t Serdes::sersize(char d)
{
    return sizeof(d);
}

size_t Serdes::des(char *input, char *d)
{
    memcpy(d, input, sizeof(*d));
    return sizeof(*d);
}

size_t Serdes::ser(bool d, char *output)
{
    memcpy(output, &d, sizeof(d));
    return sizeof(d);
}

size_t Serdes::sersize(bool d)
{
    return sizeof(d);
}

size_t Serdes::des(char *input, bool *d)
{
    memcpy(d, input, sizeof(*d));
    return sizeof(*d);
}

size_t Serdes::ser(std::string &d, char *output)
{
    size_t s=d.size();
    ser((int64_t)s, output);
    d.copy(output+sizeof(int64_t), s, 0);
    return sizeof(int64_t)+s;
}

size_t Serdes::sersize(std::string &d)
{
    return sizeof(int64_t)+d.size();
}

size_t Serdes::des(char *input, std::string &d)
{
    int64_t s;
    des(input, &s);
    d.assign(input+sizeof(s), s);
    return sizeof(s)+s;
}
