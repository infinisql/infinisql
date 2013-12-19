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
 * @file   Transaction.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 14:05:05 2013
 * 
 * @brief  Class which performs transactions. Associated with the
 * TransactionAgent which connected to the client that initiated the request.
 */

#ifndef INFINISQLTRANSACTION_H
#define INFINISQLTRANSACTION_H

#include "gch.h"
#include "TransactionAgent.h"
#include "DeadlockMgr.h"

class TransactionAgent;

/*
size_t hash_value(uuRecord_s const &);
bool operator==(uuRecord_s const &, uuRecord_s const &);
*/

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
        std::vector<indexEntry_s> indexHits;
        void *continuationData;
        bool ispossibledeadlock;
    };

    typedef struct
    {
        int64_t tableid;
        class Table *tablePtr;
        std::vector<indexInfo_s> indexEntries;
        int64_t rowEngineid; // partition for the main field of the table
                             // where the row lives
        int64_t rowid;
        int64_t engineid; // for index value, not message destination!
        locktype_e locktype;
        std::string row;
        int64_t enginesWithUniqueIndices;
        int64_t engines;
        int64_t fieldid;
        bool isunique; // for update 1 field
        int64_t destinationengineid;
        std::string *rowPtr;

        std::vector<indexEntry_s> rowidsEngineids;
        fieldValue_s oldFieldValue;
        boost::unordered_map< int64_t, class MessageCommitRollback *>
            replaceEngineMsgs; // for select
        bool ispossibledeadlock;
        boost::unordered_map< uuRecord_s, stagedRow_s > pendingStagedRows;
        uuRecord_s originaluur;
        uuRecord_s newuur;
        bool isupdatemultiplefields;
        fieldValue_s fieldVal;
        std::string newRow;
        std::vector<fieldValue_s> originalFieldValues;
        std::vector<fieldValue_s> newFieldValues;
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

    Transaction(class TransactionAgent *taPtrarg, int64_t domainidarg);
    virtual ~Transaction();

    // keep all/mostly public for stored procedures
    //private:
    void zeroCurrentCmdState();
    void processTransactionMessage(class Message *msgrcvarg);
    int64_t getEngineid(class Table *tablePtr, int64_t fieldnum);
    int64_t getEngineid(class Table *tablePtr, int64_t fieldid,
                        fieldValue_s *val);
    int64_t getEngineid(int64_t input);
    int64_t getEngineid(uint64_t input);
    int64_t getEngineid(bool input);
    int64_t getEngineid(long double input);
    int64_t getEngineid(char input);
    int64_t getEngineid(string *input);
    int64_t getengine(fieldtype_e fieldtype, fieldValue_s &fieldValue);
    void dispatch(class Message *msgrcv);
    void dispatched(class Message *msgrcv);
    void checkLock(deadlockchange_e changetype, bool isrow, int64_t rowid,
                   int64_t tableid, int64_t engineid, int64_t fieldid,
                   fieldValue_s *fieldValue);
    void badMessageHandler();
    // for ApiInterface::insert()
    void addFieldToRow();
    void addFieldToRow(int64_t val);
    void addFieldToRow(uint64_t val);
    void addFieldToRow(bool val);
    void addFieldToRow(long double val);
    void addFieldToRow(char val);
    void addFieldToRow(string &val);
    void reenter();
    // returns true if field passes null constraint check, false if fails
    // constraint check
    bool checkNullConstraintOK(int64_t fieldnum);
    void makeFieldValue(fieldValue_s *val, bool isnull, int64_t input);
    void makeFieldValue(fieldValue_s *val, bool isnull, uint64_t input);
    void makeFieldValue(fieldValue_s *val, bool isnull, bool input);
    void makeFieldValue(fieldValue_s *val, bool isnull, long double input);
    void makeFieldValue(fieldValue_s *val, bool isnull, char input);
    void makeFieldValue(fieldValue_s *val, bool isnull, string input);
    void reenter(int64_t res);
    void replace();
    void select(int64_t tableid, int64_t fieldid, locktype_e locktype,
                searchParams_s *searchParameters);
    void continueInsertRow(int64_t entrypoint);
    void continueUpdateRow(int64_t entrypoint);
    void continueDeleteRow(int64_t entrypoint);
    void continueReplaceRow(int64_t entrypoint);
    void continueSelectRows(int64_t entrypoint);
    void continueFetchRows(int64_t entrypoint);
    void continueUnlockRow(int64_t entrypoint);
    void continueCommitTransaction(int64_t entrypoint);
    void continueRollbackTransaction(int64_t entrypoint);
    void rollback();
    void revertback(uuRecord_s &uur, enginecmd_e cmd);
    void abortCmd(int reentrystatus);
    void sendTransaction(enginecmd_e enginecmd, payloadtype_e payloadtype,
                         int64_t tacmdentrypoint, int64_t engineid , void *data);
    void deadlockAbort(class MessageDeadlock &msgref);
    void updateRow();
    void rollback(uuRecord_s &uur);
    void revert(uuRecord_s &uur);
    void addRof(int64_t engineid, rowOrField_s rof&,
                boost::unordered_map< int64_t,
                class MessageCommitRollback *> &msgs);
    class MessageDispatch *makeMessageDispatch();

    void sqlPredicate(class Statement *statement, operatortypes_e op,
                      int64_t tableid, string &leftoperand,
                      string &rightoperand, locktype_e locktype,
                      vector<fieldValue_s> &inValues, void *continuationData,
                      boost::unordered_map<uuRecord_s, returnRow_s> &results);
    void continueSqlPredicate(int64_t entrypoint);
    void sqlSelectAll(class Statement *statement, int64_t tableid,
                      locktype_e locktype, pendingprimitive_e pendingprimitive,
                      boost::unordered_map<uuRecord_s, returnRow_s> &results);
    void continueSqlDelete(int64_t entrypoint);
    void continueSqlInsert(int64_t entrypoint);
    void continueSqlUpdate(int64_t entrypoint);
    void continueSqlReplace(int64_t entrypoint);
    void checkSqlLock(deadlockchange_e changetype, bool isrow, int64_t rowid,
                      int64_t tableid, int64_t engineid, int64_t fieldid,
                      fieldValue_s *fieldVal);
    void commit();
    int64_t getnextpendingcmdid();

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
    std::vector<fieldValue_s> fieldValues;
    int64_t enginesWithUniqueIndices; // drain this as unique index insert
    // messages come in
    // rowid,tableid,engineid
    uuRecord_s returnNewRow; // value to return as unique row after insert!,
    // also this is the data necessary for rollback of a particular insert:
    // rowid,tableid,engineid (and subtransactionid)
    // for update
    fieldValue_s mainFieldValue;
    fieldValue_s updateFieldValue;
    // update rollback info: engineid,tableid,fieldid,entry (and
    // subtransactionid)
    // for delete
    // to rollback: rowid,tableid,engineid (and subtransactionid)
    // for select, vector of [rowid,tableid,engineid]:
    std::vector< uuRecord_s > returnselectedrows;
    boost::unordered_map< uuRecord_s, stagedRow_s > stagedRows;
    boost::unordered_map< int64_t, fieldValue_s > fieldsToUpdate;

    sqlcmdstate_s sqlcmdstate;

    int waitfordispatched;
};

#endif  /* INFINISQLTRANSACTION_H */
