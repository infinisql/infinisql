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

/** 
 * @brief create Subtransaction object
 *
 * @param taAddrarg address of TransactionAgent associated with Transaction
 * @param transactionidarg related transactionid
 * @param domainidarg domainid
 * @param enginePtrarg Engine (partition)
 */
class SubTransaction
{
public:
    SubTransaction(Topology::addressStruct &taAddrarg,
                   int64_t transactionidarg, int64_t domainidarg,
                   class Engine *enginePtrarg);
    virtual ~SubTransaction();

    friend class Engine;

private:
    /** 
     * @brief handle incoming MessageTransaction variant
     *
     * @param msgrcvarg received MessageTransaction variant
     */
    void processTransactionMessage(class Message *msgrcvarg);
    /** 
     * @brief commit, rollback, or unlock items
     *
     * @param rowOrFields list of rows and unique index entries
     * @param cmd commit, rollback or unlock
     */
    void commitRollbackUnlock(vector<rowOrField_s> *rowOrFields,
                              enginecmd_e cmd);
    /** 
     * @brief operate on transactions waiting for locked row
     *
     * @param tableid tableid
     * @param rowid rowid
     */
    void processRowLockQueue(int64_t tableid, int64_t rowid); // keep
    /** 
     * @brief drain queue of transactions waiting for locked row
     *
     * used when row cannot be re-locked, such as when deleted
     *
     * @param tableid tableid
     * @param rowid 
     */
    void drainRowLockQueue(int64_t tableid, int64_t rowid); // keep
    /** 
     * @brief operate on transactions waiting for locked unique index entry
     *
     * @param tableid tableid
     * @param fieldid fieldid
     * @param val field value
     */
    void processIndexLockQueue(int64_t tableid, int64_t fieldid,
                               fieldValue_s *val);
    /** 
     * @brief drain queue of transactions waiting for locked unique index entry
     *
     * when index entry cannot be re-locked
     *
     * @param tableid tableid
     * @param fieldid fieldid
     * @param val field value
     */
    void drainIndexLockQueue(int64_t tableid, int64_t fieldid,
                             fieldValue_s *val);
    /** 
     * @brief create new row
     *
     * @param tableid tableid
     * @param row row string
     *
     * @return 
     */
    int64_t newrow(int64_t tableid, std::string row);
    /** 
     * @brief new unique index entry
     *
     * @param tableid tableid
     * @param fieldid fieldid
     * @param rowid rowid
     * @param engineid engineid
     * @param val field value
     *
     * @return 
     */
    locktype_e uniqueIndex(int64_t tableid, int64_t fieldid, int64_t rowid,
                           int64_t engineid, fieldValue_s *val);
    /** 
     * @field modify row
     *
     * @param tableid tableid
     * @param rowid rowid
     * @param row new row
     *
     * @return 
     */
    int64_t updaterow(int64_t tableid, int64_t rowid, std::string *row);
    /** 
     * @brief delete row
     *
     * @param tableid tableid
     * @param rowid rowid
     *
     * @return 
     */
    int64_t deleterow(int64_t tableid, int64_t rowid);
    /** 
     * @brief delete row part of row replacement (UPDATEing fieldid 0)
     *
     * @param tableid tableid
     * @param rowid rowid
     * @param forward_rowid new rowid
     * @param forward_engineid new engineid
     *
     * @return 
     */
    int64_t deleterow(int64_t tableid, int64_t rowid, int64_t forward_rowid,
                      int64_t forward_engineid);
    void indexSearch(int64_t tableid, int64_t fieldid,
                     searchParams_s *searchParameters,
                     vector<nonLockingIndexEntry_s> *indexHits);
    /** 
     * @brief return rows
     *
     * @param tableid tableid
     * @param rowids list of rowids to return
     * @param locktype lock type
     * @param pendingcmdid pending commad of sending Transaction
     * @param returnRows rows to return
     */
    void selectrows(int64_t tableid, vector<int64_t> *rowids,
                    locktype_e locktype, int64_t pendingcmdid,
                    vector<returnRow_s> *returnRows);
    /** 
     * @brief search index for and return matching rows
     *
     * rows are distributed across partitions by field 0 hash
     * this function performs index lookup and data return in 1 step
     * for selects against field 0
     *
     * @param tableid tableid
     * @param fieldid fieldid
     * @param locktype lock type
     * @param searchParams search parameters
     * @param returnRows rows to return
     */
    void searchReturn1(int64_t tableid, int64_t fieldid, locktype_e locktype,
                       searchParams_s &searchParams,
                       vector<returnRow_s> &returnRows);
    /** 
     * @brief reply to calling TransactionAgent
     *
     * @param data Message variant to send
     */
    void replyTransaction(void *data);
    /** 
     * @brief reply to calling TransactionAgent
     *
     * for draining queue of old transactions. state from old subtransaction
     * is kept in the original received Message variant
     *
     * @param sndRef Message variant to send
     * @param rcvRef original received Message variant
     */
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
