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
 * @file   SubTransaction.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:49:38 2013
 * 
 * @brief  Engine's class corresponding to TransactionAgent's Transaction.
 * Each data manipulation activity has a SubTransaction associated with the
 * Transaction which requested the activity.
 */

#include "SubTransaction.h"
#line 32 "SubTransaction.cc"

SubTransaction::SubTransaction(Topology::addressStruct &taAddrarg,
                               int64_t transactionidarg, int64_t domainidarg,
                               class Engine *enginePtrarg) :
    taAddr(taAddrarg), transactionid(transactionidarg), domainid(domainidarg),
    enginePtr(enginePtrarg)
{
    subtransactionid = enginePtr->getnextsubtransactionid();
    enginePtr->SubTransactions[subtransactionid] = this;
    schemaPtr = enginePtr->domainidsToSchemata[domainid];
}

SubTransaction::~SubTransaction()
{
    enginePtr->SubTransactions.erase(subtransactionid);
}

void SubTransaction::processTransactionMessage(class Message *msgrcvarg)
{
    msgrcv = msgrcvarg;

    switch (msgrcv->messageStruct.payloadtype)
    {
    case PAYLOADSUBTRANSACTION:
    {
        class MessageSubtransactionCmd &subtransactionCmdRef =
            *((class MessageSubtransactionCmd *)msgrcv);
        class MessageSubtransactionCmd *msg = new class MessageSubtransactionCmd;
        class MessageSubtransactionCmd &msgref = *msg;

        switch ((enginecmd_e) subtransactionCmdRef.transactionStruct.transaction_enginecmd)
        {
        case NEWROW:
        {
            msgref.subtransactionStruct.rowid =
                newrow(subtransactionCmdRef.subtransactionStruct.tableid,
                       subtransactionCmdRef.row);
            msgref.subtransactionStruct.locktype = WRITELOCK;
        }
        break;

        case UNIQUEINDEX:
        {
            msgref.subtransactionStruct.locktype =
                uniqueIndex(subtransactionCmdRef.subtransactionStruct.tableid,
                            subtransactionCmdRef.subtransactionStruct.fieldid,
                            subtransactionCmdRef.subtransactionStruct.rowid,
                            subtransactionCmdRef.subtransactionStruct.engineid,
                            &subtransactionCmdRef.fieldVal);
            msgref.subtransactionStruct.tableid =
                subtransactionCmdRef.subtransactionStruct.tableid;
            msgref.subtransactionStruct.fieldid =
                subtransactionCmdRef.subtransactionStruct.fieldid;
            msgref.fieldVal = subtransactionCmdRef.fieldVal;
        }
        break;

        case UPDATEROW:
        {
            msgref.subtransactionStruct.status =
                updaterow(subtransactionCmdRef.subtransactionStruct.tableid,
                          subtransactionCmdRef.subtransactionStruct.rowid,
                          &subtransactionCmdRef.row);
            msgref.subtransactionStruct.locktype=WRITELOCK;
        }
        break;

        case DELETEROW:
        {
            msgref.subtransactionStruct.rowid =
                subtransactionCmdRef.subtransactionStruct.rowid;
            msgref.subtransactionStruct.tableid =
                subtransactionCmdRef.subtransactionStruct.tableid;
            msgref.subtransactionStruct.engineid =
                subtransactionCmdRef.subtransactionStruct.engineid;
            msgref.subtransactionStruct.status =
                deleterow(subtransactionCmdRef.subtransactionStruct.tableid,
                          subtransactionCmdRef.subtransactionStruct.rowid);
        }
        break;

        case REPLACEDELETEROW:
        {
            msgref.subtransactionStruct.status =
                deleterow(subtransactionCmdRef.subtransactionStruct.tableid,
                          subtransactionCmdRef.subtransactionStruct.rowid,
                          subtransactionCmdRef.subtransactionStruct.forward_rowid,
                          subtransactionCmdRef.subtransactionStruct.forward_engineid);
            msgref.subtransactionStruct.locktype=WRITELOCK;
        }
        break;

        case INDEXSEARCH:
        {
            indexSearch(subtransactionCmdRef.subtransactionStruct.tableid,
                        subtransactionCmdRef.subtransactionStruct.fieldid,
                        &subtransactionCmdRef.searchParameters,
                        &msgref.indexHits);
        }
        break;

        case SELECTROWS:
        {
            selectrows(subtransactionCmdRef.subtransactionStruct.tableid,
                       &subtransactionCmdRef.rowids,
                       subtransactionCmdRef.subtransactionStruct.locktype,
                       subtransactionCmdRef.transactionStruct.transaction_pendingcmdid,
                       &msgref.returnRows);
        }
        break;

        case SEARCHRETURN1:
            searchReturn1(subtransactionCmdRef.subtransactionStruct.tableid,
                          subtransactionCmdRef.subtransactionStruct.fieldid,
                          subtransactionCmdRef.subtransactionStruct.locktype,
                          subtransactionCmdRef.searchParameters,
                          msgref.returnRows);
            break;

        default:
            fprintf(logfile, "anomaly: %i %s %i\n",
                    subtransactionCmdRef.transactionStruct.transaction_enginecmd,
                    __FILE__, __LINE__);
        }

        replyTransaction((void *)msg);
    }
    break;

    case PAYLOADCOMMITROLLBACK:
    {
        class MessageCommitRollback &subtransactionCmdRef =
            *((class MessageCommitRollback *)msgrcv);
        class Table *tablePtr;
        class Index *indexPtr;

        for (size_t n=0; n<subtransactionCmdRef.rofs.size(); n++)
        {
            rowOrField_s &rowFieldRef = subtransactionCmdRef.rofs[n];

            if (!schemaPtr->tables.count(rowFieldRef.tableid))
            {
                printf("%s %i anomaly, barfing, tableid %i\n", __FILE__,
                       __LINE__, rowFieldRef.tableid);
                abort();
            }

            tablePtr = schemaPtr->tables[rowFieldRef.tableid];

            if (rowFieldRef.isrow==true)
            {
                tablePtr->commitRollbackUnlock(rowFieldRef.rowid,
                                               subtransactionid,
                                               (enginecmd_e) subtransactionCmdRef.transactionStruct.transaction_enginecmd);

                // drain lock queue if: sueccessful delete & commit, or insert &
                // rollback is a non-existent row sufficient to infer?
                // For now, yes,
                // but later, maybe not, if there's some other flag. Figure it
                // out at
                // that point.
                if (tablePtr->rows.count(rowFieldRef.rowid) &&
                    (enginecmd_e)subtransactionCmdRef.transactionStruct.transaction_enginecmd !=
                    REVERTCMD)
                {
                    processRowLockQueue(rowFieldRef.tableid,
                                        rowFieldRef.rowid);
                }
                else
                {
                    drainRowLockQueue(rowFieldRef.tableid,
                                      rowFieldRef.rowid);
                }
            }
            else if (rowFieldRef.isnotaddunique==false)
            {
                indexPtr = &tablePtr->fields[rowFieldRef.fieldid].index;

                if (rowFieldRef.fieldVal.isnull==true)
                {
                    printf("%s %i INSERTNULLENTRY unique index\n", __FILE__,
                           __LINE__);
                    indexPtr->insertNullEntry(rowFieldRef.rowid,
                                              rowFieldRef.engineid);
                }
                else
                {

                    switch (indexPtr->fieldtype)
                    {
                    case INT:
                        indexPtr->commitRollback(rowFieldRef.fieldVal.value.integer,
                                                 subtransactionid,
                                                 (enginecmd_e) subtransactionCmdRef.transactionStruct.transaction_enginecmd);
                        break;

                    case UINT:
                        indexPtr->commitRollback(rowFieldRef.fieldVal.value.uinteger,
                                                 subtransactionid,
                                                 (enginecmd_e) subtransactionCmdRef.transactionStruct.transaction_enginecmd);
                        break;

                    case BOOL:
                        indexPtr->commitRollback(rowFieldRef.fieldVal.value.boolean,
                                                 subtransactionid,
                                                 (enginecmd_e) subtransactionCmdRef.transactionStruct.transaction_enginecmd);
                        break;

                    case FLOAT:
                        indexPtr->commitRollback(rowFieldRef.fieldVal.value.floating,
                                                 subtransactionid,
                                                 (enginecmd_e) subtransactionCmdRef.transactionStruct.transaction_enginecmd);
                        break;

                    case CHAR:
                        indexPtr->commitRollback(rowFieldRef.fieldVal.value.character,
                                                 subtransactionid,
                                                 (enginecmd_e) subtransactionCmdRef.transactionStruct.transaction_enginecmd);
                        break;

                    case CHARX:
                        indexPtr->commitRollback(rowFieldRef.fieldVal.str,
                                                 subtransactionid,
                                                 (enginecmd_e) subtransactionCmdRef.transactionStruct.transaction_enginecmd);
                        break;

                    case VARCHAR:
                        indexPtr->commitRollback(rowFieldRef.fieldVal.str,
                                                 subtransactionid,
                                                 (enginecmd_e) subtransactionCmdRef.transactionStruct.transaction_enginecmd);
                        break;

                    default:
                        fprintf(logfile, "anomaly: %i %s %i\n",
                                indexPtr->fieldtype, __FILE__, __LINE__);
                    }
                }

                if ((enginecmd_e)subtransactionCmdRef.transactionStruct.transaction_enginecmd == COMMITCMD)
                {
                    drainIndexLockQueue(rowFieldRef.tableid, rowFieldRef.fieldid,
                                        &rowFieldRef.fieldVal);
                }
                else if ((enginecmd_e)subtransactionCmdRef.transactionStruct.transaction_enginecmd == ROLLBACKCMD)
                {
                    processIndexLockQueue(rowFieldRef.tableid,
                                          rowFieldRef.fieldid,
                                          &rowFieldRef.fieldVal);
                }
            }
            else     // is not addunique index
            {
                // possibilities are: add null, delete null,
                // add nonunique, delete nonunique, delete unique, replace
                indexPtr = &tablePtr->fields[rowFieldRef.fieldid].index;

                if (rowFieldRef.isreplace==true)
                {
                    if (rowFieldRef.fieldVal.isnull==true)
                    {
                        indexPtr->replaceNull(rowFieldRef.rowid,
                                              rowFieldRef.engineid,
                                              rowFieldRef.newrowid,
                                              rowFieldRef.engineid);
                    }
                    else
                    {
                        switch (indexPtr->fieldtype)
                        {
                        case INT:
                            if (indexPtr->isunique==true)
                            {
                                indexPtr->replaceUnique(rowFieldRef.newrowid,
                                                        rowFieldRef.newengineid,
                                                        rowFieldRef.fieldVal.value.integer);
                            }
                            else
                            {
                                indexPtr->replaceNonunique(rowFieldRef.rowid,
                                                           rowFieldRef.engineid,
                                                           rowFieldRef.newrowid,
                                                           rowFieldRef.newengineid,
                                                           rowFieldRef.fieldVal.value.integer);
                            }

                            break;

                        case UINT:
                            if (indexPtr->isunique==true)
                            {
                                indexPtr->replaceUnique(rowFieldRef.newrowid,
                                                        rowFieldRef.newengineid,
                                                        rowFieldRef.fieldVal.value.uinteger);
                            }
                            else
                            {
                                indexPtr->replaceNonunique(rowFieldRef.rowid,
                                                           rowFieldRef.engineid,
                                                           rowFieldRef.newrowid,
                                                           rowFieldRef.newengineid,
                                                           rowFieldRef.fieldVal.value.uinteger);
                            }

                            break;

                        case BOOL:
                            if (indexPtr->isunique==true)
                            {
                                indexPtr->replaceUnique(rowFieldRef.newrowid,
                                                        rowFieldRef.newengineid,
                                                        rowFieldRef.fieldVal.value.boolean);
                            }
                            else
                            {
                                indexPtr->replaceNonunique(rowFieldRef.rowid,
                                                           rowFieldRef.engineid,
                                                           rowFieldRef.newrowid,
                                                           rowFieldRef.newengineid,
                                                           rowFieldRef.fieldVal.value.boolean);
                            }

                            break;

                        case FLOAT:
                            if (indexPtr->isunique==true)
                            {
                                indexPtr->replaceUnique(rowFieldRef.newrowid,
                                                        rowFieldRef.newengineid,
                                                        rowFieldRef.fieldVal.value.floating);
                            }
                            else
                            {
                                indexPtr->replaceNonunique(rowFieldRef.rowid,
                                                           rowFieldRef.engineid,
                                                           rowFieldRef.newrowid,
                                                           rowFieldRef.newengineid,
                                                           rowFieldRef.fieldVal.value.floating);
                            }

                            break;

                        case CHAR:
                            if (indexPtr->isunique==true)
                            {
                                indexPtr->replaceUnique(rowFieldRef.newrowid,
                                                        rowFieldRef.newengineid,
                                                        rowFieldRef.fieldVal.value.character);
                            }
                            else
                            {
                                indexPtr->replaceNonunique(rowFieldRef.rowid,
                                                           rowFieldRef.engineid,
                                                           rowFieldRef.newrowid,
                                                           rowFieldRef.newengineid,
                                                           rowFieldRef.fieldVal.value.character);
                            }

                            break;

                        case CHARX:
                            if (indexPtr->isunique==true)
                            {
                                indexPtr->replaceUnique(rowFieldRef.newrowid,
                                                        rowFieldRef.newengineid,
                                                        rowFieldRef.fieldVal.str);
                            }
                            else
                            {
                                indexPtr->replaceNonunique(rowFieldRef.rowid,
                                                           rowFieldRef.engineid,
                                                           rowFieldRef.newrowid,
                                                           rowFieldRef.newengineid,
                                                           rowFieldRef.fieldVal.str);
                            }

                            break;

                        case VARCHAR:
                            if (indexPtr->isunique==true)
                            {
                                indexPtr->replaceUnique(rowFieldRef.newrowid,
                                                        rowFieldRef.newengineid,
                                                        rowFieldRef.fieldVal.str);
                            }
                            else
                            {
                                indexPtr->replaceNonunique(rowFieldRef.rowid,
                                                           rowFieldRef.engineid,
                                                           rowFieldRef.newrowid,
                                                           rowFieldRef.newengineid,
                                                           rowFieldRef.fieldVal.str);
                            }

                            break;

                        default:
                            fprintf(logfile, "anomaly: %i %s %i\n",
                                    indexPtr->fieldtype, __FILE__, __LINE__);
                        }
                    }

                    continue;
                }

                if (rowFieldRef.fieldVal.isnull==true)
                {
                    if (rowFieldRef.deleteindexentry==true)
                    {
                        indexPtr->deleteNullEntry(rowFieldRef.rowid,
                                                  rowFieldRef.engineid);
                    }
                    else
                    {
                        indexPtr->insertNullEntry(rowFieldRef.rowid,
                                                  rowFieldRef.engineid);
                    }

                    continue;
                }

                switch (indexPtr->fieldtype)
                {
                case INT:
                    if (indexPtr->isunique==true)   // delete unique
                    {
                        indexPtr->deleteUniqueEntry(rowFieldRef.fieldVal.value.integer);
                        // process lock queue
                        processIndexLockQueue(rowFieldRef.tableid,
                                              rowFieldRef.fieldid,
                                              &rowFieldRef.fieldVal);
                    }
                    else if (rowFieldRef.deleteindexentry==true) // delete non-unique
                    {
                        indexPtr->deleteNonuniqueEntry(rowFieldRef.fieldVal.value.integer, rowFieldRef.rowid, rowFieldRef.engineid);
                    }
                    else     // add non-unique
                    {
                        indexPtr->insertNonuniqueEntry(rowFieldRef.fieldVal.value.integer, rowFieldRef.rowid, rowFieldRef.engineid);
                    }

                    break;

                case UINT:
                    if (indexPtr->isunique==true)   // delete unique
                    {
                        indexPtr->deleteUniqueEntry(rowFieldRef.fieldVal.value.
                                                    uinteger);
                        // process lock queue
                        processIndexLockQueue(rowFieldRef.tableid,
                                              rowFieldRef.fieldid,
                                              &rowFieldRef.fieldVal);
                    }
                    else if (rowFieldRef.deleteindexentry==true) // delete non-unique
                    {
                        indexPtr->deleteNonuniqueEntry(rowFieldRef.fieldVal.value.uinteger, rowFieldRef.rowid, rowFieldRef.engineid);
                    }
                    else     // add non-unique
                    {
                        indexPtr->insertNonuniqueEntry(rowFieldRef.fieldVal.value.uinteger, rowFieldRef.rowid, rowFieldRef.engineid);
                    }

                    break;

                case BOOL:
                    if (indexPtr->isunique==true)   // delete unique
                    {
                        indexPtr->deleteUniqueEntry(rowFieldRef.fieldVal.value.boolean);
                        // process lock queue
                        processIndexLockQueue(rowFieldRef.tableid,
                                              rowFieldRef.fieldid,
                                              &rowFieldRef.fieldVal);
                    }
                    else if (rowFieldRef.deleteindexentry==true) // delete non-unique
                    {
                        indexPtr->deleteNonuniqueEntry(rowFieldRef.fieldVal.value.boolean, rowFieldRef.rowid, rowFieldRef.engineid);
                    }
                    else     // add non-unique
                    {
                        indexPtr->insertNonuniqueEntry(rowFieldRef.fieldVal.value.boolean, rowFieldRef.rowid, rowFieldRef.engineid);
                    }

                    break;

                case FLOAT:
                    if (indexPtr->isunique==true)   // delete unique
                    {
                        indexPtr->deleteUniqueEntry(rowFieldRef.fieldVal.value.
                                                    floating);
                        // process lock queue
                        processIndexLockQueue(rowFieldRef.tableid,
                                              rowFieldRef.fieldid,
                                              &rowFieldRef.fieldVal);
                    }
                    else if (rowFieldRef.deleteindexentry==true) // delete non-unique
                    {
                        indexPtr->deleteNonuniqueEntry(rowFieldRef.fieldVal.value.floating, rowFieldRef.rowid, rowFieldRef.engineid);
                    }
                    else     // add non-unique
                    {
                        indexPtr->insertNonuniqueEntry(rowFieldRef.fieldVal.value.floating, rowFieldRef.rowid, rowFieldRef.engineid);
                    }

                    break;

                case CHAR:
                    if (indexPtr->isunique==true)   // delete unique
                    {
                        indexPtr->deleteUniqueEntry(rowFieldRef.fieldVal.value.character);
                        // process lock queue
                        processIndexLockQueue(rowFieldRef.tableid,
                                              rowFieldRef.fieldid,
                                              &rowFieldRef.fieldVal);
                    }
                    else if (rowFieldRef.deleteindexentry==true) // delete non-unique
                    {
                        indexPtr->deleteNonuniqueEntry(rowFieldRef.fieldVal.value.character, rowFieldRef.rowid, rowFieldRef.engineid);
                    }
                    else     // add non-unique
                    {
                        indexPtr->insertNonuniqueEntry(rowFieldRef.fieldVal.value.character, rowFieldRef.rowid, rowFieldRef.engineid);
                    }

                    break;

                case CHARX:
                    if (indexPtr->isunique==true)   // delete unique
                    {
                        indexPtr->deleteUniqueEntry(&rowFieldRef.fieldVal.str);
                        // process lock queue
                        processIndexLockQueue(rowFieldRef.tableid,
                                              rowFieldRef.fieldid,
                                              &rowFieldRef.fieldVal);
                    }
                    else if (rowFieldRef.deleteindexentry==true) // delete non-unique
                    {
                        indexPtr->deleteNonuniqueEntry(&rowFieldRef.fieldVal.str,
                                                       rowFieldRef.rowid,
                                                       rowFieldRef.engineid);
                    }
                    else     // add non-unique
                    {
                        indexPtr->insertNonuniqueEntry(&rowFieldRef.fieldVal.str,
                                                       rowFieldRef.rowid,
                                                       rowFieldRef.engineid);
                    }

                    break;

                case VARCHAR:
                    if (indexPtr->isunique==true)   // delete unique
                    {
                        indexPtr->deleteUniqueEntry(&rowFieldRef.fieldVal.str);
                        // process lock queue
                        processIndexLockQueue(rowFieldRef.tableid,
                                              rowFieldRef.fieldid,
                                              &rowFieldRef.fieldVal);
                    }
                    else if (rowFieldRef.deleteindexentry==true) // delete non-unique
                    {
                        indexPtr->deleteNonuniqueEntry(&rowFieldRef.fieldVal.str,
                                                       rowFieldRef.rowid,
                                                       rowFieldRef.engineid);
                    }
                    else     // add non-unique
                    {
                        indexPtr->insertNonuniqueEntry(&rowFieldRef.fieldVal.str,
                                                       rowFieldRef.rowid,
                                                       rowFieldRef.engineid);
                    }

                    break;

                default:
                    fprintf(logfile, "anomaly: %i %s %i\n", indexPtr->fieldtype,
                            __FILE__, __LINE__);
                }
            }
        }

        // rollback & unlock are fire and forget
        if ((enginecmd_e)subtransactionCmdRef.transactionStruct.transaction_enginecmd == COMMITCMD)
        {
            replyTransaction((void *) new class MessageCommitRollback);
        }
    }
    break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n",
                msgrcv->messageStruct.payloadtype, __FILE__, __LINE__);
    }
}

