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
 * @file   Mbox.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Jan 21 10:58:46 2014
 * 
 * @brief  mailbox consumer, producer, and organization
 *
 * lockfree multi-producer single consumer adapted from:
 * http://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html
 */

#include "Mbox.h"

Mbox::Mbox() : counter(8888)
{
    firstMsg=new (std::nothrow) Message();
    if (firstMsg==NULL)
    {
        return;
    }
    firstMsg->message.payloadtype=Message::PAYLOAD_NONE;
    firstMsg->nextmsg = getInt128(nullptr, 5000);
    currentMsg=firstMsg;
    lastMsg=firstMsg;
    myLastMsg=firstMsg;

    head=getInt128(firstMsg, 10000);
    tail=head;
    current=head;
    mytail=head;    
}

Mbox::~Mbox()
{
    delete firstMsg;
}

class Message *Mbox::receive(int timeout)
{
    __int128 mynext;

    while (1)
    {
#ifdef __clang__
        mynext=0;
#else
        mynext = __atomic_load_n(&(getPtr(current)->nextmsg), __ATOMIC_SEQ_CST);
#endif

        if (getPtr(mynext)==nullptr)
        {
            switch (timeout)
            {
            case -1:
                break;

            case 0:
                return nullptr;
                break;

            default:
                struct timespec ts = {0, timeout * 1000};
                nanosleep(&ts, nullptr);
                return nullptr;
            }
        }
        else
        {
            if (getPtr(current)==getPtr(mynext))
            {
                LOG("anomaly  current " << getCount(current) << " = next "
                    << getCount(mynext));
            }

            delete getPtr(current);
            current = mynext;
            return getPtr(current);
        }
    }
}

__int128 Mbox::getInt128(class Message *ptr, uint64_t count)
{
    __int128 i128;
    memcpy(&i128, &ptr, sizeof(ptr));
    memcpy((uint64_t *)&i128+1, &count, sizeof(count));

    return i128;
}

class Message *Mbox::getPtr(__int128 i128)
{
    return (class Message *)i128;
}

uint64_t Mbox::getCount(__int128 i128)
{
    return *((uint64_t *)&i128+1);
}

void Mbox::sendMsg(Message &msg)
{
#ifdef __clang__
        ;
#else
    msg.nextmsg = Mbox::getInt128(nullptr, 5555);

    __int128 mytail;
    __int128 mynext;

    while (1)
    {
        mytail = __atomic_load_n(&tail, __ATOMIC_SEQ_CST);
        mynext = __atomic_load_n(&(Mbox::getPtr(mytail)->nextmsg),
                                 __ATOMIC_SEQ_CST);

        if (mytail == __atomic_load_n(&tail, __ATOMIC_SEQ_CST))
        {
            if (Mbox::getPtr(mynext) == NULL)
            {
                if (__atomic_compare_exchange_n(&(Mbox::getPtr(mytail)->nextmsg), &mynext, Mbox::getInt128(&msg, __atomic_add_fetch(&counter, 1, __ATOMIC_SEQ_CST)), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
                {
                    break;
                }
            }
            else
            {
                // CAS(&Q->Tail, tail, <next.ptr, tail.count+1>)
                __atomic_compare_exchange_n(&tail, &mytail, Mbox::getInt128(Mbox::getPtr(mynext), __atomic_add_fetch(&counter, 1, __ATOMIC_SEQ_CST)), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
            }
        }
    }

    __atomic_compare_exchange_n(&tail, &mytail, Mbox::getInt128(&msg, __atomic_add_fetch(&counter, 1, __ATOMIC_SEQ_CST)), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#endif // __clang__
}
