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

#include "infinisql_DeadlockMgr.h"
#line 22 "DeadlockMgr.cc"

DeadlockMgr::DeadlockMgr(Topology::partitionAddress *myIdentityArg) :
    myIdentity(*myIdentityArg)
{
    delete myIdentityArg;
    mboxes.nodeid = myIdentity.address.nodeid;
    mboxes.update(myTopology);

    class Message *msgrcv=NULL;
    short waitval = 1000;

    while (1)
    {
        mboxes.sendObBatch();
        for (short n=0; n < MSGRECEIVEBATCHSIZE; n ++)
        {
            GETMSG(msgrcv, myIdentity.mbox, waitval)

                if (msgrcv==NULL)
                {
                    waitval=1000;
                    break;
                }

            class MessageDeadlock &msgrcvref =
                *((class MessageDeadlock *)msgrcv);

            switch (msgrcv->messageStruct.topic)
            {
            case TOPIC_DEADLOCKNEW:
            {
                newDeadLockLists_s &listsRef = msgrcvref.nodes;
                transactionLocksMap[msgrcvref.deadlockStruct.transactionid] = listsRef.locked;
                transactionWaitsMap[msgrcvref.deadlockStruct.transactionid] = listsRef.waiting;

                taCmd returninfo;
                returninfo.addr = msgrcvref.messageStruct.sourceAddr;
                //          returninfo.taid = msgrcvref.tainstance;
                returninfo.pendingcmdid = msgrcvref.deadlockStruct.transaction_pendingcmdid;
                returnMap[msgrcvref.deadlockStruct.transactionid] = returninfo;

                boost::unordered_set<string>::iterator it;

                for (it = listsRef.locked.begin();
                     it != listsRef.locked.end(); ++it)
                {
                    locksTransactionMap[*it].insert(msgrcvref.deadlockStruct.transactionid);
                }

                for (it = listsRef.waiting.begin();
                     it != listsRef.waiting.end(); ++it)
                {
                    waitsTransactionMap[*it].insert(msgrcvref.deadlockStruct.transactionid);
                }
            }
            break;

            case TOPIC_DEADLOCKCHANGE:
            {
                string &changedeadlockRef = msgrcvref.deadlockNode;

                switch (msgrcvref.deadlockStruct.deadlockchange)
                {
                case ADDLOCKEDENTRY:
                    if (transactionLocksMap.count(msgrcvref.deadlockStruct.transactionid))
                    {
                        transactionLocksMap[msgrcvref.deadlockStruct.transactionid].
                            insert(changedeadlockRef);
                        locksTransactionMap[changedeadlockRef].
                            insert(msgrcvref.deadlockStruct.transactionid);
                    }

                    break;

                case ADDLOCKPENDINGENTRY:
                    if (transactionWaitsMap.count(msgrcvref.deadlockStruct.transactionid))
                    {
                        transactionWaitsMap[msgrcvref.deadlockStruct.transactionid].
                            insert(changedeadlockRef);
                        waitsTransactionMap[changedeadlockRef].
                            insert(msgrcvref.deadlockStruct.transactionid);
                    }

                    break;

                case REMOVELOCKEDENTRY:
                    if (transactionLocksMap.count(msgrcvref.deadlockStruct.transactionid))
                    {
                        transactionLocksMap[msgrcvref.deadlockStruct.transactionid].
                            erase(changedeadlockRef);
                        locksTransactionMap[changedeadlockRef].
                            erase(msgrcvref.deadlockStruct.transactionid);
                    }

                    break;

                case REMOVELOCKPENDINGENTRY:
                    if (transactionWaitsMap.count(msgrcvref.deadlockStruct.transactionid))
                    {
                        transactionWaitsMap[msgrcvref.deadlockStruct.transactionid].
                            erase(changedeadlockRef);
                        waitsTransactionMap[changedeadlockRef].
                            erase(msgrcvref.deadlockStruct.transactionid);
                    }

                    break;

                case TRANSITIONPENDINGTOLOCKEDENTRY:
                    if (transactionWaitsMap.count(msgrcvref.deadlockStruct.transactionid))
                    {
                        transactionWaitsMap[msgrcvref.deadlockStruct.transactionid].
                            erase(changedeadlockRef);
                        transactionLocksMap[msgrcvref.deadlockStruct.transactionid].
                            insert(changedeadlockRef);

                        waitsTransactionMap[changedeadlockRef].
                            erase(msgrcvref.deadlockStruct.transactionid);
                        locksTransactionMap[changedeadlockRef].
                            insert(msgrcvref.deadlockStruct.transactionid);
                    }

                    break;

                default:
                    fprintf(logfile, "anomaly: %li %s %i\n", 
                            msgrcvref.deadlockStruct.deadlockchange, __FILE__,
                            __LINE__);
                }
            }
            break;

            case TOPIC_DEADLOCKREMOVE:
                removeTransaction(msgrcvref.deadlockStruct.transactionid);
                break;

            case TOPIC_TOPOLOGY:
                mboxes.update(myTopology);
                break;

            default:
                fprintf(logfile, "anomaly: %i %s %i\n",
                        msgrcv->messageStruct.topic, __FILE__, __LINE__);
            }
        }

        // do algorithm
        // set the waitval to -1 so next msg_receive() will block indefinitely
        // algorithm searches for all existing deadlocks, so there's no
        // need to run it until the maps change
        // also maybe swap maps with blank to shrink memory if the whole thing has been searched
        algorithm();
        waitval = 1000;
    }
}

DeadlockMgr::~DeadlockMgr()
{
}

// launcher
void *deadlockMgr(void *identity)
{
    new DeadlockMgr((Topology::partitionAddress *)identity);
    while (1)
    {
        sleep(10);
    }
    return NULL;
}

