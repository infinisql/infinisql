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
 * @file   AdminListener.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Thu Jan 23 02:27:09 2014
 * 
 * @brief  0mq listener for administrative commands from cluster manager
 */

#include "AdminListener.h"
#include <zmq.h>

AdminListener::AdminListener(Actor::identity_s identity)
    : Actor(identity), zmqcontext(NULL), zmqresponder(NULL)
{
}

void AdminListener::operator()()
{
    void *zmqcontext=zmq_ctx_new();
    if (zmqcontext==NULL)
    {
        LOG("can't create 0mq context, exiting");
        exit(1);
    }
    zmqresponder=zmq_socket(zmqcontext, ZMQ_REP);
    if (zmqresponder==NULL)
    {
        LOG("can't create 0mq socket, exiting");
        exit(1);
    }
    if (zmq_bind(zmqresponder, identity.zmqhostport.c_str())==-1)
    {
        LOG("can't zmq_bind, exiting");
        exit(1);
    }

    while(1)
    {
        sleep(10);
    }
}
