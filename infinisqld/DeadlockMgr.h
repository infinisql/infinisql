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
 * @file   DeadlockMgr.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:07:24 2013
 * 
 * @brief  Actor which resolves deadlocks.
 */

#ifndef INFINISQLDEADLOCKMGR_H
#define INFINISQLDEADLOCKMGR_H

#include "gch.h"

typedef struct
{
    //  int64_t taid;
    Topology::addressStruct addr;
    int64_t pendingcmdid;
} taCmd;

/** 
 * @brief Deadlock manager actor.
 *
 */
class DeadlockMgr
{
public:
    /** 
     * @brief execute Deadlock Manager actor
     *
     * uses a wait-for graph to resolve deadlocks
     *
     * @param myIdentityArg how to identify actor instance
     */
    DeadlockMgr(Topology::partitionAddress *myIdentityArg);
    virtual ~DeadlockMgr();

    friend class Transaction; // uses makeLockedItem

private:
    /** 
     * @brief create item to add to waitfor graph
     *
     * Items are added to wait-for graph under following circumstances:
     * at least 1 row/unique index entry is locked by transaction and at
     * least 1 is
     * pending a lock for that transaction. All locked and pending items
     * are added to wait-for graph as potential causes of a deadlock.
     * the item to add to the wait-for graph is a string (returnstring)
     *
     * @param isrow is a row (otherwise unique index entry)
     * @param rowid rowid
     * @param tableid tableid
     * @param engineid engineid
     * @param domainid domainid
     * @param fieldid fieldid
     * @param floatentry if index entry for float, the float value
     * @param stringentry if index entry for string, string value
     * @param returnstring item to add to wait-for graph
     */
    static void makeLockedItem(bool isrow, int64_t rowid, int64_t tableid,
                               int64_t engineid, int64_t domainid,
                               int64_t fieldid, long double floatentry,
                               std::string *stringentry,
                               std::string *returnstring);
    /** 
     * @brief wait-for graph algoritm
     *
     */
    void algorithm();
    /** 
     * upon deadlock, send message to TransactionAgent to abort transaction
     *
     * @param transactionid 
     */
    void deadlock(int64_t transactionid);
    /** 
     * @brief walk the wait-for graph for items this transactionid is waiting for
     *
     * @param transactionid transactionid
     *
     * @return true for positive response
     */
    bool walk(int64_t transactionid);
    /** 
     * @brief walk the wait-for graph for transactions that hold this item
     *
     * @param itemIt item held by transactions
     *
     * @return true for positive response
     */
    bool walk(boost::unordered_set<string>::iterator itemIt);
    /** 
     * @brief remove transactionid from wait-for graph
     *
     * @param transactionid transactionid
     */
    void removeTransaction(int64_t transactionid);

    class Mbox *mymboxPtr;
    Topology::partitionAddress myIdentity;
    class Mboxes mboxes;
    class Topology myTopology;

    REUSEMESSAGES
        // maps:
        // transactions with the locks it holds
        boost::unordered_map< int64_t, boost::unordered_set<std::string> >
        transactionLocksMap;
    // transactions with what it waits on
    boost::unordered_map< int64_t, boost::unordered_set<std::string> >
        transactionWaitsMap;
    // items whose locks are held by transactions
    boost::unordered_map< std::string, boost::unordered_set<int64_t> >
        locksTransactionMap;
    // items whose locks are waited for by transactions
    boost::unordered_map< std::string, boost::unordered_set<int64_t> >
        waitsTransactionMap;

    boost::unordered_map<int64_t, taCmd> returnMap;

    boost::unordered_set<int64_t> skipTransactionSet;
    boost::unordered_set<std::string> skipItemSet;

    // duplicates here mean deadlock:
    boost::unordered_set<int64_t> transactionGraphSet;

};

/** 
 * @brief launch Engine actor
 *
 * @param identity how to identify this
 *
 * @return 
 */
void *deadlockMgr(void *identity);

#endif  /* INFINISQLDEADLOCKMGR_H */
