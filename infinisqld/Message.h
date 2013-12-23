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
 * @date   Tue Dec 17 13:34:17 2013
 * 
 * @brief  Message objects of varying types. Each type eventually inherits
 * from the Message base class. Each type serves a different role in
 * inter-actor communication.
 */

#ifndef INFINISQLMESSAGE_H
#define INFINISQLMESSAGE_H

#include "gch.h"
#include "defs.h"
#include "Topology.h"

/* THIS STUFF PROBABLY NEEDS TO BE CONSIDERED THROUGHOUT THE CODE BASE
 * BECAUSE SIZES CHANGED FROM int64_t TO OTHER THINGS, SUCH AS int16_t
 Message:
 start 34
 Topology::addressStruct
 nodeid -> int16_t
 actorid -> int16_t
 finish 10

 MessageTransaction:
 start 65
 tainstance -> int16_t
 domainid -> int16_t
 transaction_pendingcmdid -> int32_t
 transaction_tacmdentrypoint -> int8_t
 engineinstance -> int16_t
 finish 36

 MessageSubtransactionCmd:
 subtransaction_s
 start 58
 status -> int8_t
 tableid -> int16_t
 fieldid -> int16_t
 engineid -> int16_t
 finish 33

 start 8+16 per entry
 indexHits (nonLockingIndexEntry_s):
 engineid -> int16_t
 finish 8 + 10 per entry

 MessageCommitRollback:
 rofs, rowOrField_s
 start 52 + fieldVal(25)
 tableid -> int16_t
 fieldid -> int16_t
 engineid -> int16_t
 newengineid -> int16_t
 finish 28 + fieldVal(25)
*/

/** 
 * @brief create Message object
 *
 *
 * @return 
 */
class Message
{
public:
    struct __attribute__ ((__packed__)) message_s
    {
        payloadtype_e payloadtype;
        topic_e topic;
        Topology::addressStruct sourceAddr;
        Topology::addressStruct destAddr;
    };
  
    Message();
    virtual ~Message();
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    std::string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();
    /** 
     * @brief deserialize entire message
     *
     * @param serstr string of serialized message
     *
     * @return deserialized message
     */
    static class Message *des(string *serstr);
    /** 
     * @brief return serialized Message variant string
     *
     *
     * @return serialized Message variant string
     */
    string *sermsg();
    /** 
     * @brief put addresses in Message variant envelope
     *
     * @param source source address
     * @param dest destination address
     * @param msg Message
     */
    void setEnvelope(const Topology::addressStruct &source,
                     const Topology::addressStruct &dest, class Message &msg);

    __int128 nextmsg;
    message_s messageStruct;
};

/** 
 * @brief Message variant for communicating socket events
 *
 * @return 
 */
class MessageSocket : public Message
{
public:
    struct __attribute__ ((__packed__)) socket_s
    {
        int socket;
        uint32_t events;
        listenertype_e listenertype;
    };
    MessageSocket();
    /** 
     * @brief Message variant for communicating socket events
     *
     * @param socketarg socket discriptor
     * @param eventsarg epoll events
     * @param listenertype type of listener
     * @param nodeidarg nodeid
     * @param topicarg topic
     */
    MessageSocket(int socketarg, uint32_t eventsarg,
                  listenertype_e listenertype, int64_t nodeidarg,
                  topic_e topicarg);
    virtual ~MessageSocket();
    
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();

    socket_s socketStruct;
};

/** 
 * @brief Message variant for UserSchemaMgr activities
 *
 * @return 
 */
class MessageUserSchema : public Message
{
public:
    struct __attribute__ ((__packed__)) userschema_s
    {
        operationtype_e operationtype;
        int8_t caller;
        int8_t callerstate;
        int64_t argsize;
        int64_t instance;
        int64_t operationid;
        int64_t domainid;
        int64_t userid;
        int64_t tableid;
        int64_t fieldlen;
        int64_t builtincmd;
        int64_t indexid;
        int64_t tableindexid;
        int64_t simple;
        int64_t fieldid;
        int64_t numfields;
        int64_t intdata;
        int64_t status;
        indextype_e indextype;
        fieldtype_e fieldtype;
    };
    MessageUserSchema();
    /** 
     * @brief Message variant for UserSchemaMgr activities
     *
     * @param topicarg topic
     *
     * @return 
     */
    MessageUserSchema(topic_e topicarg);
    virtual ~MessageUserSchema();
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();

    userschema_s userschemaStruct;
    procedures_s procs;
    std::string argstring;
    std::string pathname;
    std::string procname;
    std::string username;
    std::string domainname;
    std::string password;
};

/** 
 * @brief Message variant for DeadlockMgr activities
 *
 *
 * @return 
 */
