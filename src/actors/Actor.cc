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
 * @date   Tue Jan 21 04:26:26 2014
 * 
 * @brief  base class for Actors
 */

#include "Actor.h"

//std::vector< std::atomic<int> > socketAffinity;
std::atomic<int64_t> *socketAffinity;

Actor::Actor(identity_s identity) : identity(identity), msgrcv(nullptr),
                                    nextuserschemaoperationid(1)
{
    myTopology.update();
}

void Actor::operator()() const
{
}

Actor::~Actor()
{    
}

int64_t Actor::getnextuserschemaoperationdid()
{
    return ++nextuserschemaoperationid;
}

void Actor::getmsg(int timeout)
{
    msgrcv=identity.mbox->receive(timeout);
    if (msgrcv != nullptr && msgrcv->message.topic==Message::TOPIC_SERIALIZED)
    {
        msgrcv=
            Message::deserialize(((MessageSerialized *)msgrcv)->serializeddata);
    }
}

void Actor::sendObBatch()
{
    if (myTopology.obGateway != nullptr && myTopology.obBatch != nullptr)
    {
        myTopology.obGateway->sendMsg(*myTopology.obBatch);
        myTopology.obBatch=nullptr;
    }
}

void Actor::sendMsg(Message &msg)
{
    if (myTopology.nodeid != msg.message.destinationAddress.nodeid)
    {
        if (myTopology.obBatch==nullptr)
        {
            myTopology.obBatch=new (std::nothrow) MessageBatch;
            if (myTopology.obBatch==nullptr)
            {
                return; // node should probably die though
            }
            myTopology.obBatch->messagebatch[myTopology.obBatch->nmsgs++]=
                {msg.message.destinationAddress.nodeid, msg.sermsg()};
            delete &msg;
            if (myTopology.obBatch->nmsgs==OBGWMSGBATCHSIZE)
            {
                sendObBatch();
            }
        }

        return;
    }

    Mbox *destmbox;
    try
    {
        destmbox=myTopology.actoridToMboxes.at(msg.message.destinationAddress.actorid);
    }
    catch (...)
    {
        myTopology.update();
    }
    try
    {
        destmbox=myTopology.actoridToMboxes.at(msg.message.destinationAddress.actorid);        
    }
    catch (...)
    {
        LOG("no actorid " << msg.message.destinationAddress.actorid);
        return;
    }
    if (destmbox==nullptr)
    {
        LOG("no mailbox for actorid " << msg.message.destinationAddress.actorid);
        return;
    }
    destmbox->sendMsg(msg);
}

void Actor::setEnvelope(const Message::address_s &sourceAddress,
                     const Message::address_s &destinationAddress,
                     Message &msg)
{
    msg.message.sourceAddress=sourceAddress;
    msg.message.destinationAddress=destinationAddress;
}

void Actor::toActor(const Message::address_s &destinationAddress, Message &msg)
{
    setEnvelope(identity.address, destinationAddress, msg);
    sendMsg(msg);
}

void Actor::toUserSchemaManager(Message &msg)
{
    toActor(myTopology.userSchemaManager, msg);
}
