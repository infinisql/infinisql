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
 * @file   api.cc
 * @author  <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:01:05 2013
 * 
 * @brief  API for creating stored procedures. Pg objects also inherit
 * from this class, because they perform the same types of transactional
 * activities as a stored procedure.
 */

#include "gch.h"
#include "Transaction.h"
#include "infinisql.h"
#include "Asts.h"
#line 35 "api.cc"

void ApiInterface::deserialize2Vector()
{
    msgpack::unpacked msg;
    msgpack::unpack(&msg, taPtr->args, taPtr->argsize);
    msgpack::object obj = msg.get();
    obj.convert(&inputVector);
}

void ApiInterface::beginTransaction()
{
    transactionPtr = new class Transaction(taPtr, domainid);
}

void ApiInterface::destruct()
{
    spclassdestroy d = (spclassdestroy)destroyerPtr;
    d(this);
}

void ApiInterface::bouncebackproxy()
{
    transactionPtr->reenter();
}

// ok the real functions
void ApiInterface::insertRow(apifPtr re, int64_t recmd, void *reptr,
                             int64_t tableid)
{
    setReEntry(re, recmd, reptr);

    if (transactionPtr->pendingcmd != NOCOMMAND)
    {
        transactionPtr->reenter(APISTATUS_PENDING);
        return;
    }

    uuRecord_s uur = { -1, -1, -1 };
    transactionPtr->returnNewRow = uur; // why this here?
    transactionPtr->pendingcmdid = transactionPtr->getnextpendingcmdid();
    transactionPtr->pendingcmd = INSERT;
    transactionPtr->zeroCurrentCmdState();
    transactionPtr->currentCmdState.tableid = tableid;
    transactionPtr->currentCmdState.tablePtr =
        transactionPtr->schemaPtr->tables[tableid];

    indexInfo_s blankIdxInfo = {};
    transactionPtr->currentCmdState.indexEntries.resize
        (transactionPtr->currentCmdState.tablePtr->fields.size(), blankIdxInfo);
    indexEntry_s blankIdxEntry = {};
    transactionPtr->currentCmdState.rowidsEngineids.resize
        (transactionPtr->currentCmdState.tablePtr->fields.size(), blankIdxEntry);

    for (size_t n=0; n<transactionPtr->currentCmdState.tablePtr->fields.size();
         n++)
    {
        if (transactionPtr->checkNullConstraintOK(n)==false)
        {
            transactionPtr->reenter(APISTATUS_NULLCONSTRAINT);
            return;
        }

        if (transactionPtr->currentCmdState.tablePtr->fields[n].indextype != NONE)
        {
            transactionPtr->currentCmdState.indexEntries[n].engineid =
                transactionPtr->getEngineid(transactionPtr->currentCmdState.tablePtr, n);
            transactionPtr->currentCmdState.indexEntries[n].fieldid = n;
            transactionPtr->currentCmdState.indexEntries[n].tableid =
                transactionPtr->currentCmdState.tableid;
            transactionPtr->currentCmdState.indexEntries[n].fieldVal =
                transactionPtr->fieldValues[n];
        }
        else
        {
            transactionPtr->currentCmdState.indexEntries[n].engineid = -1;
        }

        transactionPtr->currentCmdState.rowidsEngineids[n].engineid =
            transactionPtr->currentCmdState.indexEntries[n].engineid;
    }

    transactionPtr->currentCmdState.rowEngineid =
        transactionPtr->getEngineid(transactionPtr->currentCmdState.tablePtr, 0);

    if (transactionPtr->currentCmdState.rowEngineid == -1)
    {
        transactionPtr->reenter(APISTATUS_NOTOK);
        return;
    }

    // now, prepare message for Engine, which expects:
    // transaction_enginecmd NEWROW, transaction_messageStruct.payloadtype
    // SUBTRANSACTIONCMDPAYLOAD
    // reentrypoint 1
    // tableid, row
    // expects in return: rowid
    class MessageSubtransactionCmd *msg = new class MessageSubtransactionCmd();
    class MessageSubtransactionCmd &msgref = *msg;
    msgref.subtransactionStruct.tableid=tableid;

    if (transactionPtr->currentCmdState.tablePtr->
        makerow(&transactionPtr->fieldValues, &msgref.row)==false)
    {
        transactionPtr->reenter(APISTATUS_FIELD);
        delete msg;
        return;
    }

    vector<fieldValue_s> f;
    transactionPtr->currentCmdState.tablePtr->unmakerow(&msgref.row, &f);
    transactionPtr->currentCmdState.row = msgref.row;
    transactionPtr->sendTransaction(NEWROW, PAYLOADSUBTRANSACTION, 1,
                                    transactionPtr->currentCmdState.rowEngineid,
                                    (void *)msg);
}

