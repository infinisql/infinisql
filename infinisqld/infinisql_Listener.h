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

#ifndef INFINISQLLISTENER_H
#define INFINISQLLISTENER_H

#include "infinisql_gch.h"

class Listener
{
public:
    Listener(Topology::partitionAddress *);
    virtual ~Listener();

    int startsocket(string &, string &);

    //private:
    class Mboxes mboxes;
    Topology::partitionAddress myIdentity;
    class Topology myTopology;
};

void *listener(void *);

#endif  /* INFINISQLLISTENER_H */