class MessageDeadlock : public Message
{
public:
    struct __attribute__ ((__packed__)) deadlock_s
    {
        int64_t transactionid;
        int64_t tainstance;
        int64_t transaction_pendingcmdid;
        int64_t deadlockchange;
    };
    MessageDeadlock();
    virtual ~MessageDeadlock();
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();

    deadlock_s deadlockStruct;
    std::string deadlockNode;
    newDeadLockLists_s nodes;
};

/** 
 * @brief Message variant for transaction processing
 *
 *
 * @return 
 */
class MessageTransaction : public Message
{
public:
    struct __attribute__ ((__packed__)) transaction_s
    {
        int64_t transactionid;
        int64_t subtransactionid;
        int64_t previoussubtransactionid;
        int16_t tainstance;
        int16_t domainid;
        enginecmd_e transaction_enginecmd;
        int32_t transaction_pendingcmdid;
        int8_t transaction_tacmdentrypoint;
        int16_t engineinstance;
    };
    MessageTransaction();
    virtual ~MessageTransaction();
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();

    transaction_s transactionStruct;
};

/** 
 * @brief Message variant for subtransactions
 *
 *
 * @return 
 */
class MessageSubtransactionCmd : public MessageTransaction
{
public:
    struct __attribute__ ((__packed__)) subtransaction_s
    {
        int8_t status;
        bool isrow;
        int64_t rowid;
        int16_t tableid;
        locktype_e locktype;
        int64_t forward_rowid;
        int16_t forward_engineid;
        int16_t fieldid;
        int16_t engineid; // index also uses rowid
    };
    MessageSubtransactionCmd();
    virtual ~MessageSubtransactionCmd();
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();

    subtransaction_s subtransactionStruct;

    std::string row;
    fieldValue_s fieldVal;
    std::vector<nonLockingIndexEntry_s> indexHits;
    searchParams_s searchParameters;
    std::vector<int64_t> rowids;
    std::vector<returnRow_s> returnRows;
};

/** 
 * @brief Message variant for committing and rolling back transactions
 *
 *
 * @return 
 */
class MessageCommitRollback : public MessageTransaction
{
public:
    MessageCommitRollback();
    virtual ~MessageCommitRollback();
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();

    std::vector<rowOrField_s> rofs;
};

/** 
 * @brief Message variant for synchronous replication
 *
 *
 * @return 
 */
class MessageDispatch : public Message
{
public:
    struct __attribute__ ((__packed__)) dispatch_s
    {
        int64_t transactionid;
        int64_t domainid;
    };
  
    struct record_s
    {
        int64_t rowid;
        pendingprimitive_e primitive;
        int64_t tableid;
        int64_t previoussubtransactionid;
        std::string row;
        std::string oldrow;
    };

    MessageDispatch();
    virtual ~MessageDispatch();
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();

    dispatch_s dispatchStruct;
  
    // pidsids[partitionid] = subtransactionid
    boost::unordered_map<int64_t, int64_t> pidsids;
    // records[partitionid][#] = {recordinfo}
    boost::unordered_map< int64_t, std::vector<record_s> > records;
};

/** 
 * @brief Message variant for acknowledgement of synchronous replication
 *
 *
 * @return 
 */
class MessageAckDispatch : public Message
{
public:
    struct __attribute__ ((__packed__)) ackdispatch_s
    {
        int64_t transactionid;
        int status;
    };
  
    MessageAckDispatch();
    /** 
     * Message variant for acknowledgement of synchronous replication
     *
     * @param transactionidarg transactionid
     * @param statusarg replication status
     *
     * @return 
     */
    MessageAckDispatch(int64_t transactionidarg, int statusarg);
    virtual ~MessageAckDispatch();
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();

    ackdispatch_s ackdispatchStruct;
};

#define ADDFLAG 0

/** 
 * @brief Message variant for Applier activities
 *
 *
 * @return 
 */
class MessageApply : public Message
{
public:
    struct __attribute__ ((__packed__)) apply_s
    {
        int64_t subtransactionid;
        int64_t applierid;
        int64_t domainid;
    };
    struct applyindex_s
    {
        fieldValue_s fieldVal;
        nonLockingIndexEntry_s entry;
        char flags;
        int16_t tableid;
        int16_t fieldid;
    };

    MessageApply();
    /** 
     * @brief Message variant for Applier activities
     *
     * @param subtransactionidarg subtransactionid
     * @param applieridarg applierid
     * @param domainidarg domainid
     *
     * @return 
     */
    MessageApply(int64_t subtransactionidarg, int64_t applieridarg,
                 int64_t domainidarg);
    virtual ~MessageApply();
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();