void ApiInterface::deleteRow(apifPtr re, int64_t recmd, void *reptr,
                             uuRecord_s &uur)
{
    setReEntry(re, recmd, reptr);

    if (!transactionPtr->stagedRows.count(uur))
    {
        transactionPtr->reenter(APISTATUS_NOTOK);
        return;
    }

    if (transactionPtr->stagedRows[uur].locktype != WRITELOCK ||
        transactionPtr->stagedRows[uur].cmd != NOCOMMAND ||
        transactionPtr->pendingcmd != NOCOMMAND)
    {
        transactionPtr->reenter(APISTATUS_NOTOK);
        return;
    }

    transactionPtr->pendingcmdid = transactionPtr->getnextpendingcmdid();
    transactionPtr->pendingcmd = DELETE;
    transactionPtr->zeroCurrentCmdState();

    transactionPtr->currentCmdState.originaluur = uur;

    // tableid,rowid,DELETEROW,SUBTRANSACTIONCMDPAYLOAD, returns status
    class MessageSubtransactionCmd *msg = new class MessageSubtransactionCmd();
    class MessageSubtransactionCmd &msgref = *msg;
    msgref.subtransactionStruct.rowid =
        transactionPtr->currentCmdState.originaluur.rowid;
    msgref.subtransactionStruct.tableid =
        transactionPtr->currentCmdState.originaluur.tableid;
    transactionPtr->sendTransaction(DELETEROW, PAYLOADSUBTRANSACTION, 1,
                                    transactionPtr->currentCmdState.originaluur.engineid,
                                    (void *)msg);
}

void ApiInterface::replaceRow(apifPtr re, int64_t recmd, void *reptr)
{
    setReEntry(re, recmd, reptr);

    if (transactionPtr->pendingcmd != NOCOMMAND)
    {
        transactionPtr->reenter(APISTATUS_PENDING);
        return;
    }

    transactionPtr->pendingcmdid = transactionPtr->getnextpendingcmdid();
    transactionPtr->pendingcmd = REPLACE;
}

void ApiInterface::fetchRows(apifPtr re, int64_t recmd, void *reptr)
{
    setReEntry(re, recmd, reptr);

    if (transactionPtr->pendingcmd != NOCOMMAND)
    {
        transactionPtr->reenter(APISTATUS_PENDING);
        return;
    }

    transactionPtr->pendingcmdid = transactionPtr->getnextpendingcmdid();
    transactionPtr->pendingcmd = FETCH;
}

void ApiInterface::unlock(apifPtr re, int64_t recmd, void *reptr, int64_t rowid,
                          int64_t tableid, int64_t engineid)
{
    setReEntry(re, recmd, reptr);

    if (transactionPtr->pendingcmd != NOCOMMAND)
    {
        transactionPtr->reenter(APISTATUS_PENDING);
        return;
    }

    transactionPtr->pendingcmdid = transactionPtr->getnextpendingcmdid();
    transactionPtr->pendingcmd = UNLOCK;
}

void ApiInterface::commit(apifPtr re, int64_t recmd, void *reptr)
{
    setReEntry(re, recmd, reptr);

    if (transactionPtr->pendingcmd != NOCOMMAND)
    {
        transactionPtr->reenter(APISTATUS_PENDING);
        return;
    }

    transactionPtr->commit();
}

void ApiInterface::rollback(apifPtr re, int64_t recmd, void *reptr)
{
    setReEntry(re, recmd, reptr);

    if (transactionPtr->pendingcmd != NOCOMMAND)
    {
        transactionPtr->reenter(APISTATUS_PENDING);
        return;
    }

    transactionPtr->rollback();
}

void ApiInterface::rollback(apifPtr re, int64_t recmd, void *reptr,
                            uuRecord_s &uur)
{
    setReEntry(re, recmd, reptr);

    if (transactionPtr->pendingcmd != NOCOMMAND)
    {
        transactionPtr->reenter(APISTATUS_PENDING);
        return;
    }

    transactionPtr->rollback(uur);
}

void ApiInterface::revert(apifPtr re, int64_t recmd, void *reptr, uuRecord_s &uur)
{
    setReEntry(re, recmd, reptr);

    if (transactionPtr->pendingcmd != NOCOMMAND)
    {
        transactionPtr->reenter(APISTATUS_PENDING);
        return;
    }

    transactionPtr->revert(uur);
}