locktype_e SubTransaction::uniqueIndex(int64_t tableid, int64_t fieldid,
                                       int64_t rowid, int64_t engineid,
                                       fieldValue_s *val)
{
    class MessageSubtransactionCmd &subtransactionCmdRef =
        *((class MessageSubtransactionCmd *)msgrcv);
    lockQueueIndexEntry queueEntry = {};
    queueEntry.pendingcmdid =
        subtransactionCmdRef.transactionStruct.transaction_pendingcmdid;
    queueEntry.tacmdentrypoint =
        subtransactionCmdRef.transactionStruct.transaction_tacmdentrypoint;
    queueEntry.entry.engineid = engineid;
    queueEntry.entry.rowid = rowid;
    queueEntry.entry.subtransactionid = subtransactionid;

    class Index &indexRef = schemaPtr->tables[tableid]->fields[fieldid].index;

    switch (indexRef.indexmaptype)
    {
    case uniqueint:
    {
        if (!indexRef.uniqueIntIndex->count(val->value.integer))
        {
            // there is no entry, so stage one and lock it
            indexRef.uniqueIntIndex->operator [](val->value.integer) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.uniqueIntIndex->at(val->value.integer).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.intLockQueue->operator [](val->value.integer).push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked: constraint violation
        return NOLOCK;
    }
    break;

    case unorderedint:
    {
        if (!indexRef.unorderedIntIndex->count(val->value.integer))
        {
            // there is no entry, so stage one and lock it
            indexRef.unorderedIntIndex->operator [](val->value.integer) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.unorderedIntIndex->at(val->value.integer).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.intLockQueue->operator [](val->value.integer).push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked: constraint violation
        return NOLOCK;
    }
    break;

    case uniqueuint:
    {
        if (!indexRef.uniqueUintIndex->count(val->value.uinteger))
        {
            // there is no entry, so stage one and lock it
            indexRef.uniqueUintIndex->operator [](val->value.uinteger) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.uniqueUintIndex->at(val->value.uinteger).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.uintLockQueue->operator [](val->value.uinteger).
                push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked, so that means constraint violation
        return NOLOCK;
    }
    break;

    case unordereduint:
    {
        if (!indexRef.unorderedUintIndex->count(val->value.uinteger))
        {
            // there is no entry, so stage one and lock it
            indexRef.unorderedUintIndex->operator [](val->value.uinteger) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.unorderedUintIndex->at(val->value.uinteger).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.uintLockQueue->operator [](val->value.uinteger).push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked: constraint violation
        return NOLOCK;
    }
    break;

    case uniquebool:
    {
        if (!indexRef.uniqueBoolIndex->count(val->value.boolean))
        {
            // there is no entry, so stage one and lock it
            indexRef.uniqueBoolIndex->operator [](val->value.boolean) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.uniqueBoolIndex->at(val->value.boolean).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.boolLockQueue->operator [](val->value.boolean).push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked: constraint violation
        return NOLOCK;
    }
    break;

    case unorderedbool:
    {
        if (!indexRef.unorderedBoolIndex->count(val->value.boolean))
        {
            // there is no entry, so stage one and lock it
            indexRef.unorderedBoolIndex->operator [](val->value.boolean) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.unorderedBoolIndex->at(val->value.boolean).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.boolLockQueue->operator [](val->value.boolean).
                push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked, so that means constraint violation
        return NOLOCK;
    }
    break;

    case uniquefloat:
    {
        if (!indexRef.uniqueFloatIndex->count(val->value.floating))
        {
            // there is no entry, so stage one and lock it
            indexRef.uniqueFloatIndex->operator [](val->value.floating) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.uniqueFloatIndex->at(val->value.floating).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.floatLockQueue->operator [](val->value.floating).
                push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked: constraint violation
        printf("%s %i NOLOCK float %Lf\n", __FILE__, __LINE__,
               val->value.floating);
        return NOLOCK;
    }
    break;

    case unorderedfloat:
    {
        if (!indexRef.unorderedFloatIndex->count(val->value.floating))
        {
            // there is no entry, so stage one and lock it
            indexRef.unorderedFloatIndex->operator [](val->value.floating) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.unorderedFloatIndex->at(val->value.floating).
            subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.floatLockQueue->operator [](val->value.floating).push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked: constraint violation
        return NOLOCK;
    }
    break;

    case uniquechar:
    {
        if (!indexRef.uniqueCharIndex->count(val->value.character))
        {
            // there is no entry, so stage one and lock it
            indexRef.uniqueCharIndex->operator [](val->value.character) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.uniqueCharIndex->at(val->value.character).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.charLockQueue->operator [](val->value.character).
                push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked: constraint violation
        printf("%s %i NOLOCK char %c\n", __FILE__, __LINE__,
               val->value.character);
        return NOLOCK;
    }
    break;

    case unorderedchar:
    {
        if (!indexRef.unorderedCharIndex->count(val->value.character))
        {
            // there is no entry, so stage one and lock it
            indexRef.unorderedCharIndex->operator [](val->value.character) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.unorderedCharIndex->at(val->value.character).
            subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.charLockQueue->operator [](val->value.character).
                push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked, so that means constraint
        // violation
        return NOLOCK;
    }
    break;

    case uniquecharx:
    {
        trimspace(val->str);

        if (!indexRef.uniqueStringIndex->count(val->str))
        {
            // there is no entry, so stage one and lock it
            indexRef.uniqueStringIndex->operator [](val->str) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.uniqueStringIndex->at(val->str).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.stringLockQueue->operator [](val->str).push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked: constraint violation
        printf("%s %i NOLOCK charx '%s'\n", __FILE__, __LINE__,
               val->str.c_str());
        return NOLOCK;
    }
    break;

    case unorderedcharx:
    {
        if (!indexRef.unorderedStringIndex->count(val->str))
        {
            // there is no entry, so stage one and lock it
            indexRef.unorderedStringIndex->operator [](val->str) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.unorderedStringIndex->at(val->str).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.stringLockQueue->operator [](val->str).push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked, so that means constraint
        // violation
        return NOLOCK;
    }
    break;

    case uniquevarchar:
    {
        trimspace(val->str);

        if (!indexRef.uniqueStringIndex->count(val->str))
        {
            // there is no entry, so stage one and lock it
            indexRef.uniqueStringIndex->operator [](val->str) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.uniqueStringIndex->at(val->str).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.stringLockQueue->operator [](val->str).push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked: constraint violation
        printf("%s %i NOLOCK varchar '%s'\n", __FILE__, __LINE__,
               val->str.c_str());
        return NOLOCK;
    }
    break;

    case unorderedvarchar:
    {
        if (!indexRef.unorderedStringIndex->count(val->str))
        {
            // there is no entry, so stage one and lock it
            indexRef.unorderedStringIndex->operator [](val->str) =
                queueEntry.entry;
            return INDEXLOCK;
        }

        if (indexRef.unorderedStringIndex->at(val->str).subtransactionid)
        {
            // there is already a staged, locked entry, so go into the lock queue
            indexRef.stringLockQueue->operator [](val->str).push(queueEntry);
            return INDEXPENDINGLOCK;
        }

        // there is an entry that is not locked, so that means constraint
        // violation
        return NOLOCK;
    }
    break;

    default:
        printf("%s %i anomaly %i\n", __FILE__, __LINE__, indexRef.indexmaptype);
    }

    return NOTFOUNDLOCK;
}

void SubTransaction::indexSearch(int64_t tableid, int64_t fieldid,
                                 searchParams_s *searchParameters,
                                 vector<nonLockingIndexEntry_s> *indexHits)
{
    class Index &indexRef = schemaPtr->tables[tableid]->fields[fieldid].index;
    // for the various operation types, also field types. 11 ops, 7 field types

    switch (searchParameters->op)
    {
    case OPERATOR_EQ:
    {
        switch (indexRef.fieldtype)
        {
        case INT:
        {
            indexRef.getequal(searchParameters->values[0].value.integer,
                              indexHits);
        }
        break;

        case UINT:
        {
            indexRef.getequal(searchParameters->values[0].value.uinteger,
                              indexHits);
        }
        break;

        case BOOL:
        {
            indexRef.getequal(searchParameters->values[0].value.boolean,
                              indexHits);
        }
        break;

        case FLOAT:
        {
            indexRef.getequal(searchParameters->values[0].value.floating,
                              indexHits);
        }
        break;

        case CHAR:
        {
            indexRef.getequal(searchParameters->values[0].value.character,
                              indexHits);
        }
        break;

        case CHARX:
        {
            indexRef.getequal(searchParameters->values[0].str,
                              indexHits);
        }
        break;

        case VARCHAR:
        {
            indexRef.getequal(searchParameters->values[0].str,
                              indexHits);
        }
        break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                    __LINE__);
        }

    }
    break;

    case OPERATOR_NE:
    {
        switch (indexRef.fieldtype)
        {
        case INT:
        {
            indexRef.getnotequal(searchParameters->values[0].value.integer,
                                 indexHits);
        }
        break;

        case UINT:
        {
            indexRef.getnotequal(searchParameters->values[0].value.uinteger,
                                 indexHits);
        }
        break;

        case BOOL:
        {
            indexRef.getnotequal(searchParameters->values[0].value.boolean,
                                 indexHits);
        }
        break;

        case FLOAT:
        {
            indexRef.getnotequal(searchParameters->values[0].value.floating,
                                 indexHits);
        }
        break;

        case CHAR:
        {
            indexRef.getnotequal(searchParameters->values[0].value.character,
                                 indexHits);
        }
        break;

        case CHARX:
        {
            indexRef.getnotequal(searchParameters->values[0].str,
                                 indexHits);
        }
        break;

        case VARCHAR:
        {
            indexRef.getnotequal(searchParameters->values[0].str,
                                 indexHits);
        }
        break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                    __LINE__);
        }
    }
    break;

    case OPERATOR_GT:
    {
        switch (indexRef.fieldtype)
        {
        case INT:
        {
            indexRef.comparison(searchParameters->values[0].value.integer,
                                searchParameters->op, indexHits);
        }
        break;

        case UINT:
        {
            indexRef.comparison(searchParameters->values[0].value.uinteger,
                                searchParameters->op, indexHits);
        }
        break;

        case BOOL:
        {
            indexRef.comparison(searchParameters->values[0].value.boolean,
                                searchParameters->op, indexHits);
        }
        break;

        case FLOAT:
        {
            indexRef.comparison(searchParameters->values[0].value.floating,
                                searchParameters->op, indexHits);
        }
        break;

        case CHAR:
        {
            indexRef.comparison(searchParameters->values[0].value.character,
                                searchParameters->op, indexHits);
        }
        break;

        case CHARX:
        {
            indexRef.comparison(&searchParameters->values[0].str,
                                searchParameters->op, indexHits);
        }
        break;

        case VARCHAR:
        {
            indexRef.comparison(&searchParameters->values[0].str,
                                searchParameters->op, indexHits);
        }
        break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                    __LINE__);
        }

    }
    break;

    case OPERATOR_LT:
    {
        switch (indexRef.fieldtype)
        {
        case INT:
        {
            indexRef.comparison(searchParameters->values[0].value.integer,
                                searchParameters->op, indexHits);
        }
        break;

        case UINT:
        {
            indexRef.comparison(searchParameters->values[0].value.uinteger,
                                searchParameters->op, indexHits);
        }
        break;

        case BOOL:
        {
            indexRef.comparison(searchParameters->values[0].value.boolean,
                                searchParameters->op, indexHits);
        }
        break;

        case FLOAT:
        {
            indexRef.comparison(searchParameters->values[0].value.floating,
                                searchParameters->op, indexHits);
        }
        break;

        case CHAR:
        {
            indexRef.comparison(searchParameters->values[0].value.character,
                                searchParameters->op, indexHits);
        }
        break;

        case CHARX:
        {
            indexRef.comparison(&searchParameters->values[0].str,
                                searchParameters->op, indexHits);
        }
        break;

        case VARCHAR:
        {
            indexRef.comparison(&searchParameters->values[0].str,
                                searchParameters->op, indexHits);
        }
        break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                    __LINE__);
        }

    }
    break;

    case OPERATOR_GTE:
    {
        switch (indexRef.fieldtype)
        {
        case INT:
        {
            indexRef.comparison(searchParameters->values[0].value.integer,
                                searchParameters->op, indexHits);
        }
        break;

        case UINT:
        {
            indexRef.comparison(searchParameters->values[0].value.uinteger,
                                searchParameters->op, indexHits);
        }
        break;

        case BOOL:
        {
            indexRef.comparison(searchParameters->values[0].value.boolean,
                                searchParameters->op, indexHits);
        }
        break;

        case FLOAT:
        {
            indexRef.comparison(searchParameters->values[0].value.floating,
                                searchParameters->op, indexHits);
        }
        break;

        case CHAR:
        {
            indexRef.comparison(searchParameters->values[0].value.character,
                                searchParameters->op, indexHits);
        }
        break;

        case CHARX:
        {
            indexRef.comparison(&searchParameters->values[0].str,
                                searchParameters->op, indexHits);
        }
        break;

        case VARCHAR:
        {
            indexRef.comparison(&searchParameters->values[0].str,
                                searchParameters->op, indexHits);
        }
        break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                    __LINE__);
        }

    }
    break;

    case OPERATOR_LTE:
    {
        switch (indexRef.fieldtype)
        {
        case INT:
        {
            indexRef.comparison(searchParameters->values[0].value.integer,
                                searchParameters->op, indexHits);
        }
        break;

        case UINT:
        {
            indexRef.comparison(searchParameters->values[0].value.uinteger,
                                searchParameters->op, indexHits);
        }
        break;

        case BOOL:
        {
            indexRef.comparison(searchParameters->values[0].value.boolean,
                                searchParameters->op, indexHits);
        }
        break;

        case FLOAT:
        {
            indexRef.comparison(searchParameters->values[0].value.floating,
                                searchParameters->op, indexHits);
        }
        break;

        case CHAR:
        {
            indexRef.comparison(searchParameters->values[0].value.character,
                                searchParameters->op, indexHits);
        }
        break;

        case CHARX:
        {
            indexRef.comparison(&searchParameters->values[0].str,
                                searchParameters->op, indexHits);
        }
        break;

        case VARCHAR:
        {
            indexRef.comparison(&searchParameters->values[0].str,
                                searchParameters->op, indexHits);
        }
        break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                    __LINE__);
        }

    }
    break;

    case OPERATOR_IN:
    {
        switch (indexRef.fieldtype)
        {
        case INT:
        {
            vector<int64_t> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].value.integer;
            }

            indexRef.getin(&v, indexHits);
        }
        break;

        case UINT:
        {
            vector<uint64_t> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].value.uinteger;
            }

            indexRef.getin(&v, indexHits);
        }
        break;

        case BOOL:
        {
            vector<bool> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].value.boolean;
            }

            indexRef.getin(&v, indexHits);
        }
        break;

        case FLOAT:
        {
            vector<long double> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].value.floating;
            }

            indexRef.getin(&v, indexHits);
        }
        break;

        case CHAR:
        {
            vector<char> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].value.character;
            }

            indexRef.getin(&v, indexHits);
        }
        break;

        case CHARX:
        {
            vector<string> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].str;
            }

            indexRef.getin(&v, indexHits);
        }
        break;

        case VARCHAR:
        {
            vector<string> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].str;
            }

            indexRef.getin(&v, indexHits);
        }
        break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                    __LINE__);
        }
    }
    break;

    case OPERATOR_NOTIN:
    {
        switch (indexRef.fieldtype)
        {
        case INT:
        {
            vector<int64_t> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].value.integer;
            }

            indexRef.getnotin(v, indexHits);
        }
        break;

        case UINT:
        {
            vector<uint64_t> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].value.uinteger;
            }

            indexRef.getnotin(v, indexHits);
        }
        break;

        case BOOL:
        {
            vector<bool> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].value.boolean;
            }

            indexRef.getnotin(v, indexHits);
        }
        break;

        case FLOAT:
        {
            vector<long double> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].value.floating;
            }

            indexRef.getnotin(v, indexHits);
        }
        break;

        case CHAR:
        {
            vector<char> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].value.character;
            }

            indexRef.getnotin(v, indexHits);
        }
        break;

        case CHARX:
        {
            vector<string> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].str;
            }

            indexRef.getnotin(v, indexHits);
        }
        break;

        case VARCHAR:
        {
            vector<string> v(searchParameters->values.size());

            for (size_t n=0; n < searchParameters->values.size(); n++)
            {
                v[n] = searchParameters->values[n].str;
            }

            indexRef.getnotin(v, indexHits);
        }
        break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                    __LINE__);
        }
    }
    break;

    case OPERATOR_BETWEEN:
    {
        switch (indexRef.fieldtype)
        {
        case INT:
        {
            indexRef.between(searchParameters->values[0].value.integer,
                             searchParameters->values[1].value.integer,
                             indexHits);
        }
        break;

        case UINT:
        {
            indexRef.between(searchParameters->values[0].value.uinteger,
                             searchParameters->values[1].value.uinteger,
                             indexHits);
        }
        break;

        case BOOL:
        {
            indexRef.between(searchParameters->values[0].value.boolean,
                             searchParameters->values[1].value.boolean,
                             indexHits);
        }
        break;

        case FLOAT:
        {
            indexRef.between(searchParameters->values[0].value.floating,
                             searchParameters->values[1].value.floating,
                             indexHits);
        }
        break;

        case CHAR:
        {
            indexRef.between(searchParameters->values[0].value.character,
                             searchParameters->values[1].value.character,
                             indexHits);
        }
        break;

        case CHARX:
        {
            indexRef.between(searchParameters->values[0].str,
                             searchParameters->values[1].str,
                             indexHits);
        }
        break;

        case VARCHAR:
        {
            indexRef.between(searchParameters->values[0].str,
                             searchParameters->values[1].str, indexHits);
        }
        break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                    __LINE__);
        }

    }
    break;

    case OPERATOR_NOTBETWEEN:
    {
        switch (indexRef.fieldtype)
        {
        case INT:
        {
            indexRef.notbetween(searchParameters->values[0].value.integer,
                                searchParameters->values[1].value.integer,
                                indexHits);
        }
        break;

        case UINT:
        {
            indexRef.notbetween(searchParameters->values[0].value.uinteger,
                                searchParameters->values[1].value.uinteger,
                                indexHits);
        }
        break;

        case BOOL:
        {
            indexRef.notbetween(searchParameters->values[0].value.boolean,
                                searchParameters->values[1].value.boolean,
                                indexHits);
        }
        break;

        case FLOAT:
        {
            indexRef.notbetween(searchParameters->values[0].value.floating,
                                searchParameters->values[1].value.floating,
                                indexHits);
        }
        break;

        case CHAR:
        {
            indexRef.notbetween(searchParameters->values[0].value.character,
                                searchParameters->values[1].value.character,
                                indexHits);
        }
        break;

        case CHARX:
        {
            indexRef.notbetween(&searchParameters->values[0].str,
                                &searchParameters->values[1].str, indexHits);
        }
        break;

        case VARCHAR:
        {
            indexRef.notbetween(&searchParameters->values[0].str,
                                &searchParameters->values[1].str, indexHits);
        }
        break;

        default:
            fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                    __LINE__);
        }

    }
    break;

    case OPERATOR_ISNULL:
        indexRef.getnulls(indexHits);
        break;

    case OPERATOR_ISNOTNULL:
        indexRef.getnotnulls(indexHits);
        break;

    case OPERATOR_SELECTALL: // combination of nulls and not nulls
        indexRef.getall(indexHits);
        break;

    case OPERATOR_REGEX:
        indexRef.regex(&searchParameters->regexString, indexHits);
        break;

    case OPERATOR_LIKE:
        indexRef.like(searchParameters->values[0].str, indexHits);
        break;

    case OPERATOR_NOTLIKE:
        indexRef.notlike(searchParameters->values[0].str, indexHits);
        break;

    default:
        fprintf(logfile, "anomaly %i %s %i\n", searchParameters->op, __FILE__,
                __LINE__);
    }
}

