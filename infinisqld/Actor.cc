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
 * @file   Actor.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Wed Jan  1 15:21:23 2014
 * 
 * @brief  base class for actors
 */

#include "Actor.h"
#line 30 "Actor.cc"

Actor::Actor()
{
}

Actor::~Actor()
{
}

void Actor::init(Topology::actorIdentity *myIdentityArg)
{
    myIdentity=*myIdentityArg;
    delete myIdentityArg;
    mboxes.nodeid=myIdentity.address.nodeid;
    mboxes.update(myTopology, myIdentity.instance);
}

void Actor::getmsg(int timeout)
{
    msgrcv=myIdentity.mbox->receive(timeout);
    if (msgrcv != NULL && msgrcv->messageStruct.topic==TOPIC_SERIALIZED)
    {
        SerializedMessage serobj(((class MessageSerialized *)msgrcv)->data);
        switch (serobj.getpayloadtype())
        {
        case PAYLOADMESSAGE:
        {
            reuseMessage=Message();
            reuseMessage.unpack(serobj);
            if (serobj.data->size() != serobj.pos)
            {
                fprintf(logfile, "unpack %i size %lu pos %lu\n",
                        serobj.getpayloadtype(),
                        (unsigned long)serobj.data->size(),
                        (unsigned long)serobj.pos);
            }
            msgrcv=&reuseMessage;
        }
        break;
        case PAYLOADSOCKET:
        {
            reuseMessageSocket=MessageSocket();
            reuseMessageSocket.unpack(serobj);
            if (serobj.data->size() != serobj.pos)
            {
                fprintf(logfile, "unpack %i size %lu pos %lu\n",
                        serobj.getpayloadtype(),
                        (unsigned long)serobj.data->size(),
                        (unsigned long)serobj.pos);
            }
            msgrcv=&reuseMessageSocket;
        }
        break;
        case PAYLOADUSERSCHEMA:
        {
            reuseMessageUserSchema=MessageUserSchema();
            reuseMessageUserSchema.unpack(serobj);
            if (serobj.data->size() != serobj.pos)
            {
                fprintf(logfile, "unpack %i size %lu pos %lu\n",
                        serobj.getpayloadtype(),
                        (unsigned long)serobj.data->size(),
                        (unsigned long)serobj.pos);
            }
            msgrcv=&reuseMessageUserSchema;
        }
        break;
        case PAYLOADDEADLOCK:
        {
            reuseMessageDeadlock=MessageDeadlock();
            reuseMessageDeadlock.unpack(serobj);
            if (serobj.data->size() != serobj.pos)
            {
                fprintf(logfile, "unpack %i size %lu pos %lu\n",
                        serobj.getpayloadtype(),
                        (unsigned long)serobj.data->size(),
                        (unsigned long)serobj.pos);
            }
            msgrcv=&reuseMessageDeadlock;
        }
        break;
        case PAYLOADSUBTRANSACTION:
        {
            reuseMessageSubtransactionCmd=MessageSubtransactionCmd();
            reuseMessageSubtransactionCmd.unpack(serobj);
            if (serobj.data->size() != serobj.pos)
            {
                fprintf(logfile, "unpack %i size %lu pos %lu\n",
                        serobj.getpayloadtype(),
                        (unsigned long)serobj.data->size(),
                        (unsigned long)serobj.pos);
            }
            msgrcv=&reuseMessageSubtransactionCmd;
        }
        break;
        case PAYLOADCOMMITROLLBACK:
        {
            reuseMessageCommitRollback=MessageCommitRollback();
            reuseMessageCommitRollback.unpack(serobj);
            if (serobj.data->size() != serobj.pos)
            {
                fprintf(logfile, "unpack %i size %lu pos %lu\n",
                        serobj.getpayloadtype(),
                        (unsigned long)serobj.data->size(),
                        (unsigned long)serobj.pos);
            }
            msgrcv=&reuseMessageCommitRollback;
        }
        break;
        case PAYLOADDISPATCH:
        {
            reuseMessageDispatch=MessageDispatch();
            reuseMessageDispatch.unpack(serobj);
            if (serobj.data->size() != serobj.pos)
            {
                fprintf(logfile, "unpack %i size %lu pos %lu\n",
                        serobj.getpayloadtype(),
                        (unsigned long)serobj.data->size(),
                        (unsigned long)serobj.pos);
            }
            msgrcv=&reuseMessageDispatch;
        }
        break;
        case PAYLOADACKDISPATCH:
        {
            reuseMessageAckDispatch=MessageAckDispatch();
            reuseMessageAckDispatch.unpack(serobj);
            if (serobj.data->size() != serobj.pos)
            {
                fprintf(logfile, "unpack %i size %lu pos %lu\n",
                        serobj.getpayloadtype(),
                        (unsigned long)serobj.data->size(),
                        (unsigned long)serobj.pos);
            }
            msgrcv=&reuseMessageAckDispatch;
        }
        break;
        case PAYLOADAPPLY:
        {
            reuseMessageApply=MessageApply();
            reuseMessageApply.unpack(serobj);
            if (serobj.data->size() != serobj.pos)
            {
                fprintf(logfile, "unpack %i size %lu pos %lu\n",
                        serobj.getpayloadtype(),
                        (unsigned long)serobj.data->size(),
                        (unsigned long)serobj.pos);
            }
            msgrcv=&reuseMessageApply;
        }
        break;
        case PAYLOADACKAPPLY:
        {
            reuseMessageAckApply=MessageAckApply();
            reuseMessageAckApply.unpack(serobj);
            if (serobj.data->size() != serobj.pos)
            {
                fprintf(logfile, "unpack %i size %lu pos %lu\n",
                        serobj.getpayloadtype(),
                        (unsigned long)serobj.data->size(),
                        (unsigned long)serobj.pos);
            }
            msgrcv=&reuseMessageAckApply;
        }
        break;
        default:
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, serobj.getpayloadtype());
        }
        delete serobj.data;
    }
}