// for isnull and isnotnull and selectall only
void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op)
{
    setReEntry(re, recmd, reptr);

    if ((op != OPERATOR_ISNULL) && (op != OPERATOR_ISNOTNULL) &&
        (op != OPERATOR_SELECTALL))
    {
        transactionPtr->reenter(APISTATUS_NOTOK);
        return;
    }

    searchParams_s searchParameters = {};
    searchParameters.op = op;
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

// eq,neq,gt,lt,gte,lte,regex
void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              int64_t input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    fieldValue_s fieldVal = {};
    fieldVal.value.integer = input;
    searchParameters.values.push_back(fieldVal);
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              uint64_t input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    fieldValue_s fieldVal = {};
    fieldVal.value.uinteger = input;
    searchParameters.values.push_back(fieldVal);
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              bool input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    fieldValue_s fieldVal = {};
    fieldVal.value.boolean = input;
    searchParameters.values.push_back(fieldVal);
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              long double input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    fieldValue_s fieldVal = {};
    fieldVal.value.floating = input;
    searchParameters.values.push_back(fieldVal);
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              char input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    fieldValue_s fieldVal = {};
    fieldVal.value.character = input;
    searchParameters.values.push_back(fieldVal);
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              string *input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;

    if (op == OPERATOR_REGEX)
    {
        searchParameters.regexString = *input;
    }
    else
    {
        fieldValue_s fieldVal = {};
        fieldVal.str = *input;
        searchParameters.values.push_back(fieldVal);
    }

    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

// in
void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              vector<int64_t> *input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;

    fieldValue_s blankFieldVal = {};
    fieldValue_s fieldVal;
    searchParameters.values.reserve(input->size());

    for (size_t n=0; n<input->size(); n++)
    {
        fieldVal = blankFieldVal;
        fieldVal.value.integer = input->at(n);
        searchParameters.values[n] = fieldVal;
    }

    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              vector<uint64_t> *input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;

    fieldValue_s blankFieldVal = {};
    fieldValue_s fieldVal;
    searchParameters.values.reserve(input->size());

    for (size_t n=0; n<input->size(); n++)
    {
        fieldVal = blankFieldVal;
        fieldVal.value.uinteger = input->at(n);
        searchParameters.values[n] = fieldVal;
    }

    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              vector<bool> *input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;

    fieldValue_s blankFieldVal = {};
    fieldValue_s fieldVal;
    searchParameters.values.reserve(input->size());

    for (size_t n=0; n<input->size(); n++)
    {
        fieldVal = blankFieldVal;
        fieldVal.value.boolean = input->at(n);
        searchParameters.values[n] = fieldVal;
    }

    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              vector<long double> *input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;

    fieldValue_s blankFieldVal = {};
    fieldValue_s fieldVal;
    searchParameters.values.reserve(input->size());

    for (size_t n=0; n<input->size(); n++)
    {
        fieldVal = blankFieldVal;
        fieldVal.value.floating = input->at(n);
        searchParameters.values[n] = fieldVal;
    }

    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              vector<char> *input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;

    fieldValue_s blankFieldVal = {};
    fieldValue_s fieldVal;
    searchParameters.values.reserve(input->size());

    for (size_t n=0; n<input->size(); n++)
    {
        fieldVal = blankFieldVal;
        fieldVal.value.character = input->at(n);
        searchParameters.values[n] = fieldVal;
    }

    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              vector<string> *input)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;

    fieldValue_s blankFieldVal = {};
    fieldValue_s fieldVal;
    searchParameters.values.reserve(input->size());

    for (size_t n=0; n<input->size(); n++)
    {
        fieldVal = blankFieldVal;
        fieldVal.str = input->at(n);
        searchParameters.values[n] = fieldVal;
    }

    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

// between
void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              int64_t lower, int64_t upper)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    searchParameters.values.reserve(2);
    fieldValue_s fieldVal = {};
    fieldVal.value.integer = lower;
    searchParameters.values[0] = fieldVal;
    fieldVal.value.integer = upper;
    searchParameters.values[1] = fieldVal;
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              uint64_t lower, uint64_t upper)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    searchParameters.values.reserve(2);
    fieldValue_s fieldVal = {};
    fieldVal.value.uinteger = lower;
    searchParameters.values[0] = fieldVal;
    fieldVal.value.uinteger = upper;
    searchParameters.values[1] = fieldVal;
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

