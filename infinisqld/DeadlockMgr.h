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
    DeadlockMgr(Topology::partitionAddress *myIdentityArg);
    virtual ~DeadlockMgr();

    friend class Transaction; // uses makeLockedItem

private:
    static void makeLockedItem(bool isrow, int64_t rowid, int64_t tableid,
                               int64_t engineid, int64_t domainid,
                               int64_t fieldid, long double floatentry,
                               std::string *stringentry,
                               std::string *returnstring);
    void algorithm();
    void deadlock(int64_t transactionid);
    bool walk(int64_t transactionid);
    bool walk(boost::unordered_set<string>::iterator itemIt);
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

void *deadlockMgr(void *);

#endif  /* INFINISQLDEADLOCKMGR_H */