    /** 
     * @brief set flag for new index entry
     *
     * @param c flags
     */
    static void setisaddflag(char *c);
    /** 
     * @brief get flag for new index entry
     *
     * @param c flags
     */
    static char getisaddflag(char c);
    /** 
     * @brief clear flag for new index entry
     *
     * @param c flags
     */
    static void cleariaddflag(char *c);

    apply_s applyStruct;
  
    std::vector<MessageDispatch::record_s> rows;
    std::vector<applyindex_s> indices;
};

/** 
 * @brief Message variant for acknowledging Applier activities
 *
 *
 * @return 
 */
class MessageAckApply : public Message
{
public:
    struct __attribute__ ((__packed__)) ackapply_s
    {
        int64_t subtransactionid;
        int64_t applierid;
        int64_t partitionid;
        int status;
    };
    MessageAckApply();
    /** 
     * @brief Message variant for acknowledging Applier activities
     *
     * @param subtransactionidarg subtransactionid
     * @param applieridarg  applierid
     * @param partitionidarg partitionid
     * @param statusarg status
     *
     * @return 
     */
    MessageAckApply(int64_t subtransactionidarg, int64_t applieridarg,
                    int64_t partitionidarg, int statusarg);
    virtual ~MessageAckApply();
    /** 
     * @brief get Message size
     *
     * @return size in bytes
     */
    size_t size();
    /** 
     * @brief create string with serialized message
     *
     *
     * @return serialized message string
     */
    string *ser();
    /** 
     * @brief serialize this
     *
     * @param serobj SerializedMessage
     */
    void package(class SerializedMessage &serobj);
    /** 
     * @brief deserialize into this
     *
     * @param serobj SerializedMessage
     */
    void unpack(SerializedMessage &serobj);
    /** 
     * @brief clear contents of this
     *
     */
    void clear();

    ackapply_s ackapplyStruct;
};

/** 
 * @brief object carrying a serialized Message variant
 *
 * 1) sender to obgw: SerializedMessage on stack
 * 2) copy pointer SerializedMessage.data to new MessageSerialized.data
 * 3) send MessageSerialized to obgw
 * 4) obgw takes MessageSerialized.data in
 *    boost_unordered::map< int64_t, vector<string *> > for sending to remote
 *    node
 * 5) obgw copies string *'s, deletes them, sends to remote node
 * 6) ibgw creates new MessageSerialized for each string *
 *    gets destAddr, etc by reading 1st sizeof(Message::message_s)
 * 7) destination copies MessageSerialized.data ptr to SerializedMessage.data
 * 8) destination deserializes into original Message*
 * 9) destination deletes SerializedMessage.data
 *
 * source actor is the sender of this
 *
 * @param sizearg size of serialized message
 *
 * @return 
 */
class SerializedMessage
{
public:
    // source sender
    SerializedMessage(size_t sizearg);
    /** 
     * @brief object carrying a serialized Message variant
     *
     * IbGateway is the sender 
     * 
     * @param dataarg serialized message to populate with
     */
    SerializedMessage(string *dataarg);
    virtual ~SerializedMessage();
  
    size_t size;
    size_t pos;
    std::string *data;
  
