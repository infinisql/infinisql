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

#ifndef INFINISQLTRANSACTION_H
#define INFINISQLTRANSACTION_H

#include "infinisql_gch.h"
#include "infinisql_TransactionAgent.h"
#include "infinisql_DeadlockMgr.h"

class TransactionAgent;

size_t hash_value(uuRecord_s const &);
bool operator==(uuRecord_s const &, uuRecord_s const &);

class ApiInterface;
typedef void(ApiInterface::*apifPtr)(int64_t, void *);

typedef void(Statement::*statementfPtr)(int64_t, void *);

typedef struct
{
    bool isaddunique; // tells commit & rollback whether this was pre-staged by
    // insert,update,replace
    locktype_e locktype;
    int64_t engineid;
    int64_t tableid;
    int64_t fieldid;
    fieldValue_s fieldVal;
} indexInfo_s;

class Transaction
{
public:
    struct sqlcmdstate_s
    {
        class Statement *statement;
        boost::unordered_map<uuRecord_s, returnRow_s> *results;
        int64_t tableid;
        locktype_e locktype;
        int64_t eventwaitcount;
        vector<indexEntry_s> indexHits;
        void *continuationData;
        bool ispossibledeadlock;
    };

    typedef struct
    {
        int64_t tableid;
        class Table *tablePtr;
        vector<indexInfo_s> indexEntries;
        int64_t rowEngineid; // partition for the main field of the table where the
        // row lives
        int64_t rowid;
        int64_t engineid; // for index value, not message destination!
        locktype_e locktype;
        string row;
        int64_t enginesWithUniqueIndices;
        int64_t engines;
        int64_t fieldid;
        bool isunique; // for update 1 field
        int64_t destinationengineid;
        string *rowPtr;

        vector<indexEntry_s> rowidsEngineids;
        fieldValue_s oldFieldValue;
        boost::unordered_map< int64_t, class MessageCommitRollback *>
            replaceEngineMsgs; // for select
        bool ispossibledeadlock;
        boost::unordered_map< uuRecord_s, stagedRow_s > pendingStagedRows;
        uuRecord_s originaluur;
        uuRecord_s newuur;
        bool isupdatemultiplefields;
        fieldValue_s fieldVal;
        string newRow;
        vector<fieldValue_s> originalFieldValues;
        vector<fieldValue_s> newFieldValues;
        int64_t uniqueindices;
    } cmdState_s;

    enum transactionstate_e
    {
        EXPANDING,
        DISPATCHING,
        COMMITTING,
        COMMITTED,
        ABORTING,
        ABORTED
    };

    Transaction(class TransactionAgent *, int64_t);
    virtual ~Transaction();

    // keep all/mostly public for stored procedures
    //private:
    void zeroCurrentCmdState(void);
    void processTransactionMessage(class Message *); // keep
    int64_t getEngineid(class Table *, int64_t);
    int64_t getEngineid(class Table *, int64_t, fieldValue_s *);
    int64_t getEngineid(int64_t);
    int64_t getEngineid(uint64_t);
    int64_t getEngineid(bool);
    int64_t getEngineid(long double);
    int64_t getEngineid(char);
    int64_t getEngineid(string *);
    int64_t getengine(fieldtype_e, fieldValue_s &);
    void dispatch(class Message *);
    void dispatched(class Message *);
    void checkLock(deadlockchange_e, bool, int64_t, int64_t, int64_t, int64_t,
                   fieldValue_s *);
    void badMessageHandler(void);
    // for ApiInterface::insert()
    void addFieldToRow(void);
    void addFieldToRow(int64_t);
    void addFieldToRow(uint64_t);
    void addFieldToRow(bool);
    void addFieldToRow(long double);
    void addFieldToRow(char);
    void addFieldToRow(string &);
    void reenter(void);
    // returns true if field passes null constraint check, false if fails
    // constraint check
    bool checkNullConstraintOK(int64_t);
    void makeFieldValue(fieldValue_s *, bool, int64_t);
    void makeFieldValue(fieldValue_s *, bool, uint64_t);
    void makeFieldValue(fieldValue_s *, bool, bool);
    void makeFieldValue(fieldValue_s *, bool, long double);
    void makeFieldValue(fieldValue_s *, bool, char);
    void makeFieldValue(fieldValue_s *, bool, string);
    void reenter(int64_t);
    void reenter(int64_t, class Transaction *);
    void replace(void);
    void select(int64_t, int64_t, locktype_e, searchParams_s *);
    void continueInsertRow(int64_t);
    void continueUpdateRow(int64_t);
    void continueDeleteRow(int64_t);
    void continueReplaceRow(int64_t);
    void continueSelectRows(int64_t);
    void continueFetchRows(int64_t);
    void continueUnlockRow(int64_t);
    void continueCommitTransaction(int64_t);
    void continueRollbackTransaction(int64_t);
    void rollback();
    void revertback(uuRecord_s &uur, enginecmd_e);
    void abortCmd(int);
    void sendTransaction(enginecmd_e, payloadtype_e, int64_t, int64_t , void *);
    void deadlockAbort(class MessageDeadlock &);
    void updateRow(void);
    void rollback(uuRecord_s &);
    void revert(uuRecord_s &);
    void addRof(int64_t, rowOrField_s &,
                boost::unordered_map< int64_t, class MessageCommitRollback *> &);
    class MessageDispatch *makeMessageDispatch();