void SubTransaction::commitRollbackUnlock(vector<rowOrField_s> *rowOrFields,
                                          enginecmd_e cmd)
{
    // remember to process lock queue after each item
    for (size_t n=0; n<rowOrFields->size(); n++)
    {
        rowOrField_s &rowOrFieldRef = rowOrFields->at(n);
        class Table &tableRef = *schemaPtr->tables[rowOrFieldRef.tableid];

        if (rowOrFieldRef.isrow==true)
        {
            tableRef.commitRollbackUnlock(rowOrFieldRef.rowid, subtransactionid,
                                          COMMITCMD);
        }
        else
        {
            class Index &indexRef = tableRef.fields[rowOrFieldRef.fieldid].index;

            switch (indexRef.fieldtype)
            {
            case INT:
                indexRef.commitRollback(rowOrFieldRef.fieldVal.value.integer,
                                        subtransactionid, cmd);

            case UINT:
                indexRef.commitRollback(rowOrFieldRef.fieldVal.value.uinteger,
                                        subtransactionid, cmd);

            case BOOL:
                indexRef.commitRollback(rowOrFieldRef.fieldVal.value.boolean,
                                        subtransactionid, cmd);

            case FLOAT:
                indexRef.commitRollback(rowOrFieldRef.fieldVal.value.floating,
                                        subtransactionid, cmd);

            case CHAR:
                indexRef.commitRollback(rowOrFieldRef.fieldVal.value.character,
                                        subtransactionid, cmd);

            case CHARX:
                indexRef.commitRollback(rowOrFieldRef.fieldVal.str,
                                        subtransactionid, cmd);

            case VARCHAR:
                indexRef.commitRollback(rowOrFieldRef.fieldVal.str,
                                        subtransactionid, cmd);
                break;

            default:
                fprintf(logfile, "anomaly %i %s %i\n", indexRef.fieldtype, __FILE__,
                        __LINE__);
            }
        }
    }
}

