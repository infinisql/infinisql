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
 * @file   AdminListener.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Thu Jan 23 02:23:53 2014
 * 
 * @brief  0mq listener for administrative commands from cluster manager
 */

#ifndef INFINISQLADMINLISTENER_H
#define INFINISQLADMINLISTENER_H

#include "Actor.h"

class AdminListener : public Actor
{
public:
    AdminListener(Actor::identity_s identity);
    void operator()();

    void *zmqcontext;
    void *zmqresponder;
};

#endif // INFINISQLADMINLISTENER_H