    void sqlPredicate(class Statement *, operatortypes_e, int64_t, string &,
                      string &, locktype_e, vector<fieldValue_s> &, void *,
                      boost::unordered_map<uuRecord_s, returnRow_s> &);
    void continueSqlPredicate(int64_t);
    void sqlSelectAll(class Statement *, int64_t, locktype_e, pendingprimitive_e,
                      boost::unordered_map<uuRecord_s, returnRow_s> &);
    void continueSqlDelete(int64_t);
    void continueSqlInsert(int64_t);
    void continueSqlUpdate(int64_t);
    void continueSqlReplace(int64_t);
    void checkSqlLock(deadlockchange_e, bool, int64_t, int64_t, int64_t, int64_t,
                      fieldValue_s *);
    void commit();
    int64_t getnextpendingcmdid(void);

    //private:
    class TransactionAgent *taPtr;
    int64_t transactionid;
    int64_t domainid;
    class Schema *schemaPtr;
    boost::unordered_map<int64_t, int64_t> engineToSubTransactionids;
    transactionstate_e state;
    pendingprimitive_e pendingcmd;
    int64_t pendingcmdid;
    class Message *msgrcv;
    int64_t nextpendingcmdid;
    // lock stuff
    int64_t lockcount;
    int64_t lockpendingcount;
    //  vector<locked_s> lockedItems;
    // re-entry info for stored procedure
    class ApiInterface *reentryObject;
    apifPtr reentryFuncPtr;
    int64_t reentryCmd;
    void *reentryState;
    // end re-entry info
    // things for each dml
    cmdState_s currentCmdState;
    int64_t tableid;
    int64_t rowEngineid; // partition for the main field of the table where the
    // row lives
    int64_t resultCode;
    // things for insert()
    vector<fieldValue_s> fieldValues;
    int64_t enginesWithUniqueIndices; // drain this as unique index insert
    // messages come in
    // rowid,tableid,engineid
    uuRecord_s returnNewRow; // value to return as unique row after insert!,
    // also this is the data necessary for rollback of a particular insert:
    // rowid,tableid,engineid (and subtransactionid)
    // for update
    fieldValue_s mainFieldValue;
    fieldValue_s updateFieldValue;
    // update rollback info: engineid,tableid,fieldid,entry (and subtransactionid)
    // for delete
    // to rollback: rowid,tableid,engineid (and subtransactionid)
    // for select, vector of [rowid,tableid,engineid]:
    vector< uuRecord_s > returnselectedrows;
    boost::unordered_map< uuRecord_s, stagedRow_s > stagedRows;
    boost::unordered_map< int64_t, fieldValue_s > fieldsToUpdate;

    sqlcmdstate_s sqlcmdstate;

    int waitfordispatched;
};

#endif  /* INFINISQLTRANSACTION_H */
