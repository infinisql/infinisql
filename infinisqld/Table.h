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
 * @file   Table.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:52:59 2013
 * 
 * @brief  Table class. Has Fields and Indices.
 */

#ifndef INFINISQLTABLE_H
#define INFINISQLTABLE_H

#include "gch.h"
#include "Field.h"

/** 
 * @brief for replaced rows, how to find the new row
 */
typedef struct
{
    int64_t rowid;
    int64_t engineid;
} forwarderEntry;

/**
 * @brief row and its meta-data
 *
 * this per-row to say which transactionid created/updated transaction last,
 * for replication so that the corresponding engine(s) will know the order
 * to apply changes. also says whether the row is locked by somebody
 */
typedef struct
{
    int64_t writelockHolder; // this is also the subtransactionid
    int64_t previoussubtransactionid;
    char flags; // commit & rollback set this to 0
    boost::unordered_set<int64_t> *readlockHolders;
    string row;
} rowdata_s;

/** 
 * @brief data for transactions waiting to lock a row
 */
typedef struct
{
    int64_t pendingcmdid;
    int64_t tacmdentrypoint;
    int64_t subtransactionid;
    locktype_e locktype;
} lockQueueRowEntry;

/** 
 * @brief create new Table object
 *
 * @param idarg tableid
 */
class Table
{
public:
    Table(int64_t idarg);
    virtual ~Table()
    {
        if (id)
        {
            delete shadowTable;
        }
    }

    friend class ApiInterface;
    friend class TransactionAgent;
    friend class Engine;
    friend class Transaction;
    friend class SubTransaction;
    friend class UserSchemaMgr;

    //private:
    /** 
     * @brief set name
     *
     * @param namearg name
     */
    void setname(string namearg);
    /** 
     * @brief get name
     *
     *
     * @return name
     */
    std::string *getname();
    /** 
     * @brief add field/column
     *
     * @param type field type
     * @param length length (for CHARX)
     * @param name field name
     * @param indextype index type
     *
     * @return fieldid
     */
    int64_t addfield(fieldtype_e type, int64_t length, std::string name,
                     indextype_e indextype);
    /** 
     * @brief assemble row string from fields
     *
     * @param fieldVal fields
     * @param res resulting string
     *
     * @return success or failure
     */
    bool makerow(vector<fieldValue_s> *fieldVal, std::string *res);
    /** 
     * @brief extract fields from row string
     *
     * @param rowstring input row
     * @param resultFields resulting fields
     *
     * @return success or failure
     */
    bool unmakerow(std::string *rowstring,
                   vector<fieldValue_s> *resultFields);
    // for fetch (cursor)
    /** 
     * @brief orphan?
     *
     * @param rowids 
     * @param locktype 
     * @param subtransactionid 
     * @param pendingcmdid 
     * @param returnRows 
     * @param lockPendingRowids 
     * @param tacmdentrypoint 
     */
    void getrows(vector<int64_t> rowids, locktype_e locktype,
                 int64_t subtransactionid, int64_t pendingcmdid,
                 vector<returnRow_s> *returnRows,
                 vector<int64_t> *lockPendingRowids, int64_t tacmdentrypoint);
    // stage. return rowid. take command (insert|update|delete),
    // subtransactionid,
    // row, rowid
    //  int64_t stage(pendingprimitive_e, int64_t, string *, int64_t);
    // commit, i guess just the rowid is enough to find it
    //  void commit(int64_t, int64_t);
    // rollback
    //  void rollback(int64_t, int64_t);
    /** 
     * @brief generate unique, ever-increasing row identifier
     *
     *
     * @return next rowid
     */
    int64_t getnextrowid();
    /** 
     * @brief create new row
     *
     * @param newrowid rowid
     * @param subtransactionid subtransactionid
     * @param row row
     */
    void newrow(int64_t newrowid, int64_t subtransactionid, string &row);
    /** 
     * @brief modify row
     *
     * @param rowid rowid
     * @param subtransactionid subtransactionid
     * @param row new row
     *
     * @return status
     */
    int64_t updaterow(int64_t rowid, int64_t subtransactionid, string *row);
    /** 
     * @brief delete row
     *
     * @param rowid rowid
     * @param subtransactionid subtransactionid
     *
     * @return 
     */
    int64_t deleterow(int64_t rowid, int64_t subtransactionid);
    /** 
     * @brief delete row part of replacement
     *
     * @param rowid rowid
     * @param subtransactionid subtransactionid
     * @param forward_rowid rowid to forward index hits to
     * @param forward_engineid engineid to forward index hits to
     *
     * @return 
     */
    int64_t deleterow(int64_t rowid, int64_t subtransactionid,
                      int64_t forward_rowid, int64_t forward_engineid);
    /** 
     * @brief return rows for select based on index hits
     *
     * @param rowids list of rowids
     * @param locktype lock type
     * @param subtransactionid subtransactionid
     * @param pendingcmdid pending command by calling Transaction
     * @param returnRows return rows
     * @param tacmdentrypoint entry point back to pending Transaction command
     */
    void selectrows(vector<int64_t> *rowids, locktype_e locktype,
                    int64_t subtransactionid, int64_t pendingcmdid,
                    vector<returnRow_s> *returnRows, int64_t tacmdentrypoint);
    /** 
     * @brief add subtransactionid to queue waiting to acquire lock on row
     *
     * @param rowid rowid
     * @param locktype lock type
     * @param subtransactionid subtransactionid
     * @param pendingcmdid pending command by calling Transaction
     * @param tacmdentrypoint entry point back to pending Transaction command
     *
     * @return 
     */
    locktype_e assignToLockQueue(int64_t rowid, locktype_e locktype,
                                 int64_t subtransactionid,
                                 int64_t pendingcmdid,
                                 int64_t tacmdentrypoint);
    void commitRollbackUnlock(int64_t rowid, int64_t subtransactionid,
                              enginecmd_e cmd);

    //private:
    int64_t id;
    std::string name;
    int64_t nextindexid;
    std::vector<class Field> fields;
    boost::unordered_map< int64_t, std::queue<lockQueueRowEntry> > lockQueue;
    class Table *shadowTable;
    boost::unordered_map<std::string, int64_t> columnaNameToFieldMap;
    //  std::unordered_map<int64_t, rowdata_s *> rows; // this is the actual data
    boost::unordered_map<int64_t, rowdata_s *> rows; // this is the actual data
    //  boost::unordered_map<int64_t, rowdata_s> rows; // this is the actual data
    int64_t rowsize;
    int64_t nextrowid; // do not mess with this directly
    // this is for the delete component of a replacement
    boost::unordered_map<int64_t, forwarderEntry> forwarderMap;
};

#endif  /* INFINISQLTABLE_H */
