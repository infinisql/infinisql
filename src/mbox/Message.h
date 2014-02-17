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
 * @file   Message.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Jan 21 04:49:59 2014
 * 
 * @brief  base and derived classes for messages passed between actors
 */

#ifndef INFINISQLMESSAGE_H
#define INFINISQLMESSAGE_H

#include <cstdint>
#include <lmdb.h>
#include "Serdes.h"
#include "../engine/Metadata.h"
#include "../engine/Catalog.h"
#include "../engine/Schema.h"
#include "../engine/Table.h"
#include "../engine/Field.h"
#include "../engine/Index.h"

#define OBGWMSGBATCHSIZE 5000

/** 
 * @brief base class for Messages, also a sendable message
 *
 */
class Message
{
public:
    /** 
     * @brief Message topic
     */
    enum topic_e : uint8_t
    {
        TOPIC_NONE=0,
            TOPIC_SOCKET,
            TOPIC_SOCKETCONNECTED,
            TOPIC_BATCH,
            TOPIC_SERIALIZED,
            TOPIC_USERSCHEMA,
            TOPIC_USERSCHEMAREPLY
    };
    /** 
     * @brief type of message
     */
    enum payloadtype_e : uint8_t
    {
        PAYLOAD_NONE=0,
            PAYLOAD_MESSAGE,
            PAYLOAD_SOCKET,
            PAYLOAD_BATCH,
            PAYLOAD_SERIALIZED,
            PAYLOAD_USERSCHEMA,
            PAYLOAD_USERSCHEMAREPLY
            };
    /** 
     * @brief address for message delivery
     */
    struct address_s
    {
        int16_t nodeid;
        int16_t actorid;
    };
    /** 
     * @brief POD contents of message
     */
    struct __attribute__ ((__packed__)) message_s
    {
        topic_e topic;
        payloadtype_e payloadtype;
        address_s sourceAddress;
        address_s destinationAddress;
    };
    
    Message();
    /** 
     * @brief base class constructor for various message types
     *
     * @param topic message topic
     * @param payloadtype type of message
     * @param destnodeid destination node
     */
    Message(topic_e topic, payloadtype_e payloadtype, int16_t destnodeid);
    virtual ~Message();

    /** 
     * @brief serialize entire message, create and return object
     *
     *
     * @return serialized message object
     */
    Serdes *sermsg();
    /** 
     * @brief deserialize entire message
     *
     * creates appropriate message type based on payloadtype, deserializes
     * in its entirety
     * 
     * @param input serialized object
     *
     * @return resultant message
     */
    static Message *deserialize(Serdes &input);

    __int128 nextmsg;
    message_s message;
};

class MessageSocket : public Message
{
public:
    struct __attribute__ ((__packed__)) socketdata_s
    {
        int sockfd;
        uint32_t events;
    };

    MessageSocket();
    MessageSocket(topic_e topic, int16_t destnodeid, int sockfd,
                  uint32_t events);

    socketdata_s socketdata;
};

class MessageTransaction : public Message
{
public:
    struct __attribute__ ((__packed__)) continuationpoint_s
    {
        int64_t callerid;
        int8_t function;
        int8_t entrypoint;

        bool operator==(const continuationpoint_s &orig) const
        {
            if (orig.callerid==callerid && orig.function==function &&
                orig.entrypoint==entrypoint)
            {
                return true;
            }
            return false;
        }
    };
    struct __attribute__ ((__packed__)) transactiondata_s
    {
        continuationpoint_s continuationpoint;
        int64_t requestid;
    };
    enum status_e : int8_t
    {
        STATUS_NONE = 0,
            STATUS_OK,
            STATUS_NOK
            };
    enum reason_e : int8_t
    {
        REASON_NONE = 0,
            REASON_GENERIC
            };
    
    MessageTransaction();
    virtual ~MessageTransaction();

    transactiondata_s transactiondata;
};

class MessageUserSchema : public MessageTransaction
{
public:
    enum crud_e : uint8_t
    {
        CRUD_NONE = 0,
            CRUD_CREATE,
            CRUD_ALTER,
            CRUD_DROP
            };
    enum meta_e : uint8_t
    {
        META_NONE = 0,
            META_CATALOG,
            META_SCHEMA,
            META_TABLE,
            META_FIELD,
            META_INDEX,
            META_USER
            };
    struct columnactivity_s
    {
        crud_e crud;
        std::string name;
        int16_t id;
        Field::type_e type;
        int64_t size;
        int64_t precision;
        int64_t scale;
        FieldValue defaultValue;
        bool nullconstraint;
    };
    struct indexactivity_s
    {
        std::string name;
        int16_t id;
        std::vector<int16_t> fieldids;
        bool uniqueconstraint;
    };
    struct __attribute__ ((__packed__)) userschemadata_s
    {
        crud_e crud;
        meta_e meta;
        int16_t id;
        int16_t parentCatalogid;
        int16_t parentSchemaid;
        int16_t parentTableid;
        int16_t partitiongroupid;
    };

    MessageUserSchema();

    userschemadata_s userschemadata;

    std::string name;
    std::string partitiongroupname;

};

class MessageUserSchemaReply : public MessageTransaction
{
public:
    struct userschemareplydata_s
    {
        status_e status;
        reason_e reason;
        int16_t id;
    };
    
    MessageUserSchemaReply();

    userschemareplydata_s userschemareplydata;
};

class MessageBatch : public Message
{
public:
    struct messagebatch_s
    {
        int16_t nodeid;
        Serdes *serializedMessage;
    };

    MessageBatch();

    int16_t nmsgs;
    messagebatch_s messagebatch[OBGWMSGBATCHSIZE];
};

class MessageSerialized : public Message
{
public:
    MessageSerialized(const Serdes &serializeddata);

    Serdes serializeddata;
};

void ser(const Message &d, Serdes &output);
size_t sersize(const Message &d);
void des(Serdes &input, Message &d);

void ser(const MessageSocket &d, Serdes &output);
size_t sersize(const MessageSocket &d);
void des(Serdes &input, MessageSocket &d);

void ser(const MessageTransaction &d, Serdes &output);
size_t sersize(const MessageTransaction &d);
void des(Serdes &input, MessageTransaction &d);

void ser(const MessageUserSchema &d, Serdes &output);
size_t sersize(const MessageUserSchema &d);
void des(Serdes &input, MessageUserSchema &d);

void ser(const MessageUserSchemaReply &d, Serdes &output);
size_t sersize(const MessageUserSchemaReply &d);
void des(Serdes &input, MessageUserSchemaReply &d);

#endif // INFINISQLMESSAGE_H
