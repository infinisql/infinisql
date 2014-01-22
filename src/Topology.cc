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
 * @file   Topology.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Wed Jan 22 09:06:10 2014
 * 
 * @brief Topology class has all of the actors, their types, and dynamic
 * configuration values. Each actor maintains a Topology object which gives
 * it a common view for the whole node and, as necessary, the whole cluster.
 */

#include "Topology.h"

Topology::Topology()
{
    
}
