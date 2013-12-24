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
 * @file   Mbox.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:32:29 2013
 * 
 * @brief  Mbox* classes perform inter-actor messaging. Mbox is a lockfree
 * multi producer, single consumer queue.
 */

#ifndef INFINISQLMBOX_H
#define INFINISQLMBOX_H

#include "Message.h"
#include "Topology.h"

using std::vector;
using std::string;

/** 
 * @brief create mailbox
 *
 * this is a queue, lockfree multi-producer single consumer
 * adapted from:
 * http://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html
 *
 */
class Mbox
{
public:
    Mbox();
    virtual ~Mbox();

    /** 
     * @brief consume a Message
     *
     * @param timeout timeout in microseconds
     *
     * @return Message object
     */
    class Message *receive(int timeout);

    /** 
     * @brief create 128bit integer from Message object
     *
     * @param ptr Message variant
     * @param count count component
     *
     * @return 
     */
    static __int128 getInt128FromPointer(class Message *ptr, uint64_t count);
    /** 
     * @brief return Message pointer from 128bit integer
     *
     * @param i128 input 128bit integer
     *
     * @return Message
     */
    static class Message *getPtr(__int128 i128);
    /** 
     * @brief return count component from 128bit integer
     *
     * @param i128 input 128bit integer
     *
     * @return count
     */
    static uint64_t getCount(__int128 i128);

    friend class MboxProducer;

private:
    pthread_mutex_t mutexLast;

    class Message *firstMsg;
    class Message *currentMsg;
    class Message *lastMsg;
    class Message *myLastMsg; // not to be modified by producer

    __int128 head;
    __int128 tail;
    __int128 current;
    __int128 mytail;

    uint64_t counter;
};

/** 
 * @brief mailbox producer
 */
class MboxProducer
{
public:
    MboxProducer();
    /** 
     * @brief create mailbox producer
     *
     * @param mboxarg Mbox to put Message object onto
     * @param nodeidarg nodeid
     */
    MboxProducer(class Mbox *mboxarg, int16_t nodeidarg);
    virtual ~MboxProducer();
    /** 
     * @brief produce Message onto mailbox
     *
     * @param msgsnd Message
     */
    void sendMsg(class Message &msgsnd);

    class Mbox *mbox;
    int16_t nodeid;
    class MessageBatchSerialized *obBatchMsg;
    class Mboxes *mboxes;

    friend class Mboxes;
};

/** This class lets each actor keep track of destinations in a consistent way */
/** 
 * @brief collection of producers
 */
class Mboxes
{
public:
    /** 
     * @brief address and MboxProducer for an actor
     *
     */
    struct location_s
    {
        Topology::addressStruct address;
        class MboxProducer *destmbox;
    };

    Mboxes();
    /** 
     * @brief collection of producers
     *
     * @param nodeidarg nodeid
     */
    Mboxes(int64_t nodeidarg);
    virtual ~Mboxes();

    /** 
     * @brief update Topology of producers
     *
     * @param top node's authoritative Topology
     */
    void update(class Topology &top);
    /** 
     * @brief update Topology of producers
     *
     * @param top node's authoritative Topology
     * @param myActorid actorid for owning actor
     */
    void update(class Topology &top, int64_t myActorid);
    /** 
     * @brief produce Message for specific actor
     *
     * @param source source address
     * @param dest destination address
     * @param msg Message
     */
    void toActor(const Topology::addressStruct &source,
                 const Topology::addressStruct &dest, class Message &msg);
    /** 
     * @brief produce Message for replica's UserSchemaMgr
     *
     * @param source source address
     * @param msg Message
     */
    void toUserSchemaMgr(const Topology::addressStruct &source,
                         class Message &msg);
    /** 
     * @brief produce Message for replica's DeadlockMgr
     *
     * @param source source address
     * @param msg Message
     */
    void toDeadlockMgr(const Topology::addressStruct &source,
                       class Message &msg);
    /** 
     * @brief produce Message for Engine associated with specific partition
     *
     * @param source source address
     * @param partitionid destination partition
     * @param msg Message
     */
    void toPartition(const Topology::addressStruct &source, int64_t partitionid,
                     class Message &msg);
    /** 
     * @bried produce Message destined to all actors of type
     *
     * @param type destination actor type
     * @param source source address
     * @param msg Message
     *
     * @return 
     */
    int64_t toAllOfType(actortypes_e type,
                        const Topology::addressStruct &source,
                        class Message &msg);
    /** 
     * @brief produce Message destined to all actors of type on current replica
     *
     * @param type destination actor type
     * @param source source address
     * @param msg Mesage
     *
     * @return 
     */
    int64_t toAllOfTypeThisReplica(actortypes_e type,
                                   const Topology::addressStruct &source,
                                   class Message &msg);
    /** 
     * @brief send batched Message objects destined to remote nodes
     */
    void sendObBatch();

    int64_t nodeid;

    class MboxProducer topologyMgr;
    class MboxProducer userSchemaMgr;
    class MboxProducer deadlockMgr;
    class MboxProducer listener;
    class MboxProducer obGateway;
    class MboxProducer ibGateway;
    std::vector<class MboxProducer> transactionAgents;
    std::vector<class MboxProducer> engines;

    // new
    std::vector<class MboxProducer *> actoridToProducers;
    std::vector<class MboxProducer *> transactionAgentPtrs;
    std::vector<class MboxProducer *> enginePtrs;
    class MboxProducer *topologyMgrPtr;
    location_s userSchemaMgrLocation;
    location_s deadlockMgrLocation;
    class MboxProducer *listenerPtr;
    class MboxProducer *obGatewayPtr;