// between bools makes no sense, but whatever
void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              bool lower, bool upper)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    searchParameters.values.reserve(2);
    fieldValue_s fieldVal = {};
    fieldVal.value.boolean = lower;
    searchParameters.values[0] = fieldVal;
    fieldVal.value.boolean = upper;
    searchParameters.values[1] = fieldVal;
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              long double lower, long double upper)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    searchParameters.values.reserve(2);
    fieldValue_s fieldVal = {};
    fieldVal.value.floating = lower;
    searchParameters.values[0] = fieldVal;
    fieldVal.value.floating = upper;
    searchParameters.values[1] = fieldVal;
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              char lower, char upper)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    searchParameters.values.reserve(2);
    fieldValue_s fieldVal = {};
    fieldVal.value.character = lower;
    searchParameters.values[0] = fieldVal;
    fieldVal.value.character = upper;
    searchParameters.values[1] = fieldVal;
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::selectRows(apifPtr re, int64_t recmd, void *reptr,
                              int64_t tableid, int64_t fieldid,
                              locktype_e locktype, operatortypes_e op,
                              string *lower, string *upper)
{
    setReEntry(re, recmd, reptr);
    searchParams_s searchParameters = {};
    searchParameters.op = op;
    searchParameters.values.reserve(2);
    fieldValue_s fieldVal = {};
    fieldVal.str = *lower;
    searchParameters.values[0] = fieldVal;
    fieldVal.str = *upper;
    searchParameters.values[1] = fieldVal;
    transactionPtr->select(tableid, fieldid, locktype, &searchParameters);
}

void ApiInterface::sendResponse(int64_t resultCode, vector<string> *v)
{
    taPtr->sockfd = sockfd;
    taPtr->sendResponse(false, resultCode, v);
}

bool ApiInterface::unmakerow(int64_t tableid, string *rowstring,
                             vector<fieldValue_s> *resultFields)
{
    return transactionPtr->schemaPtr->tables[tableid]->unmakerow(rowstring,
                                                                 resultFields);
}

void ApiInterface::updateRow(apifPtr re, int64_t recmd, void *reptr,
                             uuRecord_s &uur)
{
    setReEntry(re, recmd, reptr);
    transactionPtr->currentCmdState.originaluur = uur;
    transactionPtr->currentCmdState.isupdatemultiplefields = true; // look in
    // uses Transaction::fieldsToUpdate map
    transactionPtr->updateRow();
}
void ApiInterface::updateRowNullField(apifPtr re, int64_t recmd, void *reptr,
                                      uuRecord_s &uur, int64_t fieldid)
{
    setReEntry(re, recmd, reptr);
    transactionPtr->currentCmdState.originaluur = uur;
    transactionPtr->currentCmdState.isupdatemultiplefields = false;
    transactionPtr->currentCmdState.fieldid = fieldid;
    transactionPtr->currentCmdState.fieldVal.isnull = true;

    transactionPtr->updateRow();
}
void ApiInterface::updateRow(apifPtr re, int64_t recmd, void *reptr,
                             uuRecord_s &uur, int64_t fieldid, int64_t input)
{
    setReEntry(re, recmd, reptr);
    transactionPtr->currentCmdState.originaluur = uur;
    transactionPtr->currentCmdState.isupdatemultiplefields = false;
    transactionPtr->currentCmdState.fieldid = fieldid;
    transactionPtr->currentCmdState.fieldVal.isnull = false;
    transactionPtr->currentCmdState.fieldVal.value.integer = input;

    transactionPtr->updateRow();
}
void ApiInterface::updateRow(apifPtr re, int64_t recmd, void *reptr,
                             uuRecord_s &uur, int64_t fieldid, uint64_t input)
{
    setReEntry(re, recmd, reptr);
    transactionPtr->currentCmdState.originaluur = uur;
    transactionPtr->currentCmdState.isupdatemultiplefields = false;
    transactionPtr->currentCmdState.fieldid = fieldid;
    transactionPtr->currentCmdState.fieldVal.isnull = false;
    transactionPtr->currentCmdState.fieldVal.value.uinteger = input;

    transactionPtr->updateRow();
}
void ApiInterface::updateRow(apifPtr re, int64_t recmd, void *reptr,
                             uuRecord_s &uur, int64_t fieldid, bool input)
{
    setReEntry(re, recmd, reptr);
    transactionPtr->currentCmdState.originaluur = uur;
    transactionPtr->currentCmdState.isupdatemultiplefields = false;
    transactionPtr->currentCmdState.fieldid = fieldid;
    transactionPtr->currentCmdState.fieldVal.isnull = false;
    transactionPtr->currentCmdState.fieldVal.value.boolean = input;

    transactionPtr->updateRow();
}
void ApiInterface::updateRow(apifPtr re, int64_t recmd, void *reptr,
                             uuRecord_s &uur, int64_t fieldid, long double input)
{
    setReEntry(re, recmd, reptr);
    transactionPtr->currentCmdState.originaluur = uur;
    transactionPtr->currentCmdState.isupdatemultiplefields = false;
    transactionPtr->currentCmdState.fieldid = fieldid;
    transactionPtr->currentCmdState.fieldVal.isnull = false;
    transactionPtr->currentCmdState.fieldVal.value.floating = input;

    transactionPtr->updateRow();
}
void ApiInterface::updateRow(apifPtr re, int64_t recmd, void *reptr,
                             uuRecord_s &uur, int64_t fieldid, char input)
{
    setReEntry(re, recmd, reptr);
    transactionPtr->currentCmdState.originaluur = uur;
    transactionPtr->currentCmdState.isupdatemultiplefields = false;
    transactionPtr->currentCmdState.fieldid = fieldid;
    transactionPtr->currentCmdState.fieldVal.isnull = false;
    transactionPtr->currentCmdState.fieldVal.value.character = input;

    transactionPtr->updateRow();
}
void ApiInterface::updateRow(apifPtr re, int64_t recmd, void *reptr,
                             uuRecord_s &uur, int64_t fieldid, string input)
{
    setReEntry(re, recmd, reptr);
    transactionPtr->currentCmdState.originaluur = uur;
    transactionPtr->currentCmdState.isupdatemultiplefields = false;
    transactionPtr->currentCmdState.fieldid = fieldid;
    transactionPtr->currentCmdState.fieldVal.isnull = false;
    transactionPtr->currentCmdState.fieldVal.str = input;

    transactionPtr->updateRow();
}