void SubTransaction::replyTransaction(void *data)
{
    class MessageTransaction &msgref = *(class MessageTransaction *)data;
    class MessageTransaction &msgrcvRef =
        *(class MessageTransaction *)msgrcv;
    msgref.messageStruct.topic = TOPIC_TRANSACTION;
    msgref.messageStruct.payloadtype = msgrcvRef.messageStruct.payloadtype;
    msgref.transactionStruct.transactionid =
        msgrcvRef.transactionStruct.transactionid;
    msgref.transactionStruct.subtransactionid = subtransactionid;
    msgref.transactionStruct.engineinstance = enginePtr->partitionid;
    msgref.transactionStruct.transaction_tacmdentrypoint =
        msgrcvRef.transactionStruct.transaction_tacmdentrypoint;
    msgref.transactionStruct.transaction_pendingcmdid =
        msgrcvRef.transactionStruct.transaction_pendingcmdid;

    enginePtr->mboxes.toActor(enginePtr->myIdentity.address, taAddr,
                              *((class Message *)data));
}

// for drain queue old transactions
void SubTransaction::replyTransaction(class MessageTransaction &sndRef,
                                      class MessageTransaction &rcvRef)
{
    sndRef.messageStruct.topic = TOPIC_TRANSACTION;
    sndRef.messageStruct.payloadtype = rcvRef.messageStruct.payloadtype;
    sndRef.messageStruct.payloadtype = rcvRef.messageStruct.payloadtype;
    sndRef.transactionStruct.transactionid =
        rcvRef.transactionStruct.transactionid;
    sndRef.transactionStruct.subtransactionid = subtransactionid;
    sndRef.transactionStruct.engineinstance = enginePtr->partitionid;
    sndRef.transactionStruct.transaction_tacmdentrypoint =
        rcvRef.transactionStruct.transaction_tacmdentrypoint;
    sndRef.transactionStruct.transaction_pendingcmdid =
        rcvRef.transactionStruct.transaction_pendingcmdid;

