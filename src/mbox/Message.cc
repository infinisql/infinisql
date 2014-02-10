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
 * @file   Message.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Jan 21 05:05:25 2014
 * 
 * @brief  base and derived classes for messages passed between actors
 */

#include "Message.h"

Message::Message()
{
    
}

Message::Message(topic_e topic, payloadtype_e payloadtype, int16_t destnodeid)
    : message({topic, payloadtype, {0, 0}, {destnodeid, 0}})
{
    
}

Message::~Message()
{
    
}

void Message::ser(Serdes &output)
{
    output.ser((void *)&message, sizeof(message));
}

size_t Message::sersize()
{
    return sizeof(message);
}

Serdes *Message::sermsg()
{
    Serdes *serobj;
    switch (message.payloadtype)
    {
    case PAYLOAD_MESSAGE:
        serobj=new (std::nothrow) Serdes(sersize());
        if (serobj != NULL)
        {
            ser(*serobj);
        }
        break;

    case PAYLOAD_SOCKET:
    {
        MessageSocket &msgRef=*(MessageSocket *)this;
        serobj=new (std::nothrow) Serdes(msgRef.sersize());
        if (serobj != NULL)
        {
            msgRef.ser(*serobj);
        }
    }
    break;
        
    default:
        LOG("can't serialize payloadtype " << message.payloadtype);
        serobj=nullptr;
    }

    return serobj;
}

Message *Message::deserialize(Serdes &input)
{
    message_s messagedata;
    input.des((void *)&messagedata, sizeof(messagedata));
    Message *deserializedMessage;
    switch (messagedata.payloadtype)
    {
    case PAYLOAD_MESSAGE:
        deserializedMessage=(Message *)new (std::nothrow) Message;
        if (deserializedMessage != nullptr)
        {
            deserializedMessage->message=messagedata;
        }
        break;

    case PAYLOAD_SOCKET:
        deserializedMessage=(Message *)new (std::nothrow) MessageSocket;
        if (deserializedMessage != nullptr)
        {
            MessageSocket &msgRef=*(MessageSocket *)deserializedMessage;
            msgRef.message=messagedata;
            input.des((void *)&msgRef.socketdata, sizeof(msgRef.socketdata));
        }
        break;        

    default:
        LOG("can't deserialize message type " << messagedata.payloadtype);
        deserializedMessage=nullptr;
    }

    return deserializedMessage;
}

void Message::setEnvelope(address_s &sourceAddress,
                          address_s &destinationAddress)
{
    message.sourceAddress=sourceAddress;
    message.destinationAddress=destinationAddress;
}

MessageSocket::MessageSocket()
{
    
}

MessageSocket::MessageSocket(topic_e topic, int16_t destnodeid, int sockfd,
                             uint32_t events)
    : Message(topic, PAYLOAD_SOCKET, destnodeid), socketdata({sockfd, events})
{
    
}

void MessageSocket::ser(Serdes &output)
{
    Message::ser(output);
    output.ser((void *)&socketdata, sizeof(socketdata));
}

size_t MessageSocket::sersize()
{
    return Message::sersize() + sizeof(message);
}

void MessageSocket::des(Serdes &input)
{
    input.des((void *)&socketdata, sizeof(socketdata));
}

MessageBatch::MessageBatch()
{
    
}

MessageBatch::MessageBatch(int16_t destnodeid)
    : Message(TOPIC_BATCH, PAYLOAD_BATCH, destnodeid), nmsgs(0), messagebatch()
{
    
}