void ApiInterface::prepareResponseVector(int64_t resultCode)
{
    response.resultCode = resultCode;
    msgpack::sbuffer *sbuf = new msgpack::sbuffer;
    msgpack::pack(*sbuf, responseVector);
    response.sbuf = sbuf;
}

void ApiInterface::setReEntry(apifPtr re, int64_t recmd, void *reptr)
{
    transactionPtr->reentryObject = this;
    transactionPtr->reentryFuncPtr = re;
    transactionPtr->reentryCmd = recmd;
    transactionPtr->reentryState = reptr;
}

void ApiInterface::addFieldToRow()
{
    transactionPtr->addFieldToRow();
}

void ApiInterface::addFieldToRow(int64_t val)
{
    transactionPtr->addFieldToRow(val);
}

void ApiInterface::addFieldToRow(uint64_t val)
{
    transactionPtr->addFieldToRow(val);
}

void ApiInterface::addFieldToRow(bool val)
{
    transactionPtr->addFieldToRow(val);
}

void ApiInterface::addFieldToRow(long double val)
{
    transactionPtr->addFieldToRow(val);
}

void ApiInterface::addFieldToRow(char val)
{
    transactionPtr->addFieldToRow(val);
}

void ApiInterface::addFieldToRow(string &val)
{
    transactionPtr->addFieldToRow(val);
}

bool ApiInterface::execStatement(const char *stmtname, Statement *stmtPtr,
                                 apifPtr reentryfunction, int64_t reentrypoint,
                                 void *reentrydata)
{
    return execStatement(stmtname, stmtPtr->queries[0].storedProcedureArgs,
                  reentryfunction, reentrypoint, reentrydata);
}

bool ApiInterface::execStatement(const char *stmtname, vector<string> &args,
                                 apifPtr reentryfunction, int64_t reentrypoint,
                                 void *reentrydata)
{
    if (!taPtr->statements[domainid].count(string(stmtname)))
    {
        return false;
    }
    class Statement *stmt = new class Statement;
    *stmt=taPtr->statements[domainid][string(stmtname)];
    stmt->execute(this, reentryfunction, reentrypoint, reentrydata,
                  transactionPtr, args);
    return true;
}

int64_t ApiInterface::getResultCode()
{
    return transactionPtr->resultCode;
}

void ApiInterface::deleteTransaction()
{
    delete transactionPtr;
}

void ApiInterface::deleteStatement()
{
    delete pgPtr->statementPtr;
}

void ApiInterface::getStoredProcedureArgs(Statement *stmtPtr,
                                          vector<string> &argsRef)
{
    argsRef=stmtPtr->queries[0].storedProcedureArgs;
}

size_t hash_value(uuRecord_s const &uur)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, uur.rowid);
    boost::hash_combine(seed, uur.tableid);
    boost::hash_combine(seed, uur.engineid);

    return seed;
}

bool operator==(uuRecord_s const &uur1, uuRecord_s const &uur2)
{
    return (uur1.rowid==uur2.rowid) && (uur1.tableid==uur2.tableid) &&
        (uur1.engineid==uur2.engineid);
}