    enginePtr->mboxes.toActor(enginePtr->myIdentity.address, taAddr, sndRef);
}

// to be called whenever the row is unlocked
void SubTransaction::procesRowLockQueue(int64_t tableid, int64_t rowid)
{
    return;
    // class Table &tableRef = *schemaPtr->tables[tableid];
    // std::queue<lockQueueRowEntry> &qRef = tableRef.lockQueue[rowid];
    // //    subtransactionCmd *subtransactionCmdPtr;
    // class MessageSubtransactionCmd *subtransactionCmdPtr;

    // bool readlockedflag=false;

    // while (!qRef.empty())   // remember to return when a WRITELOCK is processed
    // {
    //     lockQueueRowEntry &entry = qRef.front();

    //     if (enginePtr->SubTransactions.count(entry.subtransactionid))
    //     {
    //         // oh yeah gotta lock the row, but don't know how to stage the
    //         // row itself
    //         if (entry.locktype==WRITELOCK)
    //         {
    //             setwritelock(&tableRef.rows[rowid]->flags);
    //             tableRef.rows[rowid]->writelockHolder = entry.subtransactionid;

    //             subtransactionCmdPtr = new class MessageSubtransactionCmd();
    //             returnRow_s returnRow= {};
    //             returnRow.rowid=rowid;
    //             returnRow.locktype=PENDINGTOWRITELOCK;
    //             subtransactionCmdPtr->returnRows.push_back(returnRow);
    //         }
    //         else if (entry.locktype==READLOCK)
    //         {
    //             if (readlockedflag==false)
    //             {
    //                 setreadlock(&tableRef.rows[rowid]->flags);
    //                 tableRef.rows[rowid]->readlockHolders =
    //                     new boost::unordered_set<int64_t>;
    //                 readlockedflag=true;
    //             }

    //             tableRef.rows[rowid]->readlockHolders->insert(entry.subtransactionid);

    //             subtransactionCmdPtr = new class MessageSubtransactionCmd();
    //             subtransactionCmdPtr->returnRows[0].locktype = PENDINGTOREADLOCK;
    //             subtransactionCmdPtr->returnRows[0].rowid = rowid;
    //             subtransactionCmdPtr->returnRows[0].row = tableRef.rows[rowid]->row;
    //         }
    //         else
    //         {
    //             qRef.pop();
    //             fprintf(logfile, "anomaly: %i %s %i\n", entry.locktype, __FILE__,
    //                     __LINE__);
    //             continue;
    //         }

    //         // ok gotta fake msgrcv for replyTransaction
    //         class SubTransaction &subTransactionRef =
    //             *enginePtr->SubTransactions[entry.subtransactionid];
    //         class MessageSubtransactionCmd rcv;
    //         rcv.transactionStruct.transactionid = subTransactionRef.transactionid;
    //         rcv.transactionStruct.subtransactionid = entry.subtransactionid;
    //         rcv.transactionStruct.engineinstance = enginePtr->partitionid;
    //         rcv.transactionStruct.transaction_tacmdentrypoint = entry.tacmdentrypoint;
    //         rcv.transactionStruct.transaction_pendingcmdid = entry.pendingcmdid;
    //         rcv.messageStruct.payloadtype = PAYLOADSUBTRANSACTION;

    //         subTransactionRef.replyTransaction(*subtransactionCmdPtr, rcv);

    //         // there can only be 1 lock holder if it's a WRITELOCK
    //         if (entry.locktype==WRITELOCK)
    //         {
    //             qRef.pop();
    //             return;
    //         }
    //     }

    //     qRef.pop();
    // }
}

