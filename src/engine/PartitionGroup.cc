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
 * @file   PartitionGroup.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Feb 17 13:30:40 2014
 * 
 * @brief  just describes partition group for storage in UserSchema
 */

#include "PartitionGroup.h"

PartitionGroup::PartitionGroup(): Metadata(), currentversionid(0),
                                  pendingversionid(0)
{
    
}

PartitionGroup::PartitionGroup(std::string &namearg, int16_t idarg)
    : Metadata(), currentversionid(1), pendingversionid(0)
{
    name=namearg;
    id=idarg;
}

void ser(const PartitionGroup &d, Serdes &output)
{
    ser((const Metadata &)d, output);
    ser(d.currentversionid, output);
    ser(d.pendingversionid, output);
}

size_t sersize(const PartitionGroup &d)
{
    return sersize((const Metadata &)d) + sersize(d.currentversionid) +
        sersize(d.pendingversionid);
}

void des(Serdes &input, PartitionGroup &d)
{
    des(input, (Metadata &)d);
    des(input, d.currentversionid);
    des(input, d.pendingversionid);
}