    std::vector<location_s> partitionToProducers;
    // allActors[nodeid][actorid] = actortype
    std::vector< vector<int> > allActors;
    boost::unordered_map< int16_t, std::vector<int> > allActorsThisReplica;
};

// put this in each actor's class definition (except ObGw)
#define REUSEMESSAGES class Message reuseMessage;                   \
    class MessageSocket reuseMessageSocket;                         \
    class MessageUserSchema reuseMessageUserSchema;                 \
    class MessageDeadlock reuseMessageDeadlock;                     \
    class MessageSubtransactionCmd reuseMessageSubtransactionCmd;   \
    class MessageCommitRollback reuseMessageCommitRollback;         \
    class MessageDispatch reuseMessageDispatch;                     \
    class MessageAckDispatch reuseMessageAckDispatch;               \
    class MessageApply reuseMessageApply;                           \
    class MessageAckApply reuseMessageAckApply;

// do this instead of mymbox.receive (except ObGw)
#define GETMSG(X, Y, Z)                                                 \
    X=Y->receive(Z);                                                    \
    if (X != NULL && X->messageStruct.topic==TOPIC_SERIALIZED)          \
    {                                                                   \
     SerializedMessage serobj(((class MessageSerialized *)X)->data);    \
     switch (serobj.getpayloadtype())                                   \
     {                                                                  \
case PAYLOADMESSAGE:                                                    \
{                                                                       \
 reuseMessage=Message();                                                \
 reuseMessage.unpack(serobj);                                           \
 if (serobj.data->size() != serobj.pos)                                 \
 {                                                                      \
  fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
  }                                                                     \
 X=&reuseMessage;                                                       \
 }                                                                      \
break;                                                                  \
case PAYLOADSOCKET:                                                     \
{                                                                       \
 reuseMessageSocket=MessageSocket();                                    \
 reuseMessageSocket.unpack(serobj);                                     \
 if (serobj.data->size() != serobj.pos)                                 \
 {                                                                      \
  fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
  }                                                                     \
 X=&reuseMessageSocket;                                                 \
 }                                                                      \
break;                                                                  \
case PAYLOADUSERSCHEMA:                                                 \
{                                                                       \
 reuseMessageUserSchema=MessageUserSchema();                            \
 reuseMessageUserSchema.unpack(serobj);                                 \
 if (serobj.data->size() != serobj.pos)                                 \
 {                                                                      \
  fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
  }                                                                     \
 X=&reuseMessageUserSchema;                                             \
 }                                                                      \
break;                                                                  \
case PAYLOADDEADLOCK:                                                   \
{                                                                       \
 reuseMessageDeadlock=MessageDeadlock();                                \
 reuseMessageDeadlock.unpack(serobj);                                   \
 if (serobj.data->size() != serobj.pos)                                 \
 {                                                                      \
  fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
  }                                                                     \
 X=&reuseMessageDeadlock;                                               \
 }                                                                      \
break;                                                                  \
case PAYLOADSUBTRANSACTION:                                             \
{                                                                       \
 reuseMessageSubtransactionCmd=MessageSubtransactionCmd();              \
 reuseMessageSubtransactionCmd.unpack(serobj);                          \
 if (serobj.data->size() != serobj.pos)                                 \
 {                                                                      \
  fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
  }                                                                     \
 X=&reuseMessageSubtransactionCmd;                                      \
 }                                                                      \
break;                                                                  \
case PAYLOADCOMMITROLLBACK:                                             \
{                                                                       \
 reuseMessageCommitRollback=MessageCommitRollback();                    \
 reuseMessageCommitRollback.unpack(serobj);                             \
 if (serobj.data->size() != serobj.pos)                                 \
 {                                                                      \
  fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
  }                                                                     \
 X=&reuseMessageCommitRollback;                                         \
 }                                                                      \
break;                                                                  \
case PAYLOADDISPATCH:                                                   \
{                                                                       \
 reuseMessageDispatch=MessageDispatch();                                \
 reuseMessageDispatch.unpack(serobj);                                   \
 if (serobj.data->size() != serobj.pos)                                 \
 {                                                                      \
  fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
  }                                                                     \
 X=&reuseMessageDispatch;                                               \
 }                                                                      \
break;                                                                  \
case PAYLOADACKDISPATCH:                                                \
{                                                                       \
 reuseMessageAckDispatch=MessageAckDispatch();                          \
 reuseMessageAckDispatch.unpack(serobj);                                \
 if (serobj.data->size() != serobj.pos)                                 \
 {                                                                      \
  fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
  }                                                                     \
 X=&reuseMessageAckDispatch;                                            \
 }                                                                      \
break;                                                                  \
case PAYLOADAPPLY:                                                      \
{                                                                       \
 reuseMessageApply=MessageApply();                                      \
 reuseMessageApply.unpack(serobj);                                      \
 if (serobj.data->size() != serobj.pos)                                 \
 {                                                                      \
  fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
  }                                                                     \
 X=&reuseMessageApply;                                                  \
 }                                                                      \
break;                                                                  \
case PAYLOADACKAPPLY:                                                   \
{                                                                       \
 reuseMessageAckApply=MessageAckApply();                                \
 reuseMessageAckApply.unpack(serobj);                                   \
 if (serobj.data->size() != serobj.pos)                                 \
 {                                                                      \
  fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
  }                                                                     \
 X=&reuseMessageAckApply;                                               \
 }                                                                      \
break;                                                                  \
default:                                                                \
printf("%s %i anomaly %i\n", __FILE__, __LINE__, serobj.getpayloadtype()); \
}                                                                       \
     delete serobj.data;                                                \
     }

#endif  /* INFINISQLMBOX_H */