void SubTransaction::drainRowLockQueue(int64_t tableid, int64_t rowid)
{
    class Table &tableRef = *schemaPtr->tables[tableid];
    std::queue<lockQueueRowEntry> &qRef = tableRef.lockQueue[rowid];

    while (!qRef.empty())
    {
        lockQueueRowEntry &entry = qRef.front();

        if (enginePtr->SubTransactions.count(entry.subtransactionid))
        {
            class SubTransaction &subTransactionRef =
                *enginePtr->SubTransactions[entry.subtransactionid];

            class MessageSubtransactionCmd *subtransactionCmdPtr =
                new MessageSubtransactionCmd();
            subtransactionCmdPtr->returnRows[0].locktype = PENDINGTONOLOCK;
            subtransactionCmdPtr->returnRows[0].rowid = rowid;
            // ok gotta fake msgrcv for replyTransaction
            class MessageSubtransactionCmd rcv;
            rcv.transactionStruct.transactionid =
                subTransactionRef.transactionid;
            rcv.transactionStruct.subtransactionid = entry.subtransactionid;
            rcv.transactionStruct.engineinstance = enginePtr->partitionid;
            rcv.transactionStruct.transaction_tacmdentrypoint =
                entry.tacmdentrypoint;
            rcv.transactionStruct.transaction_pendingcmdid = entry.pendingcmdid;
            rcv.messageStruct.payloadtype = PAYLOADSUBTRANSACTION;

            subTransactionRef.replyTransaction(*subtransactionCmdPtr, rcv);
        }

        qRef.pop();
    }
}

