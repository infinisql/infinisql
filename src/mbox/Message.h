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
            TOPIC_BATCH
    };
    /** 
     * @brief type of message
     */
    enum payloadtype_e : uint8_t
    {
        PAYLOAD_NONE=0,
            PAYLOAD_MESSAGE,
            PAYLOAD_SOCKET,
            PAYLOAD_BATCH
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
     * @brief serialize message
     *
     * @param output output object
     */
    void ser(Serdes &output);
    /** 
     * @brief size of serialized message
     *
     * @return size
     */
    size_t sersize();

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

    /** 
     * @brief set source and destination addresses for message
     *
     * @param sourceAddress source address
     * @param destinationAddress destination address
     */
    void setEnvelope(address_s &sourceAddress, address_s &destinationAddress);

    __int128 nextmsg;
    message_s message;
};

class MessageSocket : public Message
{
public:
    enum listenertype_e : uint8_t
    {
        LISTENER_NONE=0,
            LISTENER_PG
            };
    struct __attribute__ ((__packed__)) socketdata_s
    {
        int sockfd;
        uint32_t events;
        listenertype_e listenertype;
    };

    MessageSocket();
    MessageSocket(topic_e topic, int16_t destnodeid, int sockfd,
                  uint32_t events);

    void ser(Serdes &output);
    size_t sersize();
    void des(Serdes &input);

    socketdata_s socketdata;
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
    MessageBatch(int16_t destnodeid);

    int16_t nmsgs;
    messagebatch_s messagebatch[OBGWMSGBATCHSIZE];
};

#endif // INFINISQLMESSAGE_H
