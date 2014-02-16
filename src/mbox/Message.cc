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
        if (serobj != nullptr)
        {
            ser(*serobj);
        }
        break;

    case PAYLOAD_SOCKET:
    {
        MessageSocket &msgRef=*(MessageSocket *)this;
        serobj=new (std::nothrow) Serdes(msgRef.sersize());
        if (serobj != nullptr)
        {
            msgRef.ser(*serobj);
        }
    }
    break;

    case PAYLOAD_USERSCHEMA:
    {
        MessageUserSchema &msgRef=*(MessageUserSchema *)this;
        serobj=new (std::nothrow) Serdes(msgRef.sersize());
        if (serobj != nullptr)
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
            msgRef.des(input);
        }
        break;

    case PAYLOAD_USERSCHEMA:
        deserializedMessage=(Message *)new (std::nothrow) MessageUserSchema;
        if (deserializedMessage != nullptr)
        {
            MessageUserSchema &msgRef=*(MessageUserSchema *)deserializedMessage;
            msgRef.message=messagedata;
            msgRef.des(input);
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

MessageTransaction::MessageTransaction()
{
    
}

MessageTransaction::~MessageTransaction()
{
    
}

void MessageTransaction::ser(Serdes &output)
{
    Message::ser(output);
    output.ser((void *)&transactiondata, sizeof(transactiondata));
}

size_t MessageTransaction::sersize()
{
    return Message::sersize() + sizeof(transactiondata);
}

void MessageTransaction::des(Serdes &input)
{
    input.des((void *)&transactiondata, sizeof(transactiondata));
}

MessageUserSchema::MessageUserSchema()
{
    
}

void MessageUserSchema::ser(Serdes &output)
{
    MessageTransaction::ser(output);
    output.ser((void *)&userschemadata, sizeof(userschemadata));
    output.ser(name);
    output.ser(partitiongroupname);
}

size_t MessageUserSchema::sersize()
{
    return MessageTransaction::sersize() + sizeof(userschemadata) +
        Serdes::sersize(name) + Serdes::sersize(partitiongroupname);
}

void MessageUserSchema::des(Serdes &input)
{
    MessageTransaction::des(input);
    input.des((void *)&userschemadata, sizeof(userschemadata));
    input.des(name);
    input.des(partitiongroupname);
}

MessageUserSchemaReply::MessageUserSchemaReply()
{
    
}

void MessageUserSchemaReply::ser(Serdes &output)
{
    MessageTransaction::ser(output);
    output.ser((void *)&userschemareplydata, sizeof(userschemareplydata));
}

size_t MessageUserSchemaReply::sersize()
{
    return MessageTransaction::sersize() + sizeof(userschemareplydata);
}

void MessageUserSchemaReply::des(Serdes &input)
{
    MessageTransaction::des(input);
    input.des((void *)&userschemareplydata, sizeof(userschemareplydata));
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