void SubTransaction::processIndexLockQueue(int64_t tableid, int64_t fieldid,
                                           fieldValue_s *val)
{
    return;
    // class Index &indexRef = schemaPtr->tables[tableid]->fields[fieldid].index;
    // std::queue<lockQueueIndexEntry> *lockQueuePtr = NULL;

    // switch (indexRef.fieldtype)
    // {
    // case INT:
    //     if (indexRef.intLockQueue->count(val->value.integer))
    //     {
    //         lockQueuePtr = &indexRef.intLockQueue->at(val->value.integer);
    //     }

    //     break;

    // case UINT:
    //     if (indexRef.uintLockQueue->count(val->value.uinteger))
    //     {
    //         lockQueuePtr = &indexRef.uintLockQueue->at(val->value.uinteger);
    //     }

    //     break;

    // case BOOL:
    //     if (indexRef.boolLockQueue->count(val->value.boolean))
    //     {
    //         lockQueuePtr = &indexRef.boolLockQueue->at(val->value.boolean);
    //     }

    //     break;

    // case FLOAT:
    //     if (indexRef.floatLockQueue->count(val->value.floating))
    //     {
    //         lockQueuePtr = &indexRef.floatLockQueue->at(val->value.floating);
    //     }

    //     break;

    // case CHAR:
    //     if (indexRef.charLockQueue->count(val->value.character))
    //     {
    //         lockQueuePtr = &indexRef.charLockQueue->at(val->value.character);
    //     }

    //     break;

    // case CHARX:
    //     if (indexRef.stringLockQueue->count(val->str))
    //     {
    //         lockQueuePtr = &indexRef.stringLockQueue->at(val->str);
    //     }

    //     break;

    // case VARCHAR:
    //     if (indexRef.stringLockQueue->count(val->str))
    //     {
    //         lockQueuePtr = &indexRef.stringLockQueue->at(val->str);
    //     }

    //     break;

    // default:
    //     fprintf(logfile, "anomaly: %i %s %i\n", indexRef.fieldtype, __FILE__,
    //             __LINE__);
    // }

    // if (lockQueuePtr==NULL)
    // {
    //     return;
    // }

    // std::queue<lockQueueIndexEntry> &lockQueueRef = *lockQueuePtr;

    // while (!lockQueueRef.empty())
    // {
    //     lockQueueIndexEntry &entry = lockQueueRef.front();

    //     if (enginePtr->SubTransactions.count(entry.entry.subtransactionid))
    //     {
    //         class SubTransaction &subTransactionRef =
    //             *enginePtr->SubTransactions[entry.entry.subtransactionid];

    //         // do stuff
    //         switch (indexRef.fieldtype)
    //         {
    //         case INT:
    //             if (indexRef.indexmaptype==unorderedint)
    //             {
    //                 indexRef.unorderedIntIndex->at(val->value.integer) = entry.entry;
    //             }
    //             else if (indexRef.indexmaptype==uniqueint)
    //             {
    //                 indexRef.uniqueIntIndex->at(val->value.integer) = entry.entry;
    //             }
    //             else
    //             {
    //                 fprintf(logfile, "anomaly: %i %s %i\n", indexRef.indexmaptype,
    //                         __FILE__, __LINE__);
    //             }

    //         case UINT:
    //             if (indexRef.indexmaptype==unordereduint)
    //             {
    //                 indexRef.unorderedUintIndex->at(val->value.uinteger) = entry.entry;
    //             }
    //             else if (indexRef.indexmaptype==uniqueuint)
    //             {
    //                 indexRef.uniqueUintIndex->at(val->value.uinteger) = entry.entry;
    //             }
    //             else
    //             {
    //                 fprintf(logfile, "anomaly: %i %s %i\n", indexRef.indexmaptype,
    //                         __FILE__, __LINE__);
    //             }

    //             break;

    //         case BOOL:
    //             if (indexRef.indexmaptype==unorderedbool)
    //             {
    //                 indexRef.unorderedBoolIndex->at(val->value.boolean) = entry.entry;
    //             }
    //             else if (indexRef.indexmaptype==uniquebool)
    //             {
    //                 indexRef.uniqueBoolIndex->at(val->value.boolean) = entry.entry;
    //             }
    //             else
    //             {
    //                 fprintf(logfile, "anomaly: %i %s %i\n", indexRef.indexmaptype,
    //                         __FILE__, __LINE__);
    //             }

    //             break;

    //         case FLOAT:
    //             if (indexRef.indexmaptype==unorderedfloat)
    //             {
    //                 indexRef.unorderedFloatIndex->at(val->value.floating) = entry.entry;
    //             }
    //             else if (indexRef.indexmaptype==uniquefloat)
    //             {
    //                 indexRef.uniqueFloatIndex->at(val->value.floating) = entry.entry;
    //             }
    //             else
    //             {
    //                 fprintf(logfile, "anomaly: %i %s %i\n", indexRef.indexmaptype,
    //                         __FILE__, __LINE__);
    //             }

    //             break;

    //         case CHAR:
    //             if (indexRef.indexmaptype==unorderedchar)
    //             {
    //                 indexRef.unorderedCharIndex->at(val->value.character) = entry.entry;
    //             }
    //             else if (indexRef.indexmaptype==uniquechar)
    //             {
    //                 indexRef.uniqueCharIndex->at(val->value.character) = entry.entry;
    //             }
    //             else
    //             {
    //                 fprintf(logfile, "anomaly: %i %s %i\n", indexRef.indexmaptype,
    //                         __FILE__, __LINE__);
    //             }

    //             break;

    //         case CHARX:
    //             if (indexRef.indexmaptype==unorderedcharx)
    //             {
    //                 indexRef.unorderedStringIndex->at(val->str) = entry.entry;
    //             }
    //             else if (indexRef.indexmaptype==uniquecharx)
    //             {
    //                 indexRef.uniqueStringIndex->at(val->str) = entry.entry;
    //             }
    //             else
    //             {
    //                 fprintf(logfile, "anomaly: %i %s %i\n", indexRef.indexmaptype,
    //                         __FILE__, __LINE__);
    //             }

    //             break;

    //         case VARCHAR:
    //             if (indexRef.indexmaptype==unorderedvarchar)
    //             {
    //                 indexRef.unorderedStringIndex->at(val->str) = entry.entry;
    //             }
    //             else if (indexRef.indexmaptype==uniquevarchar)
    //             {
    //                 indexRef.uniqueStringIndex->at(val->str) = entry.entry;
    //             }
    //             else
    //             {
    //                 fprintf(logfile, "anomaly: %i %s %i\n", indexRef.indexmaptype,
    //                         __FILE__, __LINE__);
    //             }

    //             break;

    //         default:
    //             fprintf(logfile, "anomaly: %i %s %i\n", indexRef.fieldtype, __FILE__,
    //                     __LINE__);
    //         }

    //         //reply
    //         class MessageSubtransactionCmd *subtransactionCmdPtr =
    //             new MessageSubtransactionCmd();
    //         subtransactionCmdPtr->subtransactionStruct.locktype = PENDINGTOINDEXLOCK;
    //         subtransactionCmdPtr->subtransactionStruct.tableid = tableid;
    //         subtransactionCmdPtr->subtransactionStruct.fieldid = fieldid;
    //         subtransactionCmdPtr->fieldVal = *val;

    //         // ok gotta fake msgrcv for replyTransaction
    //         class MessageSubtransactionCmd rcv;
    //         rcv.transactionStruct.transactionid = subTransactionRef.transactionid;
    //         rcv.transactionStruct.subtransactionid = entry.entry.subtransactionid;
    //         rcv.transactionStruct.engineinstance = enginePtr->partitionid;
    //         rcv.transactionStruct.transaction_tacmdentrypoint = entry.tacmdentrypoint;
    //         rcv.transactionStruct.transaction_pendingcmdid = entry.pendingcmdid;
    //         rcv.messageStruct.payloadtype = PAYLOADSUBTRANSACTION;

    //         subTransactionRef.replyTransaction(*subtransactionCmdPtr, rcv);

    //         lockQueueRef.pop();
    //     }

    //     lockQueueRef.pop();
    // }
}