    payloadtype_e getpayloadtype();
    // raw
    void ser(size_t s, void *dataptr);
    void des(size_t s, void *dataptr);
    // pods
    void ser(int64_t d);
    static size_t sersize(int64_t d);
    void des(int64_t *d);
    void ser(int32_t d);
    static size_t sersize(int32_t d);
    void des(int32_t *d);
    void ser(int16_t d);
    static size_t sersize(int16_t d);
    void des(int16_t *d);
    void ser(int8_t d);
    static size_t sersize(int8_t d);
    void des(int8_t *d);
    // containers
    void ser(const string &d);
    static size_t sersize(const string &d);
    void des(string &d);
    void ser(vector<int64_t> &d);
    static size_t sersize(vector<int64_t> &d);
    void des(vector<int64_t> &d);
    void ser(boost::unordered_map<int64_t, int64_t> &d);
    static size_t sersize(boost::unordered_map<int64_t, int64_t> &d);
    void des(boost::unordered_map<int64_t, int64_t> &d);
    // pod structs
    void ser(Message::message_s &d);
    static size_t sersize(Message::message_s &d);
    void des(Message::message_s &d);
    void ser(MessageSocket::socket_s &d);
    static size_t sersize(MessageSocket::socket_s &d);
    void des(MessageSocket::socket_s &d);
    void ser(MessageUserSchema::userschema_s &d);
    static size_t sersize(MessageUserSchema::userschema_s &d);
    void des(MessageUserSchema::userschema_s &d);
    void ser(procedures_s &d);
    static size_t sersize(procedures_s &d);
    void des(procedures_s &d);
    void ser(MessageDeadlock::deadlock_s &d);
    static size_t sersize(MessageDeadlock::deadlock_s &d);
    void des(MessageDeadlock::deadlock_s &d);
    void ser(MessageTransaction::transaction_s &d);
    static size_t sersize(MessageTransaction::transaction_s &d);
    void des(MessageTransaction::transaction_s &d);
    void ser(MessageSubtransactionCmd::subtransaction_s &d);
    static size_t sersize(MessageSubtransactionCmd::subtransaction_s &d);
    void des(MessageSubtransactionCmd::subtransaction_s &d);
    void ser(nonLockingIndexEntry_s &d);
    static size_t sersize(nonLockingIndexEntry_s &d);
    void des(nonLockingIndexEntry_s &d);
    void ser(MessageDispatch::dispatch_s &d);
    static size_t sersize(MessageDispatch::dispatch_s &d);
    void des(MessageDispatch::dispatch_s &d);
    void ser(MessageAckDispatch::ackdispatch_s &d);
    static size_t sersize(MessageAckDispatch::ackdispatch_s &d);
    void des(MessageAckDispatch::ackdispatch_s &d);
    void ser(MessageApply::apply_s &d);
    static size_t sersize(MessageApply::apply_s &d);
    void des(MessageApply::apply_s &d);
    void ser(MessageAckApply::ackapply_s &d);
    static size_t sersize(MessageAckApply::ackapply_s &d);
    void des(MessageAckApply::ackapply_s &d);
    // level 1
    void ser(boost::unordered_set<string> &d);
    static size_t sersize(boost::unordered_set<string> &d);
    void des(boost::unordered_set<string> &d);
    void ser(fieldValue_s &d);
    static size_t sersize(fieldValue_s &d);
    void des(fieldValue_s &d);
    void ser(returnRow_s &d);
    static size_t sersize(returnRow_s &d);
    void des(returnRow_s &d);
    void ser(MessageDispatch::record_s &d);
    static size_t sersize(MessageDispatch::record_s &d);
    void des(MessageDispatch::record_s &d);
    void ser(vector<nonLockingIndexEntry_s> &d);
    static size_t sersize(vector<nonLockingIndexEntry_s> &d);
    void des(vector<nonLockingIndexEntry_s> &d);
    // level 2
    void ser(newDeadLockLists_s &d);
    static size_t sersize(newDeadLockLists_s &d);
    void des(newDeadLockLists_s &d);
    void ser(vector<fieldValue_s> &d);
    static size_t sersize(vector<fieldValue_s> &d);
    void des(vector<fieldValue_s> &d);
    void ser(vector<returnRow_s> &d);
    static size_t sersize(vector<returnRow_s> &d);
    void des(vector<returnRow_s> &d);
    void ser(vector<MessageDispatch::record_s> &d);
    static size_t sersize(vector<MessageDispatch::record_s> &d);
    void des(vector<MessageDispatch::record_s> &d);
    void ser(rowOrField_s &d);
    static size_t sersize(rowOrField_s &d);
    void des(rowOrField_s &d);
    void ser(MessageApply::applyindex_s &d);
    static size_t sersize(MessageApply::applyindex_s &d);
    void des(MessageApply::applyindex_s &d);
    // level 3
    void ser(searchParams_s &d);
    static size_t sersize(searchParams_s &d);
    void des(searchParams_s &d);
    void ser(vector<rowOrField_s> &d);
    static size_t sersize(vector<rowOrField_s> &d);
    void des(vector<rowOrField_s> &d);
    void ser(vector<MessageApply::applyindex_s> &d);
    static size_t sersize(vector<MessageApply::applyindex_s> &d);
    void des(vector<MessageApply::applyindex_s> &d);
    void ser(boost::unordered_map< int64_t,
             vector<MessageDispatch::record_s> > &d);
    static size_t sersize(boost::unordered_map< int64_t,
                          vector<MessageDispatch::record_s> > &d);
    void des(boost::unordered_map< int64_t,
             vector<MessageDispatch::record_s> > &d);
};

/** 
 * @brief Message variant carrying serialized Message variant as payload
 *
 * @param dataarg serialized Message variant
 *
 * @return 
 */
class MessageSerialized : public Message
{
public:
    MessageSerialized(string *dataarg);
    virtual ~MessageSerialized();
  
    std::string *data;
};

/** 
 * @brief Message variant containing multiple serialized Message variants
 *
 * @param nodeidarg nodeid
 *
 * @return 
 */
class MessageBatchSerialized : public Message
{
public:
    struct msgbatch_s
    {
        int16_t nodeid;
        std::string *serializedmsg;
    };
  
    MessageBatchSerialized(int16_t nodeidarg);
    virtual ~MessageBatchSerialized();
  
    short nmsgs;
    msgbatch_s msgbatch[OBGWMSGBATCHSIZE];
};

#endif  /* INFINISQLMESSAGE_H */
