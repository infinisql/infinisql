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
 * @file   SubTransaction.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:51:31 2013
 * 
 * @brief  Engine's class corresponding to TransactionAgent's Transaction.
 * Each data manipulation activity has a SubTransaction associated with the
 * Transaction which requested the activity.
 */

#ifndef INFINISQLSUBTRANSACTION_H
#define INFINISQLSUBTRANSACTION_H

#include "gch.h"
#include "Engine.h"

// both SubTransaction and Transaction use this, for different purposes, so
// not all fields have meaning for both classes.

class SubTransaction
{
public:
    //  SubTransaction(int64_t, int64_t, int64_t, class Engine *);
    SubTransaction(Topology::addressStruct &taAddrarg,
                   int64_t transactionidarg, int64_t domainidarg,
                   class Engine *enginePtrarg);
    virtual ~SubTransaction();

    friend class Engine;

private:
    void processTransactionMessage(class Message *msgrcvarg);
    void commitRollbackUnlock(vector<rowOrField_s> *rowOrFields,
                              enginecmd_e cmd);
    void processRowLockQueue(int64_t tableid, int64_t rowid); // keep
    void drainRowLockQueue(int64_t tableid, int64_t rowid); // keep
    void processIndexLockQueue(int64_t tableid, int64_t fieldid,
                               fieldValue_s *val);
    void drainIndexLockQueue(int64_t tableid, int64_t fieldid,
                             fieldValue_s *val);
    int64_t newrow(int64_t tableid, std::string row);
    locktype_e uniqueIndex(int64_t tableid, int64_t fieldid, int64_t rowid,
                           int64_t engineid, fieldValue_s *val);
    int64_t updaterow(int64_t tableid, int64_t rowid, std::string *row);
    int64_t deleterow(int64_t tableid, int64_t rowid);
    int64_t deleterow(int64_t tableid, int64_t rowid, int64_t forward_rowid,
                      int64_t forward_engineid);
    void indexSearch(int64_t tableid, int64_t fieldid,
                     searchParams_s *searchParameters,
                     vector<nonLockingIndexEntry_s> *indexHits);
    void selectrows(int64_t tableid, vector<int64_t> *rowids,
                    locktype_e locktype, int64_t pendingcmdid,
                    vector<returnRow_s> *returnRows);
    void searchReturn1(int64_t tableid, int64_t fieldid, locktype_e locktype,
                       searchParams_s &searchParams,
                       vector<returnRow_s> &returnRows);
    void replyTransaction(void *data);
    void replyTransaction(class MessageTransaction &sndRef,
                          class MessageTransaction &rcvRef);

    //private:
    int64_t subtransactionid;
    Topology::addressStruct taAddr;
    int64_t transactionid;
    int64_t domainid;
    class Message *msgrcv;
    class Engine *enginePtr;
    class Schema *schemaPtr;
};

#endif  /* INFINISQLSUBTRANSACTION_H */