void SubTransaction::drainIndexLockQueue(int64_t tableid, int64_t fieldid,
                                         fieldValue_s *val)
{
    class Index &indexRef = schemaPtr->tables[tableid]->fields[fieldid].index;
    std::queue<lockQueueIndexEntry> *lockQueuePtr=NULL;

    switch (indexRef.fieldtype)
    {
    case INT:
        if (indexRef.intLockQueue->count(val->value.integer))
        {
            lockQueuePtr = &indexRef.intLockQueue->at(val->value.integer);
        }

        break;

    case UINT:
        if (indexRef.uintLockQueue->count(val->value.uinteger))
        {
            lockQueuePtr = &indexRef.uintLockQueue->at(val->value.uinteger);
        }

        break;

    case BOOL:
        if (indexRef.boolLockQueue->count(val->value.boolean))
        {
            lockQueuePtr = &indexRef.boolLockQueue->at(val->value.boolean);
        }

        break;

    case FLOAT:
        if (indexRef.floatLockQueue->count(val->value.floating))
        {
            lockQueuePtr = &indexRef.floatLockQueue->at(val->value.floating);
        }

        break;

    case CHAR:
        if (indexRef.charLockQueue->count(val->value.character))
        {
            lockQueuePtr = &indexRef.charLockQueue->at(val->value.character);
        }

        break;

    case CHARX:
        if (indexRef.stringLockQueue->count(val->str))
        {
            lockQueuePtr = &indexRef.stringLockQueue->at(val->str);
        }

        break;

    case VARCHAR:
        if (indexRef.stringLockQueue->count(val->str))
        {
            lockQueuePtr = &indexRef.stringLockQueue->at(val->str);
        }

        break;

    default:
        fprintf(logfile, "anomaly: %i %s %i\n", indexRef.fieldtype, __FILE__,
                __LINE__);
    }

    if (lockQueuePtr==NULL)
    {
        return;
    }

    std::queue<lockQueueIndexEntry> &lockQueueRef = *lockQueuePtr;

    while (!lockQueueRef.empty())
    {
        lockQueueIndexEntry &entry = lockQueueRef.front();

        if (enginePtr->SubTransactions.count(entry.entry.subtransactionid))
        {
            class SubTransaction &subTransactionRef =
                *enginePtr->SubTransactions[entry.entry.subtransactionid];
            //reply
            class MessageSubtransactionCmd *subtransactionCmdPtr =
                new MessageSubtransactionCmd();
            subtransactionCmdPtr->subtransactionStruct.locktype =
                PENDINGTOINDEXNOLOCK;
            subtransactionCmdPtr->subtransactionStruct.tableid = tableid;
            subtransactionCmdPtr->subtransactionStruct.fieldid = fieldid;
            subtransactionCmdPtr->fieldVal = *val;

            // ok gotta fake msgrcv for replyTransaction
            // THIS HAS TO BE BUGGY 1/19/2013, fix when testing
            class MessageSubtransactionCmd rcv;
            rcv.transactionStruct.transactionid =
                subTransactionRef.transactionid;
            rcv.transactionStruct.subtransactionid =
                entry.entry.subtransactionid;
            rcv.transactionStruct.engineinstance = enginePtr->partitionid;
            rcv.transactionStruct.transaction_tacmdentrypoint =
                entry.tacmdentrypoint;
            rcv.transactionStruct.transaction_pendingcmdid = entry.pendingcmdid;
            rcv.messageStruct.payloadtype = PAYLOADSUBTRANSACTION;
            subTransactionRef.replyTransaction(*subtransactionCmdPtr, rcv);
        }

        lockQueueRef.pop();
    }
}

int64_t SubTransaction::newrow(int64_t tableid, string row)
{
    class Table &tableRef = *schemaPtr->tables[tableid];
    int64_t newrowid = tableRef.getnextrowid();
    tableRef.newrow(newrowid, subtransactionid, row);

    return newrowid;
}

int64_t SubTransaction::updaterow(int64_t tableid, int64_t rowid, string *row)
{
    class Table &tableRef = *schemaPtr->tables[tableid];
    return tableRef.updaterow(rowid, subtransactionid, row);
}

int64_t SubTransaction::deleterow(int64_t tableid, int64_t rowid)
{
    class Table &tableRef = *schemaPtr->tables[tableid];
    return tableRef.deleterow(rowid, subtransactionid);
}

int64_t SubTransaction::deleterow(int64_t tableid, int64_t rowid,
                                  int64_t forward_rowid,
                                  int64_t forward_engineid)
{
    class Table &tableRef = *schemaPtr->tables[tableid];
    return tableRef.deleterow(rowid, subtransactionid, forward_rowid,
                              forward_engineid);
}

void SubTransaction::selectrows(int64_t tableid, vector<int64_t> *rowids,
                                locktype_e locktype, int64_t pendingcmdid,
                                vector<returnRow_s> *returnRows)
{
    int64_t tacmd = ((class MessageTransaction *)msgrcv)->transactionStruct.transaction_tacmdentrypoint;
    class Table &tableRef = *schemaPtr->tables[tableid];
    tableRef.selectrows(rowids, locktype, subtransactionid, pendingcmdid,
                        returnRows, tacmd);
}

void SubTransaction::searchReturn1(int64_t tableid, int64_t fieldid,
                                   locktype_e locktype,
                                   searchParams_s &searchParams,
                                   vector<returnRow_s> &returnRows)
{
    vector<nonLockingIndexEntry_s> indexHits;

    indexSearch(tableid, fieldid, &searchParams, &indexHits);

    if (indexHits.size()==1)
    {
        nonLockingIndexEntry_s &hit = indexHits[0];
        class MessageSubtransactionCmd &msgrcvRef =
            *(class MessageSubtransactionCmd *)msgrcv;
        vector<int64_t> rowids(1, hit.rowid);
        selectrows(tableid, &rowids, locktype, msgrcvRef.transactionStruct.transaction_pendingcmdid,
                   &returnRows);
    }
    else
    {
        // NOTFOUNDLOCK result
        returnRow_s returnRow = {};
        returnRow.locktype = NOTFOUNDLOCK;
        returnRows.push_back(returnRow);
    }
}
