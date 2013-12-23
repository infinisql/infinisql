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

/** 
 * @brief create Transaction object
 *
 * @param taPtrarg TransactionAgent
 * @param domainidarg domainid
 */
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
    /** 
     * @brief clear current command state
     *
     */
    void zeroCurrentCmdState();
    /** 
     * @brief execute continuation based on MessageTransaction reply
     *
     * @param msgrcvarg MessageTransaction variant received
     */
    void processTransactionMessage(class Message *msgrcvarg);
    /** 
     * @brief get engineid/partitionid based on hash of this fieldValues
     *
     * @param tablePtr Table
     * @param fieldnum fieldid
     *
     * @return engine/partitionid
     */
    int64_t getEngineid(class Table *tablePtr, int64_t fieldnum);
    /** 
     * @brief get engine/partitionid based on hash of input
     *
     * @param tablePtr Table
     * @param fieldid fieldid
     * @param val input value
     *
     * @return engine/partitionid
     */
    int64_t getEngineid(class Table *tablePtr, int64_t fieldid,
                        fieldValue_s *val);
    /** 
     * @brief get engine/partitionid based on hash of input
     *
     * @param input input value
     *
     * @return engine/partitionid
     */
    int64_t getEngineid(int64_t input);    
    /** 
     * @brief get engine/partitionid based on hash of input
     *
     * @param input input value
     *
     * @return engine/partitionid
     */
    int64_t getEngineid(uint64_t input);
    /** 
     * @brief get engine/partitionid based on hash of input
     *
     * @param input input value
     *
     * @return engine/partitionid
     */
    int64_t getEngineid(bool input);
    /** 
     * @brief get engine/partitionid based on hash of input
     *
     * @param input input value
     *
     * @return engine/partitionid
     */
    int64_t getEngineid(long double input);
    /** 
     * @brief get engine/partitionid based on hash of input
     *
     * @param input input value
     *
     * @return engine/partitionid
     */
    int64_t getEngineid(char input);
    /** 
     * @brief get engine/partitionid based on hash of input
     *
     * @param input input value
     *
     * @return engine/partitionid
     */
    int64_t getEngineid(string *input);
    /** 
     * @brief get engine/partitionid based on hash of input
     *
     * @param fieldtype field type
     * @param fieldValue input value
     *
     * @return engine/partitionid
     */
    int64_t getengine(fieldtype_e fieldtype, fieldValue_s &fieldValue);
    /** 
     * @brief orphan, never implemented so far, stub
     *
     * @param msgrcv 
     */
    void dispatch(class Message *msgrcv);
    /** 
     * @brief orphan, never implemented so far, stub
     *
     * @param msgrcv 
     */
    void dispatched(class Message *msgrcv);
    /** 
     * @brief based on type of lock and other factors, intiate deadlock management
     *
     * @param changetype type of deadlock-affecting change event
     * @param isrow row or unique index
     * @param rowid rowid
     * @param tableid tableid
     * @param engineid engineid
     * @param fieldid fieldid
     * @param fieldValue field value
     */
    void checkLock(deadlockchange_e changetype, bool isrow, int64_t rowid,
                   int64_t tableid, int64_t engineid, int64_t fieldid,
                   fieldValue_s *fieldValue);
    /** 
     * @brief stub for unhandleable Message variant received
     *
     */
    void badMessageHandler();
    // for ApiInterface::insert()
    /** 
     * @brief deprecated
     *
     */
    void addFieldToRow();
    /** 
     * @brief deprecated
     *
     */
    void addFieldToRow(int64_t val);
    /** 
     * @brief deprecated
     *
     */
    void addFieldToRow(uint64_t val);
    /** 
     * @brief deprecated
     *
     */
    void addFieldToRow(bool val);
    /** 
     * @brief deprecated
     *
     */
    void addFieldToRow(long double val);
    /** 
     * @brief deprecated
     *
     */
    void addFieldToRow(char val);
    /** 
     * @brief deprecated
     *
     */
    void addFieldToRow(string &val);
    /** 
     * @brief continue back to calling function
     *
     */
    void reenter();
    /** 
     * @brief check if NULL constraint is violated
     *
     * @param fieldnum true if no violation, false if violation
     *
     * @return 
     */
    bool checkNullConstraintOK(int64_t fieldnum);
    /** 
     * @brief deprecated
     *
     * @param val 
     * @param isnull 
     * @param input 
     */
    void makeFieldValue(fieldValue_s *val, bool isnull, int64_t input);
    /** 
     * @brief deprecated
     *
     * @param val 
     * @param isnull 
     * @param input 
     */
    void makeFieldValue(fieldValue_s *val, bool isnull, uint64_t input);
    /** 
     * @brief deprecated
     *
     * @param val 
     * @param isnull 
     * @param input 
     */
    void makeFieldValue(fieldValue_s *val, bool isnull, bool input);
    /** 
     * @brief deprecated
     *
     * @param val 
     * @param isnull 
     * @param input 
     */
    void makeFieldValue(fieldValue_s *val, bool isnull, long double input);
    /** 
     * @brief deprecated
     *
     * @param val 
     * @param isnull 
     * @param input 
     */
    void makeFieldValue(fieldValue_s *val, bool isnull, char input);
    /** 
     * @brief deprecated
     *
     * @param val 
     * @param isnull 
     * @param input 
     */
    void makeFieldValue(fieldValue_s *val, bool isnull, string input);
    /** 
     * @brief continue back to calling function
     *
     * @param reentry status
     */
    void reenter(int64_t res);
    /** 
     * @brief deprecated
     *
     */
    void replace();
    /** 
     * @brief deprecated
     *
     * @param tableid 
     * @param fieldid 
     * @param locktype 
     * @param searchParameters 
     */
    void select(int64_t tableid, int64_t fieldid, locktype_e locktype,
                searchParams_s *searchParameters);
    /** 
     * @brief deprecated
     *
     * @param entrypoint 
     */
    void continueInsertRow(int64_t entrypoint);
    /** 
     * @brief deprecated
     *
     * @param entrypoint 
     */
    void continueUpdateRow(int64_t entrypoint);
    /** 
     * @brief deprecated
     *
     * @param entrypoint 
     */
    void continueDeleteRow(int64_t entrypoint);
    /** 
     * @brief deprecated
     *
     * @param entrypoint 
     */
    void continueReplaceRow(int64_t entrypoint);
    /** 
     * @brief deprecated
     *
     * @param entrypoint 
     */
    void continueSelectRows(int64_t entrypoint);
    /** 
     * @brief deprecated
     *
     * @param entrypoint 
     */
    void continueFetchRows(int64_t entrypoint);
    /** 
     * @brief deprecated
     *
     * @param entrypoint 
     */
    void continueUnlockRow(int64_t entrypoint);
    /** 
     * @brief continuation of COMMIT
     *
     * @param entrypoint entrypoint from which to continue
     */
    void continueCommitTransaction(int64_t entrypoint);
    /** 
     * @brief continuation of ROLLBACK
     *
     * @param entrypoint entrypoint from which to continue
     */
    void continueRollbackTransaction(int64_t entrypoint);
    /** 
     * @brief ROLLBACK
     *
     */
    void rollback();
    /** 
     * @brief likely deprecated
     *
     * @param uur 
     * @param cmd 
     */
    void revertback(uuRecord_s &uur, enginecmd_e cmd);
    /** 
     * @brief deprecated
     *
     * @param reentrystatus 
     */
    void abortCmd(int reentrystatus);
    /** 
     * @brief send MessageTransaction variant to Engine
     *
     * @param enginecmd command to execute
     * @param payloadtype MessageTransaction variant
     * @param tacmdentrypoint return function continuation entry point
     * @param engineid destination partitionid
     * @param data MessageTransaction variant
     */
    void sendTransaction(enginecmd_e enginecmd, payloadtype_e payloadtype,
                         int64_t tacmdentrypoint, int64_t engineid , void *data);
    /** 
     * @brief likely deprecated
     *
     * @param msgref 
     */
    void deadlockAbort(class MessageDeadlock &msgref);
    /** 
     * @brief deprecated
     *
     */
    void updateRow();
    /** 
     * @brief likely deprecated
     *
     * @param uur 
     */
    void rollback(uuRecord_s &uur);
    /** 
     * @brief likely deprecated
     *
     * @param uur 
     */
    void revert(uuRecord_s &uur);
    /** 
     * @brief add entry (row or field) to list for Engine processing
     *
     * @param engineid destination partitionid
     * @param rof entry to add
     * @param msgs MessageCommitRollback to add entry to
     */
    void addRof(int64_t engineid, rowOrField_s rof&,
                boost::unordered_map< int64_t,
                class MessageCommitRollback *> &msgs);
    /** 
     * @brief create MessageDispatch for synchronous replication
     *
     *
     * @return MessageDispatch object
     */
    class MessageDispatch *makeMessageDispatch();

    /** 
     * @brief SQL predicate search
     *
     * @param statement Statment
     * @param op predicate operator type
     * @param tableid tableid
     * @param leftoperand left operand of operation
     * @param rightoperand right operand of operation
     * @param locktype lock type
     * @param inValues if IN (or NOT IN), list of values to check
     * @param continuationData data to pass to continuation function
     * @param results results of search
     */
    void sqlPredicate(class Statement *statement, operatortypes_e op,
                      int64_t tableid, string &leftoperand,
                      string &rightoperand, locktype_e locktype,
                      vector<fieldValue_s> &inValues, void *continuationData,
                      boost::unordered_map<uuRecord_s, returnRow_s> &results);
    /** 
     * @brief continuation of SQL predicate search
     *
     * @param entrypoint entry point from which to continue
     */
    void continueSqlPredicate(int64_t entrypoint);
    /** 
     * @brief get all rows from a table
     *
     * @param statement Statement
     * @param tableid tableid
     * @param locktype lock type
     * @param pendingprimitive type of query (SELECT|UPDATE|DELETE)
     * @param results result rows
     */
    void sqlSelectAll(class Statement *statement, int64_t tableid,
                      locktype_e locktype, pendingprimitive_e pendingprimitive,
                      boost::unordered_map<uuRecord_s, returnRow_s> &results);
    /** 
     * @brief continuation of DELETE
     *
     * @param entrypoint entry point from which to continue
     */
    void continueSqlDelete(int64_t entrypoint);
    /** 
     * @brief continuation of INSERT
     *
     * @param entrypoint entry point from which to continue
     */
    void continueSqlInsert(int64_t entrypoint);
    /** 
     * @brief continuation of UPDATE
     *
     * @param entrypoint entry point from which to continue
     */
    void continueSqlUpdate(int64_t entrypoint);
    /** 
     * @brief continuation of REPLACE (update of field 0 in a row)
     *
     * @param entrypoint entry point from which to continue
     */
    void continueSqlReplace(int64_t entrypoint);
    /** 
     * @brief orphan
     *
     * @param changetype 
     * @param isrow 
     * @param rowid 
     * @param tableid 
     * @param engineid 
     * @param fieldid 
     * @param fieldVal 
     */
    void checkSqlLock(deadlockchange_e changetype, bool isrow, int64_t rowid,
                      int64_t tableid, int64_t engineid, int64_t fieldid,
                      fieldValue_s *fieldVal);
    /** 
     * @brief COMMIT
     *
     */
    void commit();
    /** 
     * @brief generate unique, always incrementing id for pending command
     *
     *
     * @return pending command identifier
     */
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
