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
 * @date   Tue Jan 21 10:54:36 2014
 * 
 * @brief  mailbox consumer, producer, and organization
 *
 * lockfree multi-producer single consumer adapted from:
 * http://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html
 */

#ifndef INFINISQLMBOX_H
#define INFINISQLMBOX_H

#include "global.h"

class Mbox
{
public:
    Mbox();
    ~Mbox();
    
    /**
     * @brief consume a Message
     *
     * return *message, nullptr if timeout or nothing
     * timeout: -1, wait forever (well, uncertain effect), don't use it
     * timeout: 0, do not wait
     * timeout: >0, microseconds to wait
     * 
     * @param timeout timeout in microseconds
     *
     * @return Message object
     */
    Message *receive(int timeout);
    /**
     * @brief create 128bit integer from Message object
     *
     * @param ptr Message variant
     * @param count count component
     *
     * @return 128bit integer
     */
    static __int128 getInt128(Message *inmsg, uint64_t count);
    /**
     * @brief return Message pointer from 128bit integer
     *
     * @param i128 input 128bit integer
     *
     * @return Message
     */
    static Message *getPtr(__int128 i128);
    /**
     * @brief return count component from 128bit integer
     *
     * @param i128 input 128bit integer
     *
     * @return count
     */
    static uint64_t getCount(__int128 i128);

    Message *firstMsg;
    Message *currentMsg;
    Message *lastMsg;
    Message *myLastMsg; // not to be modified by producer
    __int128 head;
    __int128 tail;
    __int128 current;
    __int128 mytail;
    uint64_t counter;
};

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
    MboxProducer(Mbox *mbox, int16_t nodeid);
    /**
     * @brief produce Message onto mailbox
     *
     * @param msgsnd Message
     */
    void sendMsg(Message &msgsnd);
    
    Mbox *mbox;
    int16_t nodeid;
    MessageBatch *obBatchMsg;
    /**
     * @todo uncomment when mboxes is up
     */
     // Mboxes *mboxes;
};

#endif // INFINISQLMBOX_H
