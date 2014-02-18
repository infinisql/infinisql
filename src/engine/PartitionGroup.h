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
 * @file   PartitionGroup.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Feb 17 13:26:18 2014
 * 
 * @brief  just describes partition group for storage in UserSchema
 */

#ifndef INFINISQLPARTITIONGROUP_H
#define INFINISQLPARTITIONGROUP_H

#include "Metadata.h"

class PartitionGroup : public Metadata
{
public:
    PartitionGroup();

    int16_t currentversionid;
    int16_t pendingversionid;
};

void ser(const PartitionGroup &d, Serdes &output);
size_t sersize(const PartitionGroup &d);
void des(Serdes &input, PartitionGroup &d);

#endif // INFINISQLPARTITIONGROUP_H
