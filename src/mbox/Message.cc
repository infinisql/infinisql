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

Serdes *Message::sermsg()
{
    Serdes *serobj;
    switch (message.payloadtype)
    {
    case PAYLOAD_MESSAGE:
        serobj=new (std::nothrow) Serdes(sersize(*this));
        if (serobj != nullptr)
        {
            ser(*this, *serobj);
        }
        break;

    case PAYLOAD_SOCKET:
    {
        MessageSocket &msgRef=*(MessageSocket *)this;
        serobj=new (std::nothrow) Serdes(sersize(msgRef));
        if (serobj != nullptr)
        {
            ser(msgRef, *serobj);
        }
    }
    break;

    case PAYLOAD_USERSCHEMA:
    {
        MessageUserSchema &msgRef=*(MessageUserSchema *)this;
        serobj=new (std::nothrow) Serdes(sersize(msgRef));
        if (serobj != nullptr)
        {
            ser(msgRef, *serobj);
        }
    }
    break;

    case PAYLOAD_USERSCHEMAREPLY:
    {
        MessageUserSchemaReply &msgRef=*(MessageUserSchemaReply *)this;
        serobj=new (std::nothrow) Serdes(sersize(msgRef));
        if (serobj != nullptr)
        {
            ser(msgRef, *serobj);
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
    des(input, (void *)&messagedata, sizeof(messagedata));
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
            des(input, *deserializedMessage);
        }
        break;

    case PAYLOAD_USERSCHEMA:
        deserializedMessage=(Message *)new (std::nothrow) MessageUserSchema;
        if (deserializedMessage != nullptr)
        {
            MessageUserSchema &msgRef=*(MessageUserSchema *)deserializedMessage;
            msgRef.message=messagedata;
            des(input, *deserializedMessage);
        }
        break;

    case PAYLOAD_USERSCHEMAREPLY:
        deserializedMessage=(Message *)new (std::nothrow)
            MessageUserSchemaReply;
        if (deserializedMessage != nullptr)
        {
            MessageUserSchemaReply &msgRef=
                *(MessageUserSchemaReply *)deserializedMessage;
            msgRef.message=messagedata;
            des(input, *deserializedMessage);
        }
        break;

    default:
        LOG("can't deserialize message type " << messagedata.payloadtype);
        deserializedMessage=nullptr;
    }

    return deserializedMessage;
}

MessageSocket::MessageSocket()
{
    
}

MessageSocket::MessageSocket(topic_e topic, int16_t destnodeid, int sockfd,
                             uint32_t events)
    : Message(topic, PAYLOAD_SOCKET, destnodeid), socketdata({sockfd, events})
{
    
}

MessageTransaction::MessageTransaction()
{
    
}

MessageTransaction::~MessageTransaction()
{
    
}

MessageUserSchema::MessageUserSchema()
{
    
}

MessageUserSchemaReply::MessageUserSchemaReply()
{
    
}

MessageBatch::MessageBatch()
    : Message(TOPIC_BATCH, PAYLOAD_BATCH, 0), nmsgs(0), messagebatch()
{
    
}

MessageSerialized::MessageSerialized(const Serdes &serializeddata)
    : serializeddata(serializeddata)
{
    memcpy(&message, serializeddata.val.mv_data, sizeof(message));
    message.topic=TOPIC_SERIALIZED;
    message.payloadtype=PAYLOAD_SERIALIZED;
}

void ser(const Message &d, Serdes &output)
{
    ser((char *)&d.message, sizeof(d.message), output);
}

size_t sersize(const Message &d)
{
    return sizeof(d.message);
}

void des(Serdes &input, Message &d)
{
    des(input, &d.message, sizeof(d.message));
}

void ser(const MessageSocket &d, Serdes &output)
{
    ser((const Message &)d, output);
    ser((char *)&d.socketdata, sizeof(d.socketdata), output);
}

size_t sersize(const MessageSocket &d)
{
    return sersize((const Message &)d) + sizeof(d.socketdata);
}

/* no need to deserialize the Message base class, because that's already
 * done by Message::deserialize()
 */
void des(Serdes &input, MessageSocket &d)
{
    des(input, &d.socketdata, sizeof(d.socketdata));
}

void ser(const MessageTransaction &d, Serdes &output)
{
    ser((const Message &)d, output);
    ser((char *)&d.transactiondata, sizeof(d.transactiondata), output);
}

size_t sersize(const MessageTransaction &d)
{
    return sersize((const Message &)d) + sizeof(d.transactiondata);
}

void des(Serdes &input, MessageTransaction &d)
{
    des(input, &d.transactiondata, sizeof(d.transactiondata));
}

void ser(const MessageUserSchema &d, Serdes &output)
{
    ser((const MessageTransaction &)d, output);
    ser((char *)&d.userschemadata, output);
    ser(d.name, output);
    ser(d.partitiongroupname, output);
}

size_t sersize(const MessageUserSchema &d)
{
    return sersize((const MessageTransaction &)d) + sizeof(d.userschemadata) +
        sersize(d.name) + sersize(d.partitiongroupname);
}

void des(Serdes &input, MessageUserSchema &d)
{
    des(input, (MessageTransaction &)d);
    des(input, &d.userschemadata, sizeof(d.userschemadata));
    des(input, d.name);
    des(input, d.partitiongroupname);
}

void ser(const MessageUserSchemaReply &d, Serdes &output)
{
    ser((const MessageTransaction &)d, output);
    ser((char *)&d.userschemareplydata, output);
}

size_t sersize(const MessageUserSchemaReply &d)
{
    return sersize((const MessageTransaction &)d) +
        sizeof(d.userschemareplydata);
}

void des(Serdes &input, MessageUserSchemaReply &d)
{
    des(input, (MessageTransaction &)d);
    des(input, &d.userschemareplydata, sizeof(d.userschemareplydata));
}