void DeadlockMgr::makeLockedItem(bool isrow, int64_t rowid, int64_t tableid,
                                 int64_t engineid, int64_t domainid, int64_t fieldid,
                                 long double floatentry, string *stringentry, string *returnstring)
{
    string &returnstringRef = *returnstring;
    size_t strlength = stringentry->length();
    returnstringRef.reserve(sizeof(isrow) + (5 * sizeof(int64_t)) +
                            sizeof(floatentry) + sizeof(strlength) + strlength);

    // this is so identical entries but with garbage in irrelevant values don't
    // show up as different
    if (isrow==true)
    {
        fieldid=-1;
        floatentry=0;
        stringentry->clear();
    }
    else
    {
        rowid=-1;
        engineid=-1;
    }

    size_t pos = 0;
    memcpy(&returnstringRef[pos], &isrow, sizeof(isrow));
    pos += sizeof(isrow);
    memcpy(&returnstringRef[pos], &rowid, sizeof(rowid));
    pos += sizeof(rowid);
    memcpy(&returnstringRef[pos], &tableid, sizeof(tableid));
    pos += sizeof(tableid);
    memcpy(&returnstringRef[pos], &engineid, sizeof(engineid));
    pos += sizeof(engineid);
    memcpy(&returnstringRef[pos], &domainid, sizeof(domainid));
    pos += sizeof(domainid);
    memcpy(&returnstringRef[pos], &fieldid, sizeof(fieldid));
    pos += sizeof(fieldid);
    memcpy(&returnstringRef[pos], &floatentry, sizeof(floatentry));
    pos += sizeof(floatentry);
    memcpy(&returnstringRef[pos], &strlength, sizeof(strlength));
    pos += sizeof(strlength);
    memcpy(&returnstringRef[pos], stringentry->c_str(), strlength);
}

void DeadlockMgr::algorithm(void)
{
    bool deadlockflag = true;

    // remove all deadlocks from existing data set
    while (deadlockflag==true)
    {
        deadlockflag = false;

        boost::unordered_map< int64_t, boost::unordered_set<string> >::iterator it;

        for (it = transactionWaitsMap.begin();
             it != transactionWaitsMap.end(); ++it)
        {

            if (walk(it->first)==true) // means a deadlock was found
            {
                deadlockflag=true;
            }

            skipTransactionSet.clear();
            skipItemSet.clear();
            transactionGraphSet.clear();
        }
    }
}

void DeadlockMgr::deadlock(int64_t transactionid)
{
    if (!returnMap.count(transactionid))
    {
        return;
    }

    //  Mbox::msgstruct msgsnd = {};
    class MessageDeadlock *msg = new class MessageDeadlock;
    class MessageDeadlock &msgref = *msg;
    //  msgsnd.data = msg;
    msgref.messageStruct.topic = TOPIC_DEADLOCKABORT;
    msgref.deadlockStruct.transactionid = transactionid;
    msgref.deadlockStruct.transaction_pendingcmdid = returnMap[transactionid].pendingcmdid;

    mboxes.toActor(myIdentity.address, returnMap[transactionid].addr, *msg);
    removeTransaction(transactionid);
}

// for walk, return true for deadlock, false for no deadlock
// walk through items that this transaction is waiting for
bool DeadlockMgr::walk(int64_t transactionid)
{
    if (transactionGraphSet.count(transactionid))
    {
        deadlock(transactionid);
        return true;
    }

    if (skipTransactionSet.count(transactionid))
    {
        return false;
    }

    skipTransactionSet.insert(transactionid);

    // check for items this transaction is waiting on (though this should
    // always be positive, or the transactionid should be removed by other means)
    if (!transactionWaitsMap.count(transactionid))   // end of path
    {
        return false;
    }

    transactionGraphSet.insert(transactionid);

    boost::unordered_set<string>::iterator it;

    for (it = transactionWaitsMap[transactionid].begin();
         it != transactionWaitsMap[transactionid].end(); ++it)
    {
        if (walk(it)==true)   // deadlock happened at some point!
        {
            return true;
        }
    }

    return false;
}

// walk through transactions that hold this item
bool DeadlockMgr::walk(boost::unordered_set<string>::iterator itemIt)
{
    const string &itemRef = (string)(*itemIt);

    if (skipItemSet.count(itemRef))
    {
        return false;
    }

    skipItemSet.insert(itemRef);

    // check for transactions locking this item
    if (!locksTransactionMap.count(itemRef))   // positively no deadlock
    {
        return false;
    }

    boost::unordered_set<int64_t>::iterator it;

    for (it = locksTransactionMap[itemRef].begin();
         it != locksTransactionMap[itemRef].end(); ++it)
    {
        if (walk(*it)==true)   // deadlock happened!
        {
            return true;
        }
    }

    return false;
}

void DeadlockMgr::removeTransaction(int64_t transactionid)
{
    // walk through all the locks that the transaction holds
    // and those which it waits on, and remove the transactionid
    // entry from those items
    boost::unordered_set<string>::iterator it;

    for (it = transactionLocksMap[transactionid].begin();
         it != transactionLocksMap[transactionid].end();
         ++it)
    {
        locksTransactionMap[*it].erase(transactionid);

        if (locksTransactionMap[*it].empty()==true)
        {
            locksTransactionMap.erase(*it);
        }
    }

    for (it = transactionWaitsMap[transactionid].begin();
         it != transactionWaitsMap[transactionid].end();
         ++it)
    {
        waitsTransactionMap[*it].erase(transactionid);

        if (waitsTransactionMap[*it].empty()==true)
        {
            locksTransactionMap.erase(*it);
        }
    }

    transactionLocksMap.erase(transactionid);
    transactionWaitsMap.erase(transactionid);
    returnMap.erase(transactionid);
}
