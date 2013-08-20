/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL (tm). It is available either under the
 * GNU Affero Public License or under a commercial license. Contact the
 * copyright holder for information about a commercial license if terms
 * of the GNU Affero Public License do not suit you.
 *
 * This copy of InfiniSQL is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with InfiniSQL. It should be in the top level of the source
 * directory in a file entitled "COPYING".
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "infinisql_Transaction.h"
#line 28 "Transaction.cc"

Transaction::Transaction(class TransactionAgent *taPtrarg, int64_t domainidarg)
  : taPtr(taPtrarg), domainid(domainidarg)
{
#ifdef PROFILE
  transactionpointcount=0;
  transactionpoints = new PROFILER[20];
  profileEntry(__LINE__);
#endif

  transactionid = taPtr->getnexttransactionid();
  taPtr->Transactions[transactionid] = this;
  schemaPtr = taPtr->domainidsToSchemata[domainid];
  state = EXPANDING;
  pendingcmd = NOCOMMAND;
  pendingcmdid = 0;
  lockcount = 0;
  lockpendingcount = 0;
  nextpendingcmdid = 0;
}

Transaction::~Transaction()
{
  taPtr->Transactions.erase(transactionid);
}

int64_t Transaction::getengine(fieldtype_e fieldtype, fieldValue_s &fieldValue)
{
  uint64_t hash;

  switch (fieldtype)
  {
    case INT:
      hash = SpookyHash::Hash64((void *) &fieldValue.value.integer,
                                sizeof(fieldValue.value.integer), 0);
      break;

    case UINT:
      hash = SpookyHash::Hash64((void *) &fieldValue.value.uinteger,
                                sizeof(fieldValue.value.uinteger), 0);
      break;

    case BOOL:
      hash = SpookyHash::Hash64((void *) &fieldValue.value.boolean,
                                sizeof(fieldValue.value.boolean), 0);
      break;

    case FLOAT:
      hash = SpookyHash::Hash64((void *) &fieldValue.value.floating,
                                sizeof(fieldValue.value.floating), 0);
      break;

    case CHAR:
      hash = SpookyHash::Hash64((void *) &fieldValue.value.character,
                                sizeof(fieldValue.value.character), 0);
      break;

    case CHARX:
    {
      trimspace(fieldValue.str);
      hash = SpookyHash::Hash64((void *) fieldValue.str.c_str(),
                                fieldValue.str.length(), 0);
    }
    break;

    case VARCHAR:
    {
      trimspace(fieldValue.str);
      hash = SpookyHash::Hash64((void *) fieldValue.str.c_str(),
                                fieldValue.str.length(), 0);
    }
    break;

    default:
      fprintf(logfile, "anomaly %i %s %i\n", fieldtype, __FILE__, __LINE__);
      return -1;
  }

  return hash % nodeTopology.numpartitions;
}

int64_t Transaction::getEngineid(class Table *tablePtr, int64_t fieldnum)
{
  fieldtype_e fieldType = tablePtr->fields[fieldnum].type;
  uint64_t hash;

  switch (fieldType)
  {
    case INT:
      hash = SpookyHash::Hash64((void *) &fieldValues[fieldnum].value.integer,
                                sizeof(fieldValues[fieldnum].value.integer), 0);
      break;

    case UINT:
      hash = SpookyHash::Hash64((void *) &fieldValues[fieldnum].value.uinteger,
                                sizeof(fieldValues[fieldnum].value.uinteger), 0);
      break;

    case BOOL:
      hash = SpookyHash::Hash64((void *) &fieldValues[fieldnum].value.boolean,
                                sizeof(fieldValues[fieldnum].value.boolean), 0);
      break;

    case FLOAT:
      hash = SpookyHash::Hash64((void *) &fieldValues[fieldnum].value.floating,
                                sizeof(fieldValues[fieldnum].value.floating), 0);
      break;

    case CHAR:
      hash = SpookyHash::Hash64((void *) &fieldValues[fieldnum].value.character,
                                sizeof(fieldValues[fieldnum].value.character), 0);
      break;

    case CHARX:
      hash = SpookyHash::Hash64((void *) fieldValues[fieldnum].str.c_str(),
                                fieldValues[fieldnum].str.length(), 0);
      break;

    case VARCHAR:
      hash = SpookyHash::Hash64((void *) fieldValues[fieldnum].str.c_str(),
                                fieldValues[fieldnum].str.length(), 0);
      break;

    default:
      fprintf(logfile, "anomaly %i %s %i\n", fieldType, __FILE__, __LINE__);
      return -1;
  }

  return hash % nodeTopology.numpartitions;
}

int64_t Transaction::getEngineid(class Table *tablePtr, int64_t fieldid,
                                 fieldValue_s *val)
{
  fieldtype_e fieldType = tablePtr->fields[fieldid].type;
  uint64_t hash;

  switch (fieldType)
  {
    case INT:
      hash = SpookyHash::Hash64((void *) &val->value.integer,
                                sizeof(val->value.integer), 0);
      break;

    case UINT:
      hash = SpookyHash::Hash64((void *) &val->value.uinteger,
                                sizeof(val->value.uinteger), 0);
      break;

    case BOOL:
      hash = SpookyHash::Hash64((void *) &val->value.boolean,
                                sizeof(val->value.boolean), 0);
      break;

    case FLOAT:
      hash = SpookyHash::Hash64((void *) &val->value.floating,
                                sizeof(val->value.floating), 0);
      break;

    case CHAR:
      hash = SpookyHash::Hash64((void *) &val->value.character,
                                sizeof(val->value.character), 0);
      break;

    case CHARX:
      hash = SpookyHash::Hash64((void *) val->str.c_str(), val->str.length(),
                                0);
      break;

    case VARCHAR:
      hash = SpookyHash::Hash64((void *) val->str.c_str(), val->str.length(),
                                0);
      break;

    default:
      fprintf(logfile, "anomaly %i %s %i\n", fieldType, __FILE__, __LINE__);
      return -1;
  }

  return hash % nodeTopology.numpartitions;
}

// TODO forget this for now 10/22/2012
void Transaction::dispatch(class Message *msgrcv)
{
}

void Transaction::dispatched(class Message *msgrcv)
{
}

void Transaction::continueInsertRow(int64_t entrypoint)
{
  class MessageSubtransactionCmd &subtransactionCmdRef =
        *((MessageSubtransactionCmd *)msgrcv);
#ifdef PROFILE
  profileEntry(__LINE__);
#endif

  switch (entrypoint)
{
    case 1:
    {
      currentCmdState.rowid = subtransactionCmdRef.cmd.rowid;
      currentCmdState.engineid = subtransactionCmdRef.cmd.engineid;
      currentCmdState.locktype = WRITELOCK;

      enginesWithUniqueIndices = 0;

      for (size_t n=0; n<currentCmdState.indexEntries.size(); n++)
      {
        if (fieldValues[n].isnull==true)
        {
          continue; // can add as many nulls as possible if nulls allowed
        }

        if (currentCmdState.tablePtr->fields[n].index.isunique==true)
        {
          enginesWithUniqueIndices++;
          currentCmdState.indexEntries[n].isaddunique = true;
          // engine UNIQUEINDEX looks for:
          // transaction_enginecmd = UNIQUEINDEX
          // tableid,fieldid,rowid,engineid (of index value),
          // fieldValue
          class MessageSubtransactionCmd *msg =
                new class MessageSubtransactionCmd();
          currentCmdState.rowidsEngineids[n].rowid =
            subtransactionCmdRef.cmd.rowid;

          msg->cmd.fieldVal = currentCmdState.indexEntries[n].fieldVal;
          msg->cmd.tableid = currentCmdState.tableid;
          msg->cmd.fieldid = n;
          msg->cmd.rowid = subtransactionCmdRef.cmd.rowid;
          msg->cmd.engineid = currentCmdState.rowidsEngineids[n].engineid;

          sendTransaction(UNIQUEINDEX, PAYLOADSUBTRANSACTION, 2,
                          currentCmdState.rowidsEngineids[n].engineid, (void *)msg);
        }
      }

      if (enginesWithUniqueIndices)
    {
        return; // means need to wait for replies
      }
    }
    break;

    case 2:
    {
      indexInfo_s idxInfo = {};
      idxInfo.engineid = subtransactionCmdRef.cmd.engineid;
      idxInfo.fieldVal = subtransactionCmdRef.cmd.fieldVal;
      idxInfo.fieldid = subtransactionCmdRef.cmd.fieldid;
      idxInfo.locktype = subtransactionCmdRef.cmd.locktype;
      idxInfo.tableid = subtransactionCmdRef.cmd.tableid;
      idxInfo.isaddunique = true;

      switch (idxInfo.locktype)
      {
        case NOLOCK: // constraint violation, abort command
          abortCmd(APISTATUS_UNIQUECONSTRAINT);
          return;
          break;

        case INDEXLOCK:
          checkLock(ADDLOCKEDENTRY, false, 0, subtransactionCmdRef.cmd.tableid,
                    0, subtransactionCmdRef.cmd.fieldid,
                    &subtransactionCmdRef.cmd.fieldVal);
          break;

        case INDEXPENDINGLOCK:
          checkLock(ADDLOCKPENDINGENTRY, false, 0,
                    subtransactionCmdRef.cmd.tableid, 0,
                    subtransactionCmdRef.cmd.fieldid,
                    &subtransactionCmdRef.cmd.fieldVal);
          fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
          return;
          break;

        case PENDINGTOINDEXLOCK:
          checkLock(TRANSITIONPENDINGTOLOCKEDENTRY, false, 0,
                    subtransactionCmdRef.cmd.tableid, 0,
                    subtransactionCmdRef.cmd.fieldid,
                    &subtransactionCmdRef.cmd.fieldVal);
          fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
          break;

        case PENDINGTOINDEXNOLOCK: // unique constraint violation
          checkLock(REMOVELOCKPENDINGENTRY, false, 0,
                    subtransactionCmdRef.cmd.tableid, 0,
                    subtransactionCmdRef.cmd.fieldid,
                    &subtransactionCmdRef.cmd.fieldVal);
          fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
          return;
          break;

        default:
          fprintf(logfile, "anomaly: %i %s %i\n",
                  subtransactionCmdRef.cmd.locktype, __FILE__, __LINE__);
      }

      currentCmdState.indexEntries[idxInfo.fieldid] = idxInfo;
      enginesWithUniqueIndices--;

      if (enginesWithUniqueIndices)
      {
        return;
      }
    }
    break;

    default:
      fprintf(logfile, "anomaly: %lu %s %i\n", entrypoint, __FILE__, __LINE__);
  }

  // all replies have been received
  uuRecord_s uur = { currentCmdState.rowid, currentCmdState.tableid,
                     currentCmdState.rowEngineid
                   };
  stagedRow_s sRow = {};
  sRow.cmd = INSERT;
  sRow.locktype = WRITELOCK;
  sRow.newRow = currentCmdState.row;
  sRow.newrowid = currentCmdState.rowid;
  sRow.newengineid = currentCmdState.rowEngineid;
  stagedRows[uur] = sRow;

  returnNewRow = uur;

  reenter(APISTATUS_OK);
}

// only 1 stage this can be in, so no need to switch on entrypoint
void Transaction::continueDeleteRow(int64_t entrypoint)
{
  class MessageSubtransactionCmd &subtransactionCmdRef =
        *((MessageSubtransactionCmd *)msgrcv);

  if (subtransactionCmdRef.cmd.status != STATUS_OK)
{
    reenter(APISTATUS_NOTOK);
    return;
  }
  else
  {
    stagedRows[currentCmdState.originaluur].cmd = DELETE;
    reenter(APISTATUS_OK);
  }
}

void Transaction::continueSelectRows(int64_t entrypoint)
{
  class MessageSubtransactionCmd &subtransactionCmdRef =
        *((MessageSubtransactionCmd *)msgrcv);

  switch (entrypoint)
{
    case 1:
    {

      // add rowid-engineids to vector, decrement currentCmdState.engines
      // if it's zero, then send messages to engines to see if they're
      // real rowids
      size_t numhits = subtransactionCmdRef.cmd.indexHits.size();
      currentCmdState.rowidsEngineids.
      reserve(currentCmdState.rowidsEngineids.size() + numhits);

      for (size_t n = 0; n < numhits; n++)
      {
        currentCmdState.rowidsEngineids.
        push_back(subtransactionCmdRef.cmd.indexHits[n]);
      }

      currentCmdState.engines--;

      if (!currentCmdState.engines)
      {
        if (currentCmdState.rowidsEngineids.empty()==true)
        {
          reenter(APISTATUS_OK); // nothing returned
          return;
        }

        // now walk through the hits creating messages destined for
        // each engine
        // SELECTROWS,SUBTRANSACTIONCMDPAYLOAD
        // tableid, rowids, locktype
        // for 1 hit, no rigamarole
        indexEntry_s rowidengineid;

        if (currentCmdState.rowidsEngineids.size()==1)
        {
          currentCmdState.engines = 1;

          class MessageSubtransactionCmd *msg =
                new class MessageSubtransactionCmd();
          msg->cmd.tableid = currentCmdState.tableid;
          msg->cmd.locktype = currentCmdState.locktype;
          rowidengineid = currentCmdState.rowidsEngineids[0];
          msg->cmd.rowids.push_back(rowidengineid.rowid);
          sendTransaction(SELECTROWS, PAYLOADSUBTRANSACTION, 2,
                          rowidengineid.engineid, (void *)msg);
        }
        else     // walk through
      {
          boost::unordered_map< int64_t, vector<int64_t> > payloads;
          currentCmdState.engines = 0;

          for (size_t n=0; n < currentCmdState.rowidsEngineids.size(); n++)
          {
            rowidengineid = currentCmdState.rowidsEngineids[n];
            payloads[rowidengineid.engineid].push_back(rowidengineid.rowid);
          }

          boost::unordered_map< int64_t, vector<int64_t> >::iterator it;

          for (it = payloads.begin(); it != payloads.end(); it++)
          {
            currentCmdState.engines++;

            class MessageSubtransactionCmd *msg =
                  new class MessageSubtransactionCmd();
            msg->cmd.tableid = currentCmdState.tableid;
            msg->cmd.locktype = currentCmdState.locktype;
            rowidengineid = currentCmdState.rowidsEngineids[0];
            msg->cmd.rowids = it->second;
            sendTransaction(SELECTROWS, PAYLOADSUBTRANSACTION, 2,
                            it->first, (void *)msg);
          }
        }
      }
    }
    break;

    case 2:
      // here's where we receive the returned rows
      // done receiving when all replies come in and no pendings remain
      // replies (currentCmdState.engines is the counter)
      // reply content is returnRows, vector of returnRow:
      // put each in mapofRows, with appropriate locktype & row.
      // return to user? rowid,tableid,engineid vector ?: returnselectedrows
      // probably save space, populate return stuff before all replies received

      // actually, i think they all have to be ready to be locked before moving them
      // into the mapofRows, otherwise rollback would be tricky for the command itself

  {
      // something faster can probably be done for simple equality
      // selects, since they have 1 returned object, or optimize later
      returnRow_s rRow = {};
      uuRecord_s uur = { -1, currentCmdState.tableid,
                         subtransactionCmdRef.engineinstance
                       };
      stagedRow_s sRow = {};

      for (size_t n=0; n < subtransactionCmdRef.cmd.returnRows.size(); n++)
      {
        rRow = subtransactionCmdRef.cmd.returnRows[n];
        uur.rowid = rRow.rowid;

        if (currentCmdState.pendingStagedRows.count(uur))
        {
          continue; // don't re-lock the same thing, but this is probably
          // gratuitous. this check needs to happen when promoting
          // to Transaction::mapofRows
        }

        sRow.originalRow = rRow.row;
        sRow.originalrowid = uur.rowid;
        sRow.originalengineid = subtransactionCmdRef.engineinstance;
        sRow.previoussubtransactionid =
          subtransactionCmdRef.previoussubtransactionid;
        sRow.cmd = NOCOMMAND;

        switch (rRow.locktype)
        {
          case NOLOCK:
            sRow.locktype = NOLOCK;
            break;

          case READLOCK:
            sRow.locktype = READLOCK;
            checkLock(ADDLOCKEDENTRY, true, rRow.rowid, currentCmdState.tableid,
                      subtransactionCmdRef.engineinstance, -1, NULL);
            break;

          case WRITELOCK:
            sRow.locktype = WRITELOCK;
            checkLock(ADDLOCKEDENTRY, true, rRow.rowid, currentCmdState.tableid,
                      subtransactionCmdRef.engineinstance, -1, NULL);
            break;

          case PENDINGLOCK:
            sRow.locktype = PENDINGLOCK;
            checkLock(ADDLOCKPENDINGENTRY, true, rRow.rowid,
                      currentCmdState.tableid, subtransactionCmdRef.engineinstance,
                      -1, NULL);
            break;

          case PENDINGTOWRITELOCK:
            sRow.locktype = WRITELOCK;
            checkLock(TRANSITIONPENDINGTOLOCKEDENTRY, true, rRow.rowid,
                      currentCmdState.tableid, subtransactionCmdRef.engineinstance,
                      -1, NULL);
            break;

          case PENDINGTOREADLOCK:
            sRow.locktype = READLOCK;
            checkLock(TRANSITIONPENDINGTOLOCKEDENTRY, true, rRow.rowid,
                      currentCmdState.tableid, subtransactionCmdRef.engineinstance,
                      -1, NULL);
            break;

          case PENDINGTONOLOCK:
            sRow.locktype = NOLOCK;
            checkLock(REMOVELOCKPENDINGENTRY, true, rRow.rowid,
                      currentCmdState.tableid, subtransactionCmdRef.engineinstance,
                      -1, NULL);
            break;

          case NOTFOUNDLOCK:
            continue;
            break;

          default:
            fprintf(logfile, "anomaly: %i %s %i\n", rRow.locktype, __FILE__,
                    __LINE__);
            continue;
        }

        currentCmdState.pendingStagedRows[uur] = sRow;
      }

      // how to know if this was engineresponse or LOCKPENDING?
      // locktype is a clue.
      if (rRow.locktype != PENDINGTOWRITELOCK &&
          rRow.locktype != PENDINGTOREADLOCK &&
          rRow.locktype != PENDINGTONOLOCK)
      {
        currentCmdState.engines--;
      }

      if (!currentCmdState.engines && !lockpendingcount)
      {
        // this means all messages have been received
        // and no PENDING locks
        // put everything in mapofRows (even NOLOCK), except
        // for entries already there. also put rowid,tableid,engineid
        // in return vector "returnselectedrows"
        returnselectedrows.clear();
        returnselectedrows.reserve(currentCmdState.pendingStagedRows.size());
        boost::unordered_map< uuRecord_s, stagedRow_s >::iterator it;

        for (it = currentCmdState.pendingStagedRows.begin();
             it != currentCmdState.pendingStagedRows.end(); it++)
        {
          if (stagedRows.count(it->first))
          {
            continue; // don't re-lock the same thing
          }

          stagedRows[it->first] = it->second;
          returnselectedrows.push_back(uur);
        }

        reenter(APISTATUS_OK);
        return;
      }
    }
    break;

    default:
      fprintf(logfile, "anomaly: %lu %s %i\n", entrypoint, __FILE__, __LINE__);
  }
}

void Transaction::continueFetchRows(int64_t entrypoint)
{
  switch (entrypoint)
  {
    case 1:
    {

    }
    break;

    case 2:
    {

    }
    break;

    default:
      fprintf(logfile, "anomaly: %lu %s %i\n", entrypoint, __FILE__, __LINE__);
  }
}

// TODO
void Transaction::continueUnlockRow(int64_t entrypoint)
{
  switch (entrypoint)
  {
    case 1:
    {

    }
    break;

    case 2:
    {

    }
    break;

    default:
      fprintf(logfile, "anomaly: %lu %s %i\n", entrypoint, __FILE__, __LINE__);
  }
}

// TODO
void Transaction::continueRollbackTransaction(int64_t entrypoint)
{
  switch (entrypoint)
  {
    case 1:
    {

    }
    break;

    case 2:
    {

    }
    break;

    default:
      fprintf(logfile, "anomaly: %lu %s %i\n", entrypoint, __FILE__, __LINE__);
  }
}

void Transaction::sendTransaction(enginecmd_e enginecmd,
                                  payloadtype_e payloadtype, int64_t tacmdentrypoint, int64_t engineid,
                                  void *data)
{
  class MessageTransaction &msgref = *(class MessageTransaction *)data;
  msgref.topic = TOPIC_TRANSACTION;
  msgref.payloadtype = payloadtype;

  msgref.transactionid = transactionid;

  if (engineToSubTransactionids.count(engineid))
  {
    msgref.subtransactionid = engineToSubTransactionids[engineid];
  }
  else
  {
    msgref.subtransactionid = 0;
  }

  msgref.tainstance = taPtr->instance;
  msgref.domainid = domainid;
  msgref.payloadtype = payloadtype;
  msgref.transaction_enginecmd = enginecmd;
  msgref.transaction_pendingcmdid = pendingcmdid;
  msgref.transaction_tacmdentrypoint = tacmdentrypoint;

#ifdef PROFILE
  profileEntry(__LINE__);
#endif
  taPtr->mboxes.toPartition(taPtr->myIdentity.address, engineid,
                            *((class Message *)data));
#ifdef PROFILE
  profileEntry(__LINE__);
#endif
}

void Transaction::processTransactionMessage(class Message *msgrcvarg)
{
  msgrcv = msgrcvarg;
  class MessageTransaction &msgrcvRef =
        *((class MessageTransaction *)msgrcv);

  if (pendingcmdid != msgrcvRef.transaction_pendingcmdid)
{
    printf("%s %i pendingcmdid %li msgrcvRef.transaction_pendingcmdid %li\n", __FILE__, __LINE__, pendingcmdid, msgrcvRef.transaction_pendingcmdid);
    badMessageHandler();
    return;
  }

  if (!engineToSubTransactionids.count(msgrcvRef.engineinstance))
  {
    engineToSubTransactionids[msgrcvRef.engineinstance] =
      msgrcvRef.subtransactionid;
  }

  switch (pendingcmd)
  {
    case NOCOMMAND:
    {
      badMessageHandler();
    }
    break;

    case INSERT:
      continueInsertRow(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case UPDATE:
      continueUpdateRow(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case DELETE:
      continueDeleteRow(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case REPLACE:
      continueReplaceRow(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case SELECT:
      continueSelectRows(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case FETCH:
      continueFetchRows(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case UNLOCK:
      continueUnlockRow(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case COMMIT:
      continueCommitTransaction(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case ROLLBACK:
      continueRollbackTransaction(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case PRIMITIVE_SQLPREDICATE:
      continueSqlPredicate(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case PRIMITIVE_SQLSELECTALL:
      continueSqlPredicate(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case PRIMITIVE_SQLSELECTALLFORDELETE:
      continueSqlPredicate(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case PRIMITIVE_SQLSELECTALLFORUPDATE:
      continueSqlPredicate(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case PRIMITIVE_SQLDELETE:
      continueSqlDelete(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case PRIMITIVE_SQLINSERT:
      continueSqlInsert(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case PRIMITIVE_SQLUPDATE:
      continueSqlUpdate(msgrcvRef.transaction_tacmdentrypoint);
      break;

    case PRIMITIVE_SQLREPLACE:
      continueSqlReplace(msgrcvRef.transaction_tacmdentrypoint);
      break;

    default:
      fprintf(logfile, "anomaly: %i %s %i\n", pendingcmd, __FILE__, __LINE__);
  }

}

void Transaction::select(int64_t tableid, int64_t fieldid, locktype_e locktype,
                         searchParams_s *searchParameters)
{
  searchParams_s &searchParamsRef = *searchParameters;

  if (pendingcmd != NOCOMMAND)
  {
    reenter(APISTATUS_PENDING);
    return;
  }

  pendingcmdid = getnextpendingcmdid();
  pendingcmd = SELECT;
  currentCmdState.tableid = tableid;
  currentCmdState.fieldid = fieldid;
  currentCmdState.locktype = locktype;

  // INDEXSEARCH,SUBTRANSACTIONCMDPAYLOAD
  // tableid,fieldid,searchParameters
  // returns indexHits
  currentCmdState.engines = 0;
  currentCmdState.rowidsEngineids.clear();
  currentCmdState.ispossibledeadlock = false;

  if (searchParamsRef.op == OPERATOR_EQ)   // IN should probably be optimized this way
    // too, eventually
  {
    int64_t destengineid=-1;
    currentCmdState.engines = 1;
    // engine id is the hashed value, then send message to it
    class MessageSubtransactionCmd *msg = new class MessageSubtransactionCmd();
    msg->cmd.tableid = currentCmdState.tableid;
    msg->cmd.fieldid = currentCmdState.fieldid;
    msg->cmd.searchParameters = searchParamsRef;
    fieldtype_e fieldtype = schemaPtr->tables[tableid]->fields[fieldid].type;

    switch (fieldtype)
    {
      case INT:
        destengineid = getEngineid(searchParamsRef.values[0].value.integer);
        break;

      case UINT:
        destengineid = getEngineid(searchParamsRef.values[0].value.uinteger);
        break;

      case BOOL:
        destengineid = getEngineid(searchParamsRef.values[0].value.boolean);
        break;

      case FLOAT:
        destengineid = getEngineid(searchParamsRef.values[0].value.floating);
        break;

      case CHAR:
        destengineid = getEngineid(searchParamsRef.values[0].value.character);
        break;

      case CHARX:
        destengineid = getEngineid(&searchParamsRef.values[0].str);
        break;

      case VARCHAR:
        destengineid = getEngineid(&searchParamsRef.values[0].str);
        break;

      default:
        fprintf(logfile, "anomaly %i %s %i\n", fieldtype, __FILE__, __LINE__);
    }

    sendTransaction(INDEXSEARCH, PAYLOADSUBTRANSACTION, 1,
                    destengineid, (void *)msg);
  }
  else
  {
    // walk through all engines, send message to each, bumping engines
    currentCmdState.engines = nodeTopology.numpartitions;

    for (int n=0; n < currentCmdState.engines; n++)
    {
      class MessageSubtransactionCmd *msg =
            new class MessageSubtransactionCmd();
      msg->cmd.tableid = currentCmdState.tableid;
      msg->cmd.fieldid = currentCmdState.fieldid;
      msg->cmd.searchParameters = searchParamsRef;
      sendTransaction(INDEXSEARCH, PAYLOADSUBTRANSACTION, 1,
                      n, (void *)msg);
    }
  }
}

void Transaction::deadlockAbort(class MessageDeadlock &msgref)
{
  if (msgref.transaction_pendingcmdid != pendingcmdid)
  {
    return;
  }

  if (currentCmdState.pendingStagedRows.empty()==true)
  {
    return;
  }

  int64_t rowid, tableid, engineid;
  rowOrField_s blankRof = {};
  rowOrField_s rof;
  //    vector<rowOrField> *rofs;

  boost::unordered_map< uuRecord_s, stagedRow_s >::iterator it;

  for (it = currentCmdState.pendingStagedRows.begin();
       it != currentCmdState.pendingStagedRows.end(); it++)
  {
    rowid = it->first.rowid;
    tableid = it->first.tableid;
    engineid = it->first.engineid;
    rof = blankRof;

    rof.isrow = true;
    rof.rowid = rowid;
    rof.tableid = tableid;

    class MessageCommitRollback *msg = new class MessageCommitRollback();
    // rofs = new vector<rowOrField>;
    msg->rofs.push_back(rof);

    sendTransaction(ROLLBACKCMD, PAYLOADCOMMITROLLBACK, 0, engineid,
                    (void *)msg);
  }

  reenter(APISTATUS_DEADLOCK);
}

void Transaction::checkLock(deadlockchange_e changetype, bool isrow,
                            int64_t rowid, int64_t tableid, int64_t engineid, int64_t fieldid,
                            fieldValue_s *fieldVal)
{
  switch (changetype)
  {
    case ADDLOCKEDENTRY:
      lockcount++;
      break;

    case ADDLOCKPENDINGENTRY:
      lockpendingcount++;
      break;

    case REMOVELOCKEDENTRY:
      lockcount--;
      break;

    case REMOVELOCKPENDINGENTRY:
      lockpendingcount--;
      break;

    case TRANSITIONPENDINGTOLOCKEDENTRY:
      lockpendingcount--;
      lockcount++;
      break;

    default:
      fprintf(logfile, "anomaly %i %s %i\n", changetype, __FILE__, __LINE__);
  }

  // OK, now, what? I'm either in deadlock and sending another message
  // not deadlocked, not deadlocked
  // deadlocked, then not
  // not deadlocked, then deadlocked
  if (currentCmdState.ispossibledeadlock==false &&
      (!lockcount || !lockpendingcount))
  {
    // move along
    return;
  }

  if (currentCmdState.ispossibledeadlock==false && lockpendingcount
      && lockcount)
  {
    // new deadlock!
    currentCmdState.ispossibledeadlock = true;
    // send a bunch of messages, in pendingMapofRows, then the input
    class MessageDeadlock *msg = new class MessageDeadlock;
    class MessageDeadlock &msgref = *msg;
    newDeadLockLists_s &nodesRef = msgref.nodes;

    boost::unordered_map< uuRecord_s, stagedRow_s >::iterator it;

    for (it = currentCmdState.pendingStagedRows.begin();
         it != currentCmdState.pendingStagedRows.end(); it++)
    {
      stagedRow_s &sRowRef = it->second;
      int64_t this_rowid = it->first.rowid;
      int64_t this_tableid = it->first.tableid;
      int64_t this_engineid = it->first.engineid;
      string deadlockNode;
      // free this if there's no message to send (no contents in its sets)

      // row first
      DeadlockMgr::makeLockedItem(true, this_rowid, this_tableid,
                                  this_engineid, domainid, 0, (long double)0, (string *)NULL,
                                  &deadlockNode);

      if (sRowRef.locktype==WRITELOCK || sRowRef.locktype==READLOCK)
      {
        nodesRef.locked.insert(deadlockNode);
      }
      else if (sRowRef.locktype==PENDINGLOCK)
      {
        nodesRef.waiting.insert(deadlockNode);
      }

      // indices
      boost::unordered_map< int64_t, lockFieldValue_s >::iterator it;

      for (it = sRowRef.uniqueIndices.begin();
           it != sRowRef.uniqueIndices.end(); it++)
      {
        lockFieldValue_s &lockFieldValueRef = it->second;

        if (lockFieldValueRef.locktype==INDEXLOCK)
        {
          deadlockNode.clear();
          long double fieldinput;
          memcpy(&fieldinput, &lockFieldValueRef.fieldVal.value,
                 sizeof(fieldinput));
          DeadlockMgr::makeLockedItem(false, 0, this_tableid,
                                      this_engineid, domainid, it->first, fieldinput,
                                      &lockFieldValueRef.fieldVal.str, &deadlockNode);
          nodesRef.locked.insert(deadlockNode);
        }
        else if (lockFieldValueRef.locktype==INDEXPENDINGLOCK)
        {
          deadlockNode.clear();
          long double fieldinput;
          memcpy(&fieldinput, &lockFieldValueRef.fieldVal.value,
                 sizeof(fieldinput));
          DeadlockMgr::makeLockedItem(false, 0, this_tableid,
                                      this_engineid, domainid, it->first, fieldinput,
                                      &lockFieldValueRef.fieldVal.str, &deadlockNode);
          nodesRef.waiting.insert(deadlockNode);
        }
      }
    }

    if (nodesRef.locked.empty()==true && nodesRef.waiting.empty()==true)
    {
      // delete nodes; // nothing to deadlock, but this should be an anomaly
      delete msg;
      fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
      return;
    }

    msgref.topic = TOPIC_DEADLOCKNEW;
    msgref.transactionid = transactionid;
    msgref.tainstance = taPtr->instance;
    msgref.transaction_pendingcmdid = pendingcmdid;

    //    taPtr->mboxes.deadlockMgr.send(msgsnd, true);
    taPtr->mboxes.toDeadlockMgr(taPtr->myIdentity.address, *msg);
    return;
  }

  if (lockcount && lockpendingcount)
  {
    // still deadlocked, just send 1 message based on type
    class MessageDeadlock *msg = new class MessageDeadlock();
    class MessageDeadlock &msgref = *msg;

    // prepare string(s) for submittal
    if (isrow==true)
    {
      DeadlockMgr::makeLockedItem(true, rowid, tableid, engineid, domainid,
                                  fieldid, (long double)0, (string *)NULL, &msgref.deadlockNode);
    }
    else
    {
      long double fieldinput;
      memcpy(&fieldinput, &fieldVal->value, sizeof(fieldinput));
      DeadlockMgr::makeLockedItem(false, rowid, tableid, engineid, domainid,
                                  fieldid, fieldinput, &fieldVal->str, &msgref.deadlockNode);
    }

    // send message to dmgr
    msgref.topic = TOPIC_DEADLOCKCHANGE;
    msgref.deadlockchange = changetype;
    msgref.transactionid = transactionid;

    //    taPtr->mboxes.deadlockMgr.send(msgsnd, true);
    taPtr->mboxes.toDeadlockMgr(taPtr->myIdentity.address, *msg);

    return;
  }

  if (!lockcount || !lockpendingcount)
  {
    // deadlock over, send message to dmgr to that effect
    currentCmdState.ispossibledeadlock = false;

    // send message to dmgr
    class MessageDeadlock *msg = new class MessageDeadlock;
    class MessageDeadlock &msgref = *msg;
    msgref.topic = TOPIC_DEADLOCKREMOVE;
    msgref.transactionid = transactionid;

    //    taPtr->mboxes.deadlockMgr.send(msgsnd, true);
    taPtr->mboxes.toDeadlockMgr(taPtr->myIdentity.address, *msg);
    return;
  }
}

void Transaction::updateRow(void)
{
  if (!stagedRows.count(currentCmdState.originaluur))
  {
    reenter(APISTATUS_NOTOK);
    return;
  }

  if (stagedRows[currentCmdState.originaluur].locktype != WRITELOCK ||
      stagedRows[currentCmdState.originaluur].cmd != NOCOMMAND ||
      pendingcmd != NOCOMMAND)
  {
    reenter(APISTATUS_NOTOK);
    return;
  }

  pendingcmdid = getnextpendingcmdid();
  currentCmdState.tablePtr =
    schemaPtr->tables[currentCmdState.originaluur.tableid];
  class Table &tableRef = *currentCmdState.tablePtr;
  // construct new row and put in currentCmdState.newRow
  currentCmdState.originalFieldValues.clear();

  if (tableRef.unmakerow(&stagedRows[currentCmdState.originaluur].originalRow,
                         &currentCmdState.originalFieldValues)==false)
  {
    reenter(APISTATUS_FIELD);
    return;
  }

  currentCmdState.newFieldValues.
  reserve(currentCmdState.originalFieldValues.size());

  // create new row from old row & updates and set it in currentCmdState.newRow
  for (size_t n=0; n < currentCmdState.originalFieldValues.size(); n++)
  {
    if (currentCmdState.isupdatemultiplefields==false)
    {
      if (currentCmdState.fieldid==(int64_t)n)
      {
        currentCmdState.newFieldValues.push_back(currentCmdState.fieldVal);
      }
      else
      {
        currentCmdState.newFieldValues.
        push_back(currentCmdState.originalFieldValues[n]);
      }
    }
    else if (fieldsToUpdate.count(n))
    {
      currentCmdState.newFieldValues.push_back(fieldsToUpdate[n]);
    }
    else
    {
      currentCmdState.newFieldValues.
      push_back(currentCmdState.originalFieldValues[n]);
    }
  }

  if (tableRef.makerow(&currentCmdState.newFieldValues,
                       &currentCmdState.newRow)==false)
  {
    reenter(APISTATUS_FIELD);
    return;
  }

  if (currentCmdState.isupdatemultiplefields==false)
  {
    if (currentCmdState.fieldid==0)
    {
      pendingcmd=REPLACE;
      replace();
      return;
    }
  }
  else if (fieldsToUpdate.count(0))
  {
    pendingcmd=REPLACE;
    replace();
    return;
  }

  // otherwise, it's an update
  pendingcmd=UPDATE;
  currentCmdState.newuur = currentCmdState.originaluur;
  continueUpdateRow(1); // somewhere after replaces, so continueReplace forwards
  // there...
}

void Transaction::replace(void)
{
  // do like insert first, check null constraints then get new row
  for (size_t n=0; n < currentCmdState.newFieldValues.size(); n++)
  {
    if (checkNullConstraintOK(n)==false)
    {
      reenter(APISTATUS_NULLCONSTRAINT);
      return;
    }
  }

  currentCmdState.newuur.tableid = currentCmdState.originaluur.tableid;
  currentCmdState.newuur.engineid = getEngineid(currentCmdState.tablePtr, 0,
                                    &currentCmdState.newFieldValues[0]);

  // just send message to engine to put the new row in place
  //    subtransactionCmd *cmd = new subtransactionCmd();
  class MessageSubtransactionCmd *msg = new class MessageSubtransactionCmd();

  msg->cmd.tableid = currentCmdState.originaluur.tableid;
  msg->cmd.row = currentCmdState.newRow;

  sendTransaction(NEWROW, PAYLOADSUBTRANSACTION, 1,
                  currentCmdState.newuur.engineid, (void *)msg);
}

void Transaction::continueUpdateRow(int64_t entrypoint)
{
  switch (entrypoint)
  {
    case 1:
    {
      // for update only, send currentCmdState.newRow, rowid,tableid to
      // subtransaction subtransactionCmd *cmd = new subtransactionCmd();
      class MessageSubtransactionCmd *msg =
            new class MessageSubtransactionCmd();
      msg->cmd.tableid = currentCmdState.newuur.tableid;
      msg->cmd.rowid = currentCmdState.newuur.rowid;
      msg->cmd.row = currentCmdState.newRow;

      sendTransaction(UPDATEROW, PAYLOADSUBTRANSACTION, 2,
                      currentCmdState.newuur.engineid, (void *)msg);
    }
    break;

    case 2:
  {
      // process updaterow message (status needs to be STATUS_OK)
      class MessageSubtransactionCmd &subtransactionCmdRef =
            *((MessageSubtransactionCmd *)msgrcv);
      int64_t status = subtransactionCmdRef.cmd.status;

      if (status != STATUS_OK)
    {
        fprintf(logfile, "anomaly %li %s %i\n", status, __FILE__, __LINE__);
        revert(currentCmdState.newuur);
        reenter(status);
      }
    }

    //         break; just fall through if the row was staged ok
    case 3: // do unique indices. point them to currentCmdState.newuur.rowid & engineid
    {
      stagedRow_s sRow = stagedRows[currentCmdState.originaluur];
      sRow.cmd = UPDATE;
      sRow.newRow = currentCmdState.newRow;
      sRow.newengineid = currentCmdState.newuur.engineid;
      sRow.newrowid = currentCmdState.newuur.rowid;
      // sRow.originalRow should already be in the stagedRows
      // following 2 are probably gratuitous
      sRow.originalengineid = currentCmdState.originaluur.engineid;
      sRow.originalrowid = currentCmdState.originaluur.rowid;
      // sRow.uniqueIndices should be born clear

      currentCmdState.uniqueindices = 0;

      for (size_t n=0; n < currentCmdState.newFieldValues.size(); n++)
      {
        if (currentCmdState.newFieldValues[n].isnull==true)
        {
          continue; // can add as many nulls as possible if nulls allowed
        }

        if (currentCmdState.tablePtr->fields[n].index.isunique==true)
        {
          if (currentCmdState.newFieldValues[n].value.floating ==
              currentCmdState.originalFieldValues[n].value.floating &&
              currentCmdState.newFieldValues[n].isnull ==
              currentCmdState.originalFieldValues[n].isnull &&
              currentCmdState.newFieldValues[n].str ==
              currentCmdState.originalFieldValues[n].str)
          {
            continue; // no change to field
          }

          currentCmdState.uniqueindices++;
          lockFieldValue_s lockFieldVal = {};
          lockFieldVal.locktype = NOLOCK; // no lock yet
          lockFieldVal.fieldVal.isnull = false;
          lockFieldVal.fieldVal.str = currentCmdState.newFieldValues[n].str;
          memcpy(&lockFieldVal.fieldVal.value,
                 &currentCmdState.newFieldValues[n].value,
                 sizeof(lockFieldVal.fieldVal.value));
          lockFieldVal.engineid = getEngineid(currentCmdState.tablePtr,
                                              n, &lockFieldVal.fieldVal);
          sRow.uniqueIndices[n] = lockFieldVal;

          class MessageSubtransactionCmd *msg =
                new class MessageSubtransactionCmd();

          msg->cmd.isrow = false;
          msg->cmd.fieldVal.isnull = lockFieldVal.fieldVal.isnull;
          msg->cmd.fieldVal.str = lockFieldVal.fieldVal.str;
          memcpy(&msg->cmd.fieldVal.value, &lockFieldVal.fieldVal.value,
                 sizeof(lockFieldVal.fieldVal.value));
          msg->cmd.tableid = currentCmdState.newuur.tableid;
          msg->cmd.fieldid = n;
          msg->cmd.rowid = currentCmdState.newuur.rowid;
          msg->cmd.engineid = currentCmdState.newuur.engineid;
          sendTransaction(UNIQUEINDEX, PAYLOADSUBTRANSACTION, 4,
                          lockFieldVal.engineid, (void *)msg);
        }
      }

      currentCmdState.pendingStagedRows[currentCmdState.originaluur] = sRow;

      if (!currentCmdState.uniqueindices)
    {
        continueUpdateRow(5); // no need to wait for unique index responses
      }
    }
    break;

    case 4:
    {
      // get responses from unique index set
      class MessageSubtransactionCmd &subtransactionCmdRef =
            *((MessageSubtransactionCmd *)msgrcv);
      int64_t tableid = subtransactionCmdRef.cmd.tableid;
      int64_t fieldid = subtransactionCmdRef.cmd.fieldid;
      fieldValue_s fieldVal;
      fieldVal.isnull = subtransactionCmdRef.cmd.fieldVal.isnull;
      fieldVal.str = subtransactionCmdRef.cmd.fieldVal.str;
      memcpy(&fieldVal.value, &subtransactionCmdRef.cmd.fieldVal.value,
             sizeof(fieldVal.value));

      switch (subtransactionCmdRef.cmd.locktype)
    {
        case NOLOCK: // unique constraint violation, abort command
          printf("%s %i APISTATUS_UNIQUECONSTRAINT (NOLOCK)\n", __FILE__, __LINE__);
          abortCmd(APISTATUS_UNIQUECONSTRAINT);
          return;
          break;

        case INDEXLOCK:
          currentCmdState.pendingStagedRows[currentCmdState.originaluur].
          uniqueIndices[fieldid].locktype = INDEXLOCK;
          checkLock(ADDLOCKEDENTRY, false, 0, tableid, 0, fieldid,
                    &fieldVal);
          break;

        case INDEXPENDINGLOCK:
          currentCmdState.pendingStagedRows[currentCmdState.originaluur].
          uniqueIndices[fieldid].locktype = INDEXPENDINGLOCK;
          checkLock(ADDLOCKPENDINGENTRY, false, 0, tableid, 0,
                    fieldid, &fieldVal);
          return;
          break;

        case PENDINGTOINDEXLOCK:
          currentCmdState.pendingStagedRows[currentCmdState.originaluur].
          uniqueIndices[fieldid].locktype = INDEXLOCK;
          checkLock(TRANSITIONPENDINGTOLOCKEDENTRY, false, 0, tableid,
                    0, fieldid, &fieldVal);
          break;

        case PENDINGTOINDEXNOLOCK: // unique constraint violation
          printf("%s %i APISTATUS_UNIQUECONSTRAINT (PENDINGTOINDEXNOLOCK)\n", __FILE__, __LINE__);
          abortCmd(APISTATUS_UNIQUECONSTRAINT);
          return;
          break;

        default:
          fprintf(logfile, "anomaly: %i %s %i\n",
                  subtransactionCmdRef.cmd.locktype, __FILE__, __LINE__);
      }

      if (--enginesWithUniqueIndices)   // need to wait for more replies
      {
        return;
      }

      // otherwise, we're home free!
    }

    // break; fall through intentionally
    case 5: // post to stagedRows & reenter ? include row for user response? and
      // uur?
    {
      stagedRows[currentCmdState.originaluur] =
        currentCmdState.pendingStagedRows[currentCmdState.originaluur];
      // no need to return anything explicit, since the new row is in stagedRows
      // for the original uur
      reenter(APISTATUS_OK);
    }
    break;

    default:
      fprintf(logfile, "anomaly: %lu %s %i\n", entrypoint, __FILE__, __LINE__);
  }
}

void Transaction::continueReplaceRow(int64_t entrypoint)
{
  class MessageSubtransactionCmd &subtransactionCmdRef =
        *((MessageSubtransactionCmd *)msgrcv);

  switch (entrypoint)
{
    case 1:
    {
      // NEWROW assumed always succeeds
      currentCmdState.newuur.rowid = subtransactionCmdRef.cmd.rowid;

      // now delete the old row, with forwarder
      class MessageSubtransactionCmd *msg =
            new class MessageSubtransactionCmd();
      msg->cmd.tableid = currentCmdState.originaluur.tableid;
      msg->cmd.rowid = currentCmdState.originaluur.rowid;
      msg->cmd.forward_rowid = currentCmdState.newuur.rowid;
      msg->cmd.forward_engineid = currentCmdState.newuur.engineid;
      sendTransaction(REPLACEDELETEROW, PAYLOADSUBTRANSACTION, 2,
                      currentCmdState.originaluur.engineid, (void *)msg);
    }
    break;

    case 2:
  {
      // process deleted
      int64_t status = subtransactionCmdRef.cmd.status;

      if (status != STATUS_OK)
      {
        // rollback inserted row, fire & forget, reenter
        // rollback: ROLLBACKCMD,COMMITROLLBACKPAYLOAD
        // vector of rowOrField
        rowOrField_s rof = {};
        class MessageCommitRollback *msg = new class MessageCommitRollback();
        rof.isrow = true;
        rof.tableid = currentCmdState.newuur.tableid;
        rof.rowid = currentCmdState.newuur.rowid;
        msg->rofs.operator [](0) = rof;
        sendTransaction(ROLLBACKCMD, PAYLOADCOMMITROLLBACK, 0,
                        currentCmdState.newuur.engineid, (void *)msg);

        reenter(APISTATUS_NOTOK);
        return;
      }

      // otherwise, I'm now an update & jump to continueUpdateRow(1)
      pendingcmd = UPDATE;
      continueUpdateRow(3);
    }
    break;

    default:
      fprintf(logfile, "anomaly: %lu %s %i\n", entrypoint, __FILE__, __LINE__);
  }
}

void Transaction::abortCmd(int reentrystatus)
{
  // this should be just abort stopping deadlock for this transaction and
  // revert of the original uur. then reenter.
  revert(currentCmdState.originaluur);

  if (currentCmdState.ispossibledeadlock==true)
  {
    currentCmdState.ispossibledeadlock = false;
    // send message to dmgr
    class MessageDeadlock *msg = new class MessageDeadlock;
    class MessageDeadlock &msgref = *msg;
    msgref.topic = TOPIC_DEADLOCKREMOVE;
    msgref.transactionid = transactionid;
    //    taPtr->mboxes.deadlockMgr.send(msgsnd, true);
    taPtr->mboxes.toDeadlockMgr(taPtr->myIdentity.address, *msg);
  }

  reenter(reentrystatus);
}

void Transaction::continueCommitTransaction(int64_t entrypoint)
{
  // take care of return message from mirroring operation, then
  // walk through map of rows, along with indices
#ifdef PROFILE
  profileEntry(__LINE__);
#endif

  //  printf("%s %i continueCommitTransaction(%li)\n", __FILE__, __LINE__, entrypoint);

  switch (entrypoint)
  {
    case 1:
    {
      // take care of return message from mirroring operation, then
      // walk through map of rows, along with indices
      if (--waitfordispatched)
      {
        return;
      }

      boost::unordered_map< uuRecord_s, stagedRow_s >::iterator it;
      boost::unordered_map< int64_t, class MessageCommitRollback *> msgs;
      currentCmdState.replaceEngineMsgs.clear();
      rowOrField_s blankRof = {};
      rowOrField_s rof;

      for (it = stagedRows.begin(); it != stagedRows.end(); it++)
      {
        class Table &tableRef = *schemaPtr->tables[it->first.tableid];
        stagedRow_s &sRowRef = it->second;
        rof = blankRof;
        rof.tableid = it->first.tableid;

        switch (sRowRef.cmd)
        {
          case NOCOMMAND:
          {
            // subtransaction will just unlock this row
            rof.isrow = true;
            rof.rowid = it->first.rowid;

            addRof(it->first.engineid, rof, msgs);
          }
          break;

          case INSERT:
          {
            rof.isrow = true;
            rof.rowid = it->first.rowid;

            addRof(it->first.engineid, rof, msgs);
            // index stuff
            vector <fieldValue_s> fieldValues;
            tableRef.unmakerow(&sRowRef.newRow, &fieldValues);

            for (size_t n=0; n < tableRef.fields.size(); n++)
            {
              class Field &fieldRef = tableRef.fields[n];
              class Index &indexRef = fieldRef.index;

              if (indexRef.indextype==NONE)
              {
                continue;
              }

              rof = blankRof;
              rof.isrow = false;
              rof.tableid = it->first.tableid;
              rof.fieldid = n;

              if (fieldValues[n].isnull==true)
              {
                rof.isnotaddunique = true;
                rof.deleteindexentry = false;
                rof.rowid=it->first.rowid;
                rof.engineid=it->first.engineid;
                rof.fieldVal.isnull=true;
                addRof(n % nodeTopology.numpartitions, rof, msgs);

                continue;
              }

              rof.fieldVal = fieldValues[n];

              if (indexRef.isunique==true)   // commit something already locked
              {
                rof.isnotaddunique = false;
              }
              else
              {
                rof.isnotaddunique = true;
                rof.deleteindexentry = false;
                rof.engineid = it->first.engineid;
                rof.rowid = it->first.rowid;
              }

              switch (fieldRef.type)
              {
                case CHARX:
                  trimspace(rof.fieldVal.str);
                  break;

                case VARCHAR:
                  trimspace(rof.fieldVal.str);

                default:
                  ;
              }

              addRof(getEngineid(&tableRef, n, &rof.fieldVal),
                     rof, msgs);
            }
          }
          break;

          case DELETE:
          {
            rof.isrow = true;
            rof.rowid = it->first.rowid;

            addRof(it->first.engineid, rof, msgs);
            // index stuff
            vector <fieldValue_s> fieldValues;
            tableRef.unmakerow(&sRowRef.originalRow, &fieldValues);

            for (size_t n=0; n < tableRef.fields.size(); n++)
            {
              class Field &fieldRef = tableRef.fields[n];
              class Index &indexRef = fieldRef.index;

              if (indexRef.indextype==NONE)
              {
                continue;
              }

              rof = blankRof;
              rof.isrow = false;
              rof.tableid = it->first.tableid;
              rof.fieldid = n;
              rof.isnotaddunique = true;
              rof.deleteindexentry = true;
              rof.engineid = it->first.engineid;
              rof.rowid = it->first.rowid;

              if (fieldValues[n].isnull==true)
              {
                rof.fieldVal.isnull=true;
                addRof(n % nodeTopology.numpartitions, rof, msgs);

                continue;
              }

              rof.fieldVal = fieldValues[n];

              switch (fieldRef.type)
              {
                case CHARX:
                  trimspace(rof.fieldVal.str);
                  break;

                case VARCHAR:
                  trimspace(rof.fieldVal.str);

                default:
                  ;
              }

              addRof(getEngineid(&tableRef, n, &rof.fieldVal),
                     rof, msgs);
            }
          }
          break;

          case UPDATE:
          {
            if (sRowRef.originalrowid==sRowRef.newrowid &&
                sRowRef.originalengineid==sRowRef.newengineid)
            {
              rof.isrow = true;
              rof.rowid = it->first.rowid;
              addRof(it->first.engineid, rof, msgs);
              // index stuff
              vector <fieldValue_s> originalFieldValues;
              tableRef.unmakerow(&sRowRef.originalRow, &originalFieldValues);
              vector <fieldValue_s> newFieldValues;
              tableRef.unmakerow(&sRowRef.newRow, &newFieldValues);

              for (size_t n=0; n < tableRef.fields.size(); n++)
              {
                class Field &fieldRef = tableRef.fields[n];
                class Index &indexRef = fieldRef.index;

                if (indexRef.indextype==NONE)
                {
                  continue;
                }

                if ((originalFieldValues[n].value.floating !=
                     newFieldValues[n].value.floating) ||
                    (originalFieldValues[n].isnull !=
                     newFieldValues[n].isnull) ||
                    (originalFieldValues[n].str != newFieldValues[n].str))
                {
                  // add index entry
                  rof = blankRof;
                  rof.isrow = false;
                  rof.tableid = it->first.tableid;
                  rof.fieldid = n;
                  rof.fieldVal = newFieldValues[n];

                  if (newFieldValues[n].isnull==true)
                  {
                    rof.isnotaddunique = true;
                    rof.deleteindexentry = false;
                    rof.rowid=sRowRef.newrowid;
                    rof.engineid=sRowRef.newengineid;
                    rof.fieldVal.isnull=true;
                    addRof(n % nodeTopology.numpartitions, rof, msgs);

                  }
                  else
                  {
                    if (indexRef.isunique==true) //commit something already locked
                    {
                      rof.isnotaddunique = false;
                    }
                    else
                    {
                      rof.isnotaddunique = true;
                      rof.deleteindexentry = false;
                      rof.engineid = sRowRef.newengineid;
                      rof.rowid = sRowRef.newrowid;
                    }

                    switch (fieldRef.type)
                    {
                      case CHARX:
                        trimspace(rof.fieldVal.str);
                        break;

                      case VARCHAR:
                        trimspace(rof.fieldVal.str);

                      default:
                        ;
                    }

                    addRof(getEngineid(&tableRef, n, &rof.fieldVal), rof, msgs);
                  }

                  // delete index entry
                  rof = blankRof;
                  rof.isrow = false;
                  rof.tableid = it->first.tableid;
                  rof.fieldid = n;
                  rof.isnotaddunique = true;
                  rof.deleteindexentry = true;
                  rof.engineid = sRowRef.originalengineid;
                  rof.rowid = sRowRef.originalrowid;

                  if (originalFieldValues[n].isnull==true)
                  {
                    rof.fieldVal.isnull=true;
                    addRof(n % nodeTopology.numpartitions, rof, msgs);

                    continue;
                  }

                  rof.fieldVal = originalFieldValues[n];

                  switch (fieldRef.type)
                  {
                    case CHARX:
                      trimspace(rof.fieldVal.str);
                      break;

                    case VARCHAR:
                      trimspace(rof.fieldVal.str);

                    default:
                      ;
                  }

                  addRof(getEngineid(&tableRef, n, &rof.fieldVal), rof, msgs);
                }
              }
            }
            else     // replace
            {
              rof.isrow = true;
              // commit new row
              rof.rowid = sRowRef.newrowid;
              addRof(sRowRef.newengineid, rof, msgs);
              // commit delete on original row
              rof.rowid = it->first.rowid;


              if (!currentCmdState.replaceEngineMsgs.count(it->first.engineid))
              {
                currentCmdState.replaceEngineMsgs[it->first.engineid] =
                  new class MessageCommitRollback();
              }

              currentCmdState.replaceEngineMsgs[it->first.engineid]->rofs.push_back(rof);
              // index stuff
              vector <fieldValue_s> originalFieldValues;
              tableRef.unmakerow(&sRowRef.originalRow, &originalFieldValues);
              vector <fieldValue_s> newFieldValues;
              tableRef.unmakerow(&sRowRef.newRow, &newFieldValues);

              for (size_t n=0; n < tableRef.fields.size(); n++)
              {
                class Index &indexRef = tableRef.fields[n].index;

                if (indexRef.indextype==NONE)
                {
                  continue;
                }

                if ((originalFieldValues[n].value.floating !=
                     newFieldValues[n].value.floating) ||
                    (originalFieldValues[n].isnull !=
                     newFieldValues[n].isnull) ||
                    (originalFieldValues[n].str != newFieldValues[n].str))
                {
                  // add index entry
                  rof = blankRof;
                  rof.isrow = false;
                  rof.tableid = it->first.tableid;
                  rof.fieldid = n;

                  if (newFieldValues[n].isnull==true)
                  {
                    rof.isnotaddunique = true;
                    rof.deleteindexentry = false;
                    rof.rowid=sRowRef.newrowid;
                    rof.engineid=sRowRef.newengineid;
                    rof.fieldVal.isnull=true;
                    addRof(n % nodeTopology.numpartitions, rof, msgs);
                  }
                  else
                  {
                    rof.fieldVal = newFieldValues[n];

                    if (indexRef.isunique==true) //commit something already locked
                    {
                      rof.isnotaddunique = false;
                    }
                    else
                    {
                      rof.isnotaddunique = true;
                      rof.deleteindexentry = false;
                      rof.engineid = sRowRef.newengineid;
                      rof.rowid = sRowRef.newrowid;
                    }

                    class Field &fieldRef = tableRef.fields[n];

                    switch (fieldRef.type)
                    {
                      case CHARX:
                        trimspace(rof.fieldVal.str);
                        break;

                      case VARCHAR:
                        trimspace(rof.fieldVal.str);

                      default:
                        ;
                    }

                    addRof(getEngineid(&tableRef, n, &rof.fieldVal), rof, msgs);
                  }

                  // delete index entry
                  rof = blankRof;
                  rof.isrow = false;
                  rof.tableid = it->first.tableid;
                  rof.fieldid = n;
                  rof.isnotaddunique = true;
                  rof.deleteindexentry = true;
                  rof.engineid = sRowRef.originalengineid;
                  rof.rowid = sRowRef.originalrowid;

                  if (originalFieldValues[n].isnull==true)
                  {
                    rof.fieldVal.isnull=true;
                    addRof(n % nodeTopology.numpartitions, rof, msgs);

                    continue;
                  }

                  rof.fieldVal = originalFieldValues[n];
                  class Field &fieldRef = tableRef.fields[n];

                  switch (fieldRef.type)
                  {
                    case CHARX:
                      trimspace(rof.fieldVal.str);
                      break;

                    case VARCHAR:
                      trimspace(rof.fieldVal.str);

                    default:
                      ;
                  }

                  addRof(getEngineid(&tableRef, n, &rof.fieldVal), rof, msgs);
                }
                else     // replace index value, point to new rowid,engineid
                {
                  rof = blankRof;
                  rof.isrow = false;
                  rof.tableid = it->first.tableid;
                  rof.fieldid = n;
                  rof.fieldVal = originalFieldValues[n];
                  // need a rowOrField.replaceevalue flag
                  // and a function in Index::
                  rof.isreplace = true;
                  rof.isnotaddunique = true;
                  rof.rowid = sRowRef.originalrowid;
                  rof.engineid = sRowRef.originalengineid;
                  rof.newrowid = sRowRef.newrowid;
                  rof.newengineid = sRowRef.newengineid;

                  if (rof.fieldVal.isnull==true)
                  {
                    addRof(n % nodeTopology.numpartitions, rof, msgs);
                  }
                  else
                  {
                    class Field &fieldRef = tableRef.fields[n];

                    switch (fieldRef.type)
                    {
                      case CHARX:
                        trimspace(rof.fieldVal.str);
                        break;

                      case VARCHAR:
                        trimspace(rof.fieldVal.str);

                      default:
                        ;
                    }

                    addRof(getEngineid(&tableRef, n, &rof.fieldVal), rof, msgs);
                  }

                }
              }
            }
          }
          break;

          default:
            fprintf(logfile, "anomaly: %i %s %i\n", sRowRef.cmd, __FILE__,
                    __LINE__);
        }
      }

      // send to engines
      boost::unordered_map< int64_t, class MessageCommitRollback *>::iterator
          msgsIt;
      currentCmdState.engines = msgs.size();

      if (!currentCmdState.engines)
    {
        // nothing to do
        //        taPtr->Transactions.erase(transactionid);
        //        reenter(APISTATUS_OK);
        continueCommitTransaction(4);
        return;
      }

      for (msgsIt = msgs.begin(); msgsIt != msgs.end(); msgsIt++)
      {
        sendTransaction(COMMITCMD, PAYLOADCOMMITROLLBACK, 2, msgsIt->first,
                        (void *)msgsIt->second);
      }
    }
    break;

    case 2: // take responses, just count them down. if replace deletes, do
      // commit2
    {
      if (!(--currentCmdState.engines))
      {
        if (currentCmdState.replaceEngineMsgs.empty()==false)
        {
          currentCmdState.engines = currentCmdState.replaceEngineMsgs.size();
          boost::unordered_map<int64_t,
                class MessageCommitRollback *>::iterator it;

          for (it = currentCmdState.replaceEngineMsgs.begin();
               it != currentCmdState.replaceEngineMsgs.end(); it++)
          {
            sendTransaction(COMMITCMD, PAYLOADCOMMITROLLBACK, 3, it->first,
                            (void *)it->second);
          }
        }
        else
        {
          continueCommitTransaction(4);
        }
      }
    }
    break;

    case 3: // processing replacedelete engine responses
      if (--currentCmdState.engines)
      {
        return;
      }

      //            break; pass through to finish commit
    case 4: // gotta end the subtransactions TOPIC_ENDSUBTRANSACTION
    {
      map<int64_t, int64_t>::iterator it;
      class MessageSubtransactionCmd msg;
      msg.topic = TOPIC_ENDSUBTRANSACTION;
      msg.payloadtype = PAYLOADSUBTRANSACTION;

      for (it = engineToSubTransactionids.begin();
           it != engineToSubTransactionids.end(); it++)
      {
        if ((size_t)it->first > taPtr->myTopology.numpartitions || it->first < 0)
        {
          printf("%s %i anomaly %li %lu\n", __FILE__, __LINE__, it->first,
                 taPtr->myTopology.numpartitions);
          reenter(APISTATUS_NOTOK);
          return;
        }

        msg.subtransactionid = it->second;
        class MessageSubtransactionCmd *nmsg =
              new class MessageSubtransactionCmd;
        *nmsg=msg;
        taPtr->mboxes.toPartition(taPtr->myIdentity.address, it->first, *nmsg);
      }

#ifdef PROFILE
      profileEntry(__LINE__);
#endif
      reenter(APISTATUS_OK);
      return;
    }
    break;

    default:
      fprintf(logfile, "anomaly: %lu %s %i\n", entrypoint, __FILE__, __LINE__);
  }
}

void Transaction::rollback()
{
  boost::unordered_map< uuRecord_s, stagedRow_s >::iterator it;
  rowOrField_s blankRof = {};
  rowOrField_s rof;

  for (it = stagedRows.begin(); it != stagedRows.end(); it++)
  {
    stagedRow_s &sRowRef = it->second;
    rof = blankRof;
    rof.isrow = true;
    rof.tableid = it->first.tableid;

    if (sRowRef.cmd==UPDATE && ((it->first.rowid != sRowRef.newrowid) ||
                                (it->first.engineid != sRowRef.newengineid)))
    {
      // send rollback to new row if it's a replacement
      rof.rowid = sRowRef.newrowid;

      class MessageCommitRollback *msg = new class MessageCommitRollback();
      msg->rofs.push_back(rof);

      sendTransaction(ROLLBACKCMD, PAYLOADCOMMITROLLBACK, 0,
                      sRowRef.newengineid, (void *)msg);
    }

    rof.rowid = it->first.rowid;
    //        rofs = new vector<rowOrField>;
    class MessageCommitRollback *msg = new class MessageCommitRollback();
    msg->rofs.push_back(rof);
    sendTransaction(ROLLBACKCMD, PAYLOADCOMMITROLLBACK, 0,
                    it->first.engineid, (void *)msg);

    // now for indices (tableid already set above)
    rof.isrow = false;
    rof.isnotaddunique = false;
    rof.isreplace = false;

    boost::unordered_map< int64_t, lockFieldValue_s >::iterator itIndices;

    for (itIndices = it->second.uniqueIndices.begin();
         itIndices != it->second.uniqueIndices.end(); itIndices++)
    {
      rof.fieldid = itIndices->first;
      rof.fieldVal = itIndices->second.fieldVal;

      class MessageCommitRollback *msg = new class MessageCommitRollback();
      msg->rofs.push_back(rof);
      sendTransaction(ROLLBACKCMD, PAYLOADCOMMITROLLBACK, 0,
                      itIndices->second.engineid, (void *)msg);
    }
  }

  // tell the engines to kill their subtransactions
  map<int64_t, int64_t>::iterator itEngines;
  class MessageSubtransactionCmd msg;
  msg.topic = TOPIC_ENDSUBTRANSACTION;
  msg.payloadtype = PAYLOADSUBTRANSACTION;

  for (itEngines = engineToSubTransactionids.begin();
       itEngines != engineToSubTransactionids.end(); itEngines++)
  {
    msg.subtransactionid = itEngines->second;
    class MessageSubtransactionCmd *nmsg = new class MessageSubtransactionCmd;
    *nmsg = msg;

    taPtr->mboxes.toPartition(taPtr->myIdentity.address, itEngines->first,
                              *nmsg);
    //    taPtr->mboxes.engines[itEngines->first].send(msgsnd, true);
  }

  reenter(APISTATUS_OK);
  return;
}

// cmd is either ROLLBACKCMD or REVERTCMD to either rollback or revert
void Transaction::revertback(uuRecord_s &uur, enginecmd_e cmd)
{
  if (!stagedRows.count(uur))
  {
    return;
  }

  stagedRow_s &sRowRef = stagedRows[uur];
  rowOrField_s rof = {};

  rof.isrow = true;
  rof.tableid = uur.tableid;

  if (sRowRef.cmd==UPDATE && ((uur.rowid != sRowRef.newrowid) ||
                              (uur.engineid != sRowRef.newengineid)))
  {
    rof.rowid = sRowRef.newrowid;

    class MessageCommitRollback *msg = new class MessageCommitRollback();
    msg->rofs.push_back(rof);

    sendTransaction(cmd, PAYLOADCOMMITROLLBACK, 0,
                    sRowRef.newengineid, (void *)msg);
  }

  rof.rowid = uur.rowid;
  class MessageCommitRollback *msg = new class MessageCommitRollback();
  msg->rofs.push_back(rof);
  sendTransaction(ROLLBACKCMD, PAYLOADCOMMITROLLBACK, 0,
                  uur.engineid, (void *)msg);

  // indices
  rof.isrow = false;
  rof.isnotaddunique = false;
  rof.isreplace = false;
  boost::unordered_map< int64_t, lockFieldValue_s >::iterator it;

  for (it = sRowRef.uniqueIndices.begin(); it != sRowRef.uniqueIndices.end();
       it++)
  {
    rof.fieldid = it->first;
    rof.fieldVal = it->second.fieldVal;

    class MessageCommitRollback *msg = new class MessageCommitRollback();
    msg->rofs.push_back(rof);
    sendTransaction(ROLLBACKCMD, PAYLOADCOMMITROLLBACK, 0,
                    it->second.engineid, (void *)msg);
  }
}

void Transaction::reenter(int64_t res)
{
  if (reentryObject != NULL)
  {
    resultCode = res;
    pendingcmd = NOCOMMAND;
    pendingcmdid = 0;
    (*reentryObject.*reentryFuncPtr)(reentryCmd, reentryState);
  }
  else
  {
    delete this;
  }
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

void Transaction::zeroCurrentCmdState(void)
{
  currentCmdState.tableid = -1;
  currentCmdState.tablePtr = NULL;
  currentCmdState.indexEntries.clear();
  currentCmdState.rowEngineid = -1;
  currentCmdState.enginesWithUniqueIndices = 0;
  currentCmdState.rowid = 0;
  currentCmdState.engineid = -1;
  currentCmdState.locktype = NOLOCK;
  currentCmdState.row.clear();
  currentCmdState.fieldid = -1;
  currentCmdState.isunique = false;
  currentCmdState.destinationengineid = -1;
  currentCmdState.rowPtr = NULL;
  currentCmdState.fieldVal.isnull = false;
  currentCmdState.fieldVal.str.clear();
  memset(&currentCmdState.fieldVal.value, 0,
         sizeof(currentCmdState.fieldVal.value));
  currentCmdState.engines = 0;
}

int64_t Transaction::getEngineid(int64_t input)
{
  return SpookyHash::Hash64((void *) &input, sizeof(input), 0) %
         nodeTopology.numpartitions;
}

int64_t Transaction::getEngineid(uint64_t input)
{
  return SpookyHash::Hash64((void *) &input, sizeof(input), 0) %
         nodeTopology.numpartitions;
}

int64_t Transaction::getEngineid(bool input)
{
  return SpookyHash::Hash64((void *) &input, sizeof(input), 0) %
         nodeTopology.numpartitions;
}

int64_t Transaction::getEngineid(long double input)
{
  return SpookyHash::Hash64((void *) &input, sizeof(input), 0) %
         nodeTopology.numpartitions;
}

int64_t Transaction::getEngineid(char input)
{
  return SpookyHash::Hash64((void *) &input, sizeof(input), 0) %
         nodeTopology.numpartitions;
}

int64_t Transaction::getEngineid(string *input)
{
  trimspace(*input);
  return SpookyHash::Hash64((void *) input->c_str(), input->length(), 0) %
         nodeTopology.numpartitions;
}

void Transaction::badMessageHandler(void)
{
  printf("Transaction bad message stub %s %i\n", __FILE__, __LINE__); // stub
}

// for ApiInterface::insert()
void Transaction::addFieldToRow(void)
{
  fieldValue_s fieldVal = {};
  fieldVal.isnull = true;
  fieldValues.push_back(fieldVal);
}

void Transaction::addFieldToRow(int64_t val)
{
  fieldValue_s fieldVal = {};
  fieldVal.value.integer = val;
  fieldValues.push_back(fieldVal);
}

void Transaction::addFieldToRow(uint64_t val)
{
  fieldValue_s fieldVal = {};
  fieldVal.value.uinteger = val;
  fieldValues.push_back(fieldVal);
}

void Transaction::addFieldToRow(bool val)
{
  fieldValue_s fieldVal = {};
  fieldVal.value.boolean = val;
  fieldValues.push_back(fieldVal);
}

void Transaction::addFieldToRow(long double val)
{
  fieldValue_s fieldVal = {};
  fieldVal.value.floating = val;
  fieldValues.push_back(fieldVal);
}

void Transaction::addFieldToRow(char val)
{
  fieldValue_s fieldVal = {};
  fieldVal.value.character = val;
  fieldValues.push_back(fieldVal);
}

void Transaction::addFieldToRow(string &val)
{
  fieldValue_s fieldVal = {};
  fieldVal.str = val;
  fieldValues.push_back(fieldVal);
}

void Transaction::reenter(void)
{
  pendingcmd = NOCOMMAND;
  pendingcmdid = 0;
  (*reentryObject.*reentryFuncPtr)(reentryCmd, reentryState);
}

// returns true if field passes null constraint check, false if fails
// constraint check
bool Transaction::checkNullConstraintOK(int64_t fieldnum)
{
  if (currentCmdState.tablePtr->fields[fieldnum].indextype == UNIQUENOTNULL ||
      currentCmdState.tablePtr->fields[fieldnum].indextype ==
      NONUNIQUENOTNULL ||
      currentCmdState.tablePtr->fields[fieldnum].indextype ==UNORDEREDNOTNULL)
  {
    if (fieldValues[fieldnum].isnull==true)
    {
      return false;
    }
  }

  return true;
}

void Transaction::makeFieldValue(fieldValue_s *val, bool isnull, int64_t input)
{
  if (isnull==true)
  {
    val->isnull = true;
  }
  else
  {
    val->value.integer = input;
  }
}

void Transaction::makeFieldValue(fieldValue_s *val, bool isnull, uint64_t input)
{
  if (isnull==true)
  {
    val->isnull = true;
  }
  else
  {
    val->value.uinteger = input;
  }
}

void Transaction::makeFieldValue(fieldValue_s *val, bool isnull, bool input)
{
  if (isnull==true)
  {
    val->isnull = true;
  }
  else
  {
    val->value.boolean = input;
  }
}

void Transaction::makeFieldValue(fieldValue_s *val, bool isnull, long double input)
{
  if (isnull==true)
  {
    val->isnull = true;
  }
  else
  {
    val->value.floating = input;
  }
}

void Transaction::makeFieldValue(fieldValue_s *val, bool isnull, char input)
{
  if (isnull==true)
  {
    val->isnull = true;
  }
  else
  {
    val->value.character = input;
  }
}

void Transaction::makeFieldValue(fieldValue_s *val, bool isnull, string input)
{
  if (isnull==true)
  {
    val->isnull = true;
  }
  else
  {
    val->str = input;
  }
}

void Transaction::rollback(uuRecord_s &uur)
{
  revertback(uur, ROLLBACKCMD);
  reenter(APISTATUS_OK);
}

void Transaction::revert(uuRecord_s &uur)
{
  revertback(uur, REVERTCMD);
}

void Transaction::addRof(int64_t engineid, rowOrField_s &rof,
                         boost::unordered_map< int64_t, class MessageCommitRollback *> &msgs)
{
  if (!msgs.count(engineid))
  {
    msgs[engineid] = new class MessageCommitRollback();
  }

  msgs[engineid]->rofs.push_back(rof);
}

int64_t Transaction::getnextpendingcmdid(void)
{
  return ++nextpendingcmdid;
}

class MessageDispatch *Transaction::makeMessageDispatch()
{
  class MessageDispatch *msg = new class MessageDispatch;

  msg->transactionid = transactionid;
  msg->domainid = domainid;
  msg->pidsids = engineToSubTransactionids;
  boost::unordered_map< uuRecord_s, stagedRow_s >::iterator it;

  for (it = stagedRows.begin(); it != stagedRows.end(); it++)
  {
    const uuRecord_s &uurRef = it->first;
    stagedRow_s &srowRef = it->second;
    MessageDispatch::record_s r;

    if (srowRef.cmd==INSERT || srowRef.cmd==UPDATE || srowRef.cmd==DELETE)
    {
      r.primitive = srowRef.cmd;
      r.tableid = uurRef.tableid;
      r.previoussubtransactionid = srowRef.previoussubtransactionid;

      if (srowRef.cmd==INSERT || srowRef.cmd == UPDATE)
      {
        r.row = srowRef.newRow;
        r.rowid = srowRef.newrowid;

        if (srowRef.cmd==UPDATE)
        {
          r.oldrow = srowRef.originalRow;
        }
      }
      else // DELETE
      {
        r.row = srowRef.originalRow;
        r.rowid = srowRef.originalrowid;
      }

      msg->records[uurRef.engineid].push_back(r);
    }
  }

  if (!msg->records.size())
  {
    delete msg;
    return NULL;
  }

  return msg;
}

void Transaction::sqlPredicate(class Statement *statement,
                               operatortypes_e op, int64_t tableid, string &leftoperand,
                               string &rightoperand, locktype_e locktype, vector<fieldValue_s> &inValues,
                               void *continuationData, boost::unordered_map<uuRecord_s, returnRow_s> &results)
{
  sqlcmdstate = (sqlcmdstate_s)
  {
    0
  };
  sqlcmdstate.statement = statement;
  sqlcmdstate.results = &results;
  sqlcmdstate.locktype = locktype;
  sqlcmdstate.tableid = tableid;
  sqlcmdstate.continuationData = continuationData;

  if (pendingcmd != NOCOMMAND)
  {
    sqlcmdstate.statement->searchExpression(1, (class Ast *)sqlcmdstate.continuationData);
    return;
  }

  pendingcmdid = getnextpendingcmdid();
  pendingcmd = PRIMITIVE_SQLPREDICATE;

  string *fieldidoperand;

  if (op==OPERATOR_ISNULL || op==OPERATOR_ISNOTNULL)
  {
    fieldidoperand=&rightoperand;
  }
  else
  {
    fieldidoperand=&leftoperand;
  }

  string &fieldidoperandRef=*fieldidoperand;

  if (fieldidoperandRef[0] != OPERAND_FIELDID)
    //  if (leftoperand[0] != OPERAND_FIELDID)
  {
    printf("%s %i operand is not fieldid, it is %c\n", __FILE__, __LINE__,
           fieldidoperandRef[0]);

    //        leftoperand[0]);
    if (fieldidoperandRef[0]==OPERAND_IDENTIFIER)
      //    if (leftoperand[0]==OPERAND_IDENTIFIER)
    {
      printf("%s %i identifier: %s\n", __FILE__, __LINE__, fieldidoperandRef.substr(1, string::npos).c_str());
      //      printf("%s %i identifier: %s\n", __FILE__, __LINE__, leftoperand.substr(1, string::npos).c_str());
    }

    return;
  }

  int64_t fieldid;
  memcpy(&fieldid, &fieldidoperandRef[1], sizeof(fieldid));
  //  memcpy(&fieldid, &leftoperand[1], sizeof(fieldid));
  class Field &fieldRef = schemaPtr->tables[tableid]->fields[fieldid];

  searchParams_s searchParams = {};

  switch (op)
  {
    case OPERATOR_BETWEEN:
    {
      searchParams.values.resize(2, fieldValue_s());

      switch (rightoperand[0])
      {
        case OPERAND_STRING:
        {
          size_t len;
          memcpy(&len, &rightoperand[1], sizeof(len));

          if (fieldRef.type==CHAR)
          {
            searchParams.values[0].value.character =
              rightoperand[1+sizeof(int64_t)];
            searchParams.values[1].value.character =
              rightoperand[1+sizeof(int64_t)+len];
          }
          else
          {
            searchParams.values[0].str.resize(len, (char)0);
            searchParams.values[1].str.resize(rightoperand.size()-(1+sizeof(len)+len),
                                              (char)0);
            memcpy(&searchParams.values[0].str[0], &rightoperand[1+sizeof(len)],
                   len);
            memcpy(&searchParams.values[1].str[0], &rightoperand[1+sizeof(len)+len],
                   searchParams.values[1].str.size());
          }
        }
        break;

        case OPERAND_INTEGER:
          memcpy(&searchParams.values[0].value.integer,
                 &rightoperand[1], sizeof(int64_t));
          memcpy(&searchParams.values[1].value.integer,
                 &rightoperand[1+sizeof(int64_t)], sizeof(int64_t));
          break;

        case OPERAND_FLOAT:
          memcpy(&searchParams.values[0].value.floating,
                 &rightoperand[1], sizeof(long double));
          memcpy(&searchParams.values[1].value.floating,
                 &rightoperand[1+sizeof(long double)], sizeof(long double));
          break;

        default:
          printf("%s %i operand type %c not supported on rhs of predicate.\n",
                 __FILE__, __LINE__, rightoperand[0]);
          return;
      }
    }
    break;

    case OPERATOR_NOTBETWEEN:
    {
      searchParams.values.resize(2, fieldValue_s());

      switch (rightoperand[0])
      {
        case OPERAND_STRING:
        {
          size_t len;
          memcpy(&len, &rightoperand[1], sizeof(len));

          if (fieldRef.type==CHAR)
          {
            searchParams.values[0].value.character =
              rightoperand[1+sizeof(int64_t)];
            searchParams.values[1].value.character =
              rightoperand[1+sizeof(int64_t)+len];
          }
          else
          {
            searchParams.values[0].str =
              rightoperand.substr(1+sizeof(len), len);
            searchParams.values[1].str =
              rightoperand.substr(1+sizeof(len)+len, string::npos);
          }
        }
        break;

        case OPERAND_INTEGER:
          memcpy(&searchParams.values[0].value.integer,
                 &rightoperand[1], sizeof(int64_t));
          memcpy(&searchParams.values[1].value.integer,
                 &rightoperand[1+sizeof(int64_t)], sizeof(int64_t));
          break;

        case OPERAND_FLOAT:
          memcpy(&searchParams.values[0].value.floating,
                 &rightoperand[1], sizeof(long double));
          memcpy(&searchParams.values[1].value.floating,
                 &rightoperand[1+sizeof(long double)], sizeof(long double));
          break;

        default:
          printf("%s %i operand type %c not supported on rhs of predicate.\n",
                 __FILE__, __LINE__, rightoperand[0]);
          return;
      }
    }
    break;

    case OPERATOR_IN:
    {
      searchParams.values = inValues;
    }
    break;

    case OPERATOR_NOTIN:
    {
      searchParams.values = inValues;
    }
    break;

    case OPERATOR_ISNULL:
      // unary operator, don't do anything, but don't do default either
      break;

    case OPERATOR_ISNOTNULL:
      // unary operator, don't do anything, but don't do default either
      break;

    default:
    {
      searchParams.values.push_back(fieldValue_s {});

      switch (rightoperand[0])
      {
        case OPERAND_STRING:
          if (fieldRef.type==CHAR)
          {
            searchParams.values[0].value.character = rightoperand[1];
          }
          else
          {
            searchParams.values[0].str = rightoperand.substr(1, string::npos);
          }

          break;

        case OPERAND_INTEGER:
          memcpy(&searchParams.values[0].value.integer, &rightoperand[1],
                 sizeof(int64_t));
          break;

        case OPERAND_BOOLEAN:
          if (rightoperand[1]=='t')
          {
            searchParams.values[0].value.boolean=true;
          }
          else
          {
            searchParams.values[0].value.boolean=false;
          }

          break;

        case OPERAND_FLOAT:
          memcpy(&searchParams.values[0].value.floating, &rightoperand[1],
                 sizeof(long double));
          break;

        default:
          printf("%s %i operand type %c not supported on rhs of predicate.\n",
                 __FILE__, __LINE__, rightoperand[0]);
          return;
      }
    }
  }

  if (op==OPERATOR_EQ)
  {
    // send to only single engine based on hashval
    fieldtype_e fieldtype = schemaPtr->tables[tableid]->fields[fieldid].type;
    volatile int64_t destengineid = -1;
    sqlcmdstate.eventwaitcount = 1;

    switch (fieldtype)
    {
      case INT:
        destengineid =
          getEngineid(searchParams.values[0].value.integer);
        break;

      case UINT:
        destengineid =
          getEngineid(searchParams.values[0].value.uinteger);
        break;

      case BOOL:
        destengineid =
          getEngineid(searchParams.values[0].value.boolean);
        break;

      case FLOAT:
        destengineid =
          getEngineid(searchParams.values[0].value.floating);
        //        printf("%s %i sqlPredicate(%Lf) destengineid %li\n", __FILE__, __LINE__, searchParams.values[0].value.floating, destengineid);
        break;

      case CHAR:
        destengineid =
          getEngineid(searchParams.values[0].value.character);
        break;

      case CHARX:
        destengineid =
          getEngineid(&searchParams.values[0].str);
        break;

      case VARCHAR:
        destengineid =
          getEngineid(&searchParams.values[0].str);
        break;

      default:
        printf("%s %i anomaly %i\n", __FILE__, __LINE__, fieldtype);
        return;
    }

    if (fieldid==0 &&
        schemaPtr->tables[tableid]->fields[fieldid].index.isunique == true)
    {
      class MessageSubtransactionCmd *msg =
            new class MessageSubtransactionCmd();
      msg->cmd.tableid = tableid;
      msg->cmd.fieldid = fieldid;
      msg->cmd.locktype = locktype;
      searchParams.op = op;
      msg->cmd.searchParameters = searchParams;
      sendTransaction(SEARCHRETURN1, PAYLOADSUBTRANSACTION, 2, destengineid,
                      msg);
    }
    else
  {
      class MessageSubtransactionCmd *msg =
            new class MessageSubtransactionCmd();
      msg->cmd.tableid = tableid;
      msg->cmd.fieldid = fieldid;
      searchParams.op = op;
      msg->cmd.searchParameters = searchParams;
      sendTransaction(INDEXSEARCH, PAYLOADSUBTRANSACTION, 1,
                      destengineid, msg);
    }
  }
  else
{
    class MessageSubtransactionCmd msg;
    msg.cmd.tableid = tableid;
    msg.cmd.fieldid = fieldid;
    searchParams.op = op;
    msg.cmd.searchParameters = searchParams;

    if (msg.cmd.searchParameters.op==OPERATOR_ISNULL)
    {
      sqlcmdstate.eventwaitcount=1;
      class MessageSubtransactionCmd *nmsg = new class MessageSubtransactionCmd;
      *nmsg = msg;
      sendTransaction(INDEXSEARCH, PAYLOADSUBTRANSACTION, 1,
                      fieldid % nodeTopology.numpartitions, nmsg);
    }
    else
    {
      sqlcmdstate.eventwaitcount = nodeTopology.numpartitions;

      for (int n=0; n < sqlcmdstate.eventwaitcount; n++)
      {
        class MessageSubtransactionCmd *nmsg =
              new class MessageSubtransactionCmd;
        *nmsg = msg;
        sendTransaction(INDEXSEARCH, PAYLOADSUBTRANSACTION, 1,
                        n, nmsg);
      }
    }
  }
}

void Transaction::continueSqlPredicate(int64_t entrypoint)
{
  class MessageSubtransactionCmd &subtransactionCmdRef =
        *((MessageSubtransactionCmd *)msgrcv);

  switch (entrypoint)
{
    case 1:
    {
      sqlcmdstate.indexHits.insert(sqlcmdstate.indexHits.end(),
                                   subtransactionCmdRef.cmd.indexHits.begin(),
                                   subtransactionCmdRef.cmd.indexHits.end());

      if (--sqlcmdstate.eventwaitcount == 0)
      {
        if (sqlcmdstate.indexHits.empty()==true)
        {
          // no rows returned
          pendingcmd = NOCOMMAND;
          pendingcmdid = 0;

          if (pendingcmd==PRIMITIVE_SQLPREDICATE)
          {
            sqlcmdstate.statement->searchExpression(1, (class Ast *)sqlcmdstate.continuationData);
          }
          else
          {
            // SQLSELECTALL
            sqlcmdstate.statement->continueSelect(1, NULL);
          }

          return;
        }

        if (sqlcmdstate.indexHits.size()==1)
        {
          // send just 1 message to retrieve 1 row
          sqlcmdstate.eventwaitcount = 1;
          class MessageSubtransactionCmd *msg =
                new class MessageSubtransactionCmd();
          msg->cmd.tableid = sqlcmdstate.tableid;
          msg->cmd.locktype = sqlcmdstate.locktype;
          indexEntry_s &hit = sqlcmdstate.indexHits[0];
          msg->cmd.rowids.push_back(hit.rowid);
          sendTransaction(SELECTROWS, PAYLOADSUBTRANSACTION, 2,
                          hit.engineid, msg);
        }
        else
      {
          /* map of engineids to vectors of rowids */
          boost::unordered_map< int64_t, vector<int64_t> > payloads;

          for (size_t n=0; n < sqlcmdstate.indexHits.size(); n++)
          {
            indexEntry_s &hit = sqlcmdstate.indexHits[n];
            payloads[hit.engineid].push_back(hit.rowid);
          }

          sqlcmdstate.eventwaitcount = payloads.size();
          boost::unordered_map< int64_t, vector<int64_t> >::iterator it;

          for (it = payloads.begin(); it != payloads.end(); it++)
          {
            class MessageSubtransactionCmd *msg =
                  new class MessageSubtransactionCmd();
            msg->cmd.tableid = sqlcmdstate.tableid;
            msg->cmd.locktype = sqlcmdstate.locktype;
            msg->cmd.rowids = it->second;
            sendTransaction(SELECTROWS, PAYLOADSUBTRANSACTION, 2,
                            it->first, (void *)msg);
          }
        }
      }
    }
    break;

    case 2:
  {
      boost::unordered_map<uuRecord_s, returnRow_s> &resultsRef =
        *sqlcmdstate.results;

      uuRecord_s uur = {-1, sqlcmdstate.tableid,
                        subtransactionCmdRef.engineinstance
                       };
      bool islockchange = false;

      for (size_t n=0; n < subtransactionCmdRef.cmd.returnRows.size(); n++)
      {
        returnRow_s &returnrowRef = subtransactionCmdRef.cmd.returnRows[n];
        uur.rowid = returnrowRef.rowid;

        switch (returnrowRef.locktype)
        {
          case NOLOCK:
            break;

          case READLOCK:
            break;

          case WRITELOCK:
            break;

          case PENDINGLOCK:
            // abort if lock pending for now, but make backlog (6/26/2013)
            sqlcmdstate.statement->abortQuery(APISTATUS_LOCK);
            return;
            break;

          case PENDINGTOWRITELOCK:
            // abort if lock pending for now, but make backlog (6/26/2013)
            sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
            return;
            break;

          case PENDINGTOREADLOCK:
            // abort if lock pending for now, but make backlog (6/26/2013)
            sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
            return;
            break;

          case PENDINGTONOLOCK:
            // abort if lock pending for now, but make backlog (6/26/2013)
            sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
            return;
            break;

          case NOTFOUNDLOCK:
            continue;
            break;

          default:
            fprintf(logfile, "anomaly: %i %s %i\n", returnrowRef.locktype, __FILE__,
                    __LINE__);
            continue;
        }

        resultsRef[uur] = returnrowRef;
      }

      if (islockchange==false)
      {
        sqlcmdstate.eventwaitcount--;
      }

      if (sqlcmdstate.eventwaitcount==0 && lockpendingcount==0)
      {
        // re-enter, the statement is finished
        switch (pendingcmd)
        {
          case PRIMITIVE_SQLPREDICATE:
            pendingcmd = NOCOMMAND;
            pendingcmdid = 0;
            sqlcmdstate.statement->searchExpression(1,
                                                    (class Ast *)sqlcmdstate.continuationData);
            break;

          case PRIMITIVE_SQLSELECTALL:
            pendingcmd = NOCOMMAND;
            pendingcmdid = 0;
            sqlcmdstate.statement->continueSelect(1, NULL);
            break;

          case PRIMITIVE_SQLSELECTALLFORDELETE:
            pendingcmd = NOCOMMAND;
            pendingcmdid = 0;
            sqlcmdstate.statement->continueDelete(1, NULL);
            break;

          case PRIMITIVE_SQLSELECTALLFORUPDATE:
            pendingcmd = NOCOMMAND;
            pendingcmdid = 0;
            sqlcmdstate.statement->continueUpdate(1, NULL);
            break;

          default:
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, pendingcmd);
        }
      }
    }
    break;

    default:
      printf("%s %i anomaly %li\n", __FILE__, __LINE__, entrypoint);
      return;
  }
}

void Transaction::sqlSelectAll(class Statement *statement, int64_t tableid,
                               locktype_e locktype, pendingprimitive_e pendingprimitive,
                               boost::unordered_map<uuRecord_s, returnRow_s> &results)
{
  sqlcmdstate = (sqlcmdstate_s)
  {
    0
  };
  sqlcmdstate.statement = statement;
  sqlcmdstate.results = &results;
  sqlcmdstate.locktype = locktype;
  sqlcmdstate.tableid = tableid;

  if (pendingcmd != NOCOMMAND)
  {
    sqlcmdstate.statement->continueSelect(1, NULL);
    return;
  }

  pendingcmdid = getnextpendingcmdid();
  pendingcmd = pendingprimitive;

  class MessageSubtransactionCmd msg;
  msg.cmd.tableid = tableid;
  msg.cmd.fieldid = 0;
  msg.cmd.locktype = locktype;
  msg.cmd.searchParameters.op = OPERATOR_SELECTALL;

  sqlcmdstate.eventwaitcount=nodeTopology.numpartitions;

  for (int64_t n=0; n < sqlcmdstate.eventwaitcount; n++)
  {
    class MessageSubtransactionCmd *nmsg =
          new class MessageSubtransactionCmd;
    *nmsg = msg;
    sendTransaction(INDEXSEARCH, PAYLOADSUBTRANSACTION, 1, n, nmsg);
  }
}

void Transaction::continueSqlDelete(int64_t entrypoint)
{
  class MessageSubtransactionCmd &msgrcvRef =
        *((MessageSubtransactionCmd *)msgrcv);

  if (pendingcmdid != msgrcvRef.transaction_pendingcmdid)
{
    badMessageHandler();
    return;
  }

  sqlcmdstate.eventwaitcount--;

  if (msgrcvRef.cmd.status != STATUS_OK)
  {
    sqlcmdstate.statement->abortQuery(msgrcvRef.cmd.status);
    return;
  }

  if (!sqlcmdstate.eventwaitcount)
  {
    sqlcmdstate.statement->continueDelete(2, NULL);
  }
}

void Transaction::continueSqlInsert(int64_t entrypoint)
{
  class MessageSubtransactionCmd &msgrcvRef =
        *(MessageSubtransactionCmd *)msgrcv;

  switch (entrypoint)
{
    case 1:
    {
      sqlcmdstate.statement->currentQuery->results.newrowuur =
      {
        msgrcvRef.cmd.rowid, sqlcmdstate.statement->currentQuery->tableid,
        sqlcmdstate.statement->currentQuery->results.newrowengineid
      };
      stagedRow_s newStagedRow = {};
      newStagedRow.newRow = sqlcmdstate.statement->currentQuery->results.newrow;
      newStagedRow.newrowid = sqlcmdstate.statement->currentQuery->results.newrowuur.rowid;
      newStagedRow.locktype = WRITELOCK;
      newStagedRow.cmd=INSERT;

      class Table &tableRef =
            *schemaPtr->tables[sqlcmdstate.statement->currentQuery->results.newrowuur.tableid];

      for (size_t n=0; n < tableRef.fields.size(); n++)
    {
        // nonunique indices are handled in commit
        if (tableRef.fields[n].index.isunique != true ||
            sqlcmdstate.statement->currentQuery->results.insertValues[n].isnull==true)
        {
          continue;
        }

        lockFieldValue_s lockFieldValue = {};
        lockFieldValue.engineid = getengine(tableRef.fields[n].type,
                                            sqlcmdstate.statement->currentQuery->results.insertValues[n]);
        // locktype could potentially change
        lockFieldValue.locktype = INDEXLOCK;
        lockFieldValue.fieldVal = sqlcmdstate.statement->currentQuery->results.insertValues[n];
        newStagedRow.uniqueIndices[n]=lockFieldValue;

        sqlcmdstate.eventwaitcount++;
        class MessageSubtransactionCmd *msg =
              new class MessageSubtransactionCmd();
        msg->cmd.tableid =
          sqlcmdstate.statement->currentQuery->results.newrowuur.tableid;
        msg->cmd.rowid =
          sqlcmdstate.statement->currentQuery->results.newrowuur.rowid;
        msg->cmd.engineid =
          sqlcmdstate.statement->currentQuery->results.newrowuur.engineid;
        msg->cmd.fieldid = n;
        msg->cmd.fieldVal =
          sqlcmdstate.statement->currentQuery->results.insertValues[n];

        sendTransaction(UNIQUEINDEX, PAYLOADSUBTRANSACTION, 2,
                        lockFieldValue.engineid, msg);
      }

      stagedRows[sqlcmdstate.statement->currentQuery->results.newrowuur] =
        newStagedRow;

      if (sqlcmdstate.eventwaitcount)
    {
        // come back for responses
        return;
      }
    }
    break;

    case 2:
    {
      switch (msgrcvRef.cmd.locktype)
      {
        case NOLOCK: // constraint violation, abort command
          sqlcmdstate.statement->abortQuery(APISTATUS_UNIQUECONSTRAINT);
          return;
          break;

        case INDEXLOCK:
          break;

        case INDEXPENDINGLOCK:
          sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
          fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
          return;
          break;

        case PENDINGTOINDEXLOCK:
          sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
          fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
          return;
          break;

        case PENDINGTOINDEXNOLOCK: // unique constraint violation
          sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
          fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
          return;
          break;

        default:
          fprintf(logfile, "anomaly: %i %s %i\n", msgrcvRef.cmd.locktype,
                  __FILE__, __LINE__);
          sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
          return;
      }

      if (--sqlcmdstate.eventwaitcount)
      {
        return;
      }
    }
    break;

    default:
      printf("%s %i anomaly %li\n", __FILE__, __LINE__, entrypoint);
  }

  // all responses returned, if any, so finish statement
  sqlcmdstate.statement->continueInsert(1, NULL);
}

void Transaction::continueSqlUpdate(int64_t entrypoint)
{
  // only 1 entrypoint
  class MessageSubtransactionCmd &msgrcvRef =
        *(MessageSubtransactionCmd *)msgrcv;

  switch (msgrcvRef.cmd.locktype)
{
    case WRITELOCK: // insertrow
      break;

    case NOLOCK:
      printf("%s %i APISTATUS_UNIQUECONSTRAINT (NOLOCK)\n", __FILE__, __LINE__);
      sqlcmdstate.statement->abortQuery(APISTATUS_UNIQUECONSTRAINT);
      return;
      break;

    case INDEXLOCK:
      break;

    case INDEXPENDINGLOCK:
      sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
      return;
      break;

    case PENDINGTOINDEXLOCK:
      sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
      return;
      break;

    case PENDINGTOINDEXNOLOCK: // unique constraint violation
      sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
      return;
      break;

    default:
      printf("%s %i locktype %i\n", __FILE__, __LINE__, msgrcvRef.cmd.locktype);
      sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
      return;
  }

  if (--sqlcmdstate.eventwaitcount)
  {
    return;
  }

  pendingcmd=NOCOMMAND;
  sqlcmdstate.statement->continueUpdate(2, NULL);
}

void Transaction::continueSqlReplace(int64_t entrypoint)
{
  class MessageSubtransactionCmd &msgrcvRef =
        *(MessageSubtransactionCmd *)msgrcv;

  switch (entrypoint)
{
    case 1:
    {
      stagedRow_s &stagedRowRef =
        stagedRows[sqlcmdstate.statement->currentQuery->results.originalrowuur];
      stagedRowRef.newrowid=msgrcvRef.cmd.rowid;

      sqlcmdstate.statement->currentQuery->results.newrowuur =
      {
        msgrcvRef.cmd.rowid, sqlcmdstate.statement->currentQuery->tableid,
        sqlcmdstate.statement->currentQuery->results.newrowengineid
      };

      // now delete the old row, with forwarder
      class MessageSubtransactionCmd *msg =
            new class MessageSubtransactionCmd();
      msg->cmd.tableid =
        sqlcmdstate.statement->currentQuery->results.newrowuur.tableid;
      msg->cmd.rowid = stagedRowRef.originalrowid;
      msg->cmd.forward_rowid = stagedRowRef.newrowid;
      msg->cmd.forward_engineid = stagedRowRef.newengineid;
      sendTransaction(REPLACEDELETEROW, PAYLOADSUBTRANSACTION, 2,
                      stagedRowRef.originalengineid, msg);
      sqlcmdstate.eventwaitcount++;

      // indices for all fields
    class Table &tableRef =
            *schemaPtr->tables[sqlcmdstate.statement->currentQuery->tableid];
      vector<fieldValue_s> fieldValues;
      //      tableRef.unmakerow(&sqlcmdstate.statement->currentQuery->results.newrow,
      tableRef.unmakerow(&stagedRowRef.newRow, &fieldValues);

      for (size_t n=0; n < fieldValues.size(); n++)
    {
        class Field &fieldRef = tableRef.fields[n];

        if (fieldRef.indextype==NONE)
        {
          continue;
        }

        if (fieldRef.index.isunique==true &&
            sqlcmdstate.statement->currentQuery->results.setFields.count(n))
        {
          // update, new entry, sendTransaction, stagedRows.uniqueIndices
          sqlcmdstate.eventwaitcount++;

          lockFieldValue_s lockFieldValue = {};
          lockFieldValue.engineid = getengine(fieldRef.type, fieldValues[n]);
          // locktype could potentially change
          lockFieldValue.locktype = INDEXLOCK;
          lockFieldValue.fieldVal = fieldValues[n];
          stagedRowRef.uniqueIndices[n]=lockFieldValue;

          class MessageSubtransactionCmd *msg =
                new class MessageSubtransactionCmd();
          msg->cmd.isrow = false;
          msg->cmd.fieldVal = fieldValues[n];
          msg->cmd.tableid = sqlcmdstate.statement->currentQuery->tableid;
          //              sqlcmdstate.statement->currentQuery->results.newrowuur.tableid;
          msg->cmd.fieldid = n;
          msg->cmd.rowid = stagedRowRef.newrowid;
          msg->cmd.engineid = stagedRowRef.newengineid;
          sendTransaction(UNIQUEINDEX, PAYLOADSUBTRANSACTION, 2,
                          lockFieldValue.engineid, msg);
        }
      }
    }
    break;

    case 2:
  {
      // like continueSqlUpdate(1)
      switch (msgrcvRef.cmd.locktype)
      {
        case WRITELOCK: // insertrow
          break;

        case NOLOCK:
          sqlcmdstate.statement->abortQuery(APISTATUS_UNIQUECONSTRAINT);
          return;
          break;

        case INDEXLOCK:
          break;

        case INDEXPENDINGLOCK:
          sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
          return;
          break;

        case PENDINGTOINDEXLOCK:
          sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
          return;
          break;

        case PENDINGTOINDEXNOLOCK: // unique constraint violation
          sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
          return;
          break;

        default:
          printf("%s %i locktype %i\n", __FILE__, __LINE__, msgrcvRef.cmd.locktype);
          sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
          return;
      }

      if (--sqlcmdstate.eventwaitcount)
      {
        return;
      }

      pendingcmd=NOCOMMAND;
      sqlcmdstate.statement->continueUpdate(2, NULL);
    }
    break;

    default:
      printf("%s %i anomaly %li\n", __FILE__, __LINE__, entrypoint);
      sqlcmdstate.statement->abortQuery(STATUS_NOTOK);
      return;
  }
}

void Transaction::checkSqlLock(deadlockchange_e changetype, bool isrow,
                               int64_t rowid, int64_t tableid, int64_t engineid, int64_t fieldid,
                               fieldValue_s *fieldVal)
{
  switch (changetype)
  {
    case ADDLOCKEDENTRY:
      lockcount++;
      break;

    case ADDLOCKPENDINGENTRY:
      lockpendingcount++;
      break;

    case REMOVELOCKEDENTRY:
      lockcount--;
      break;

    case REMOVELOCKPENDINGENTRY:
      lockpendingcount--;
      break;

    case TRANSITIONPENDINGTOLOCKEDENTRY:
      lockpendingcount--;
      lockcount++;
      break;

    default:
      fprintf(logfile, "anomaly %i %s %i\n", changetype, __FILE__, __LINE__);
  }

  // OK, now, what? I'm either in deadlock and sending another message
  // not deadlocked, not deadlocked
  // deadlocked, then not
  // not deadlocked, then deadlocked
  if (sqlcmdstate.ispossibledeadlock==false &&
      (!lockcount || !lockpendingcount))
  {
    // move along
    return;
  }

  if (sqlcmdstate.ispossibledeadlock==false && lockpendingcount
      && lockcount)
  {
    // new deadlock!
    sqlcmdstate.ispossibledeadlock = true;
    // send a bunch of messages, in pendingMapofRows, then the input
    class MessageDeadlock *msg = new class MessageDeadlock;
    class MessageDeadlock &msgref = *msg;
    newDeadLockLists_s &nodesRef = msgref.nodes;

    boost::unordered_map< uuRecord_s, stagedRow_s >::iterator it;

    for (it = currentCmdState.pendingStagedRows.begin();
         it != currentCmdState.pendingStagedRows.end(); it++)
    {
      stagedRow_s &sRowRef = it->second;
      int64_t this_rowid = it->first.rowid;
      int64_t this_tableid = it->first.tableid;
      int64_t this_engineid = it->first.engineid;
      string deadlockNode;
      // free this if there's no message to send (no contents in its sets)

      // row first
      DeadlockMgr::makeLockedItem(true, this_rowid, this_tableid,
                                  this_engineid, domainid, 0, (long double)0, (string *)NULL,
                                  &deadlockNode);

      if (sRowRef.locktype==WRITELOCK || sRowRef.locktype==READLOCK)
      {
        nodesRef.locked.insert(deadlockNode);
      }
      else if (sRowRef.locktype==PENDINGLOCK)
      {
        nodesRef.waiting.insert(deadlockNode);
      }

      // indices
      boost::unordered_map< int64_t, lockFieldValue_s >::iterator it;

      for (it = sRowRef.uniqueIndices.begin();
           it != sRowRef.uniqueIndices.end(); it++)
      {
        lockFieldValue_s &lockFieldValueRef = it->second;

        if (lockFieldValueRef.locktype==INDEXLOCK)
        {
          deadlockNode.clear();
          long double fieldinput;
          memcpy(&fieldinput, &lockFieldValueRef.fieldVal.value,
                 sizeof(fieldinput));
          DeadlockMgr::makeLockedItem(false, 0, this_tableid,
                                      this_engineid, domainid, it->first, fieldinput,
                                      &lockFieldValueRef.fieldVal.str, &deadlockNode);
          nodesRef.locked.insert(deadlockNode);
        }
        else if (lockFieldValueRef.locktype==INDEXPENDINGLOCK)
        {
          deadlockNode.clear();
          long double fieldinput;
          memcpy(&fieldinput, &lockFieldValueRef.fieldVal.value,
                 sizeof(fieldinput));
          DeadlockMgr::makeLockedItem(false, 0, this_tableid,
                                      this_engineid, domainid, it->first, fieldinput,
                                      &lockFieldValueRef.fieldVal.str, &deadlockNode);
          nodesRef.waiting.insert(deadlockNode);
        }
      }
    }

    if (nodesRef.locked.empty()==true && nodesRef.waiting.empty()==true)
    {
      // delete nodes; // nothing to deadlock, but this should be an anomaly
      delete msg;
      fprintf(logfile, "anomaly: %s %i\n", __FILE__, __LINE__);
      return;
    }

    msgref.topic = TOPIC_DEADLOCKNEW;
    msgref.transactionid = transactionid;
    msgref.tainstance = taPtr->instance;
    msgref.transaction_pendingcmdid = pendingcmdid;

    //    taPtr->mboxes.deadlockMgr.send(msgsnd, true);
    taPtr->mboxes.toDeadlockMgr(taPtr->myIdentity.address, *msg);
    return;
  }

  if (lockcount && lockpendingcount)
  {
    // still deadlocked, just send 1 message based on type
    class MessageDeadlock *msg = new class MessageDeadlock();
    class MessageDeadlock &msgref = *msg;

    // prepare string(s) for submittal
    if (isrow==true)
    {
      DeadlockMgr::makeLockedItem(true, rowid, tableid, engineid, domainid,
                                  fieldid, (long double)0, (string *)NULL, &msgref.deadlockNode);
    }
    else
    {
      long double fieldinput;
      memcpy(&fieldinput, &fieldVal->value, sizeof(fieldinput));
      DeadlockMgr::makeLockedItem(false, rowid, tableid, engineid, domainid,
                                  fieldid, fieldinput, &fieldVal->str, &msgref.deadlockNode);
    }

    // send message to dmgr
    msgref.topic = TOPIC_DEADLOCKCHANGE;
    msgref.deadlockchange = changetype;
    msgref.transactionid = transactionid;

    taPtr->mboxes.toDeadlockMgr(taPtr->myIdentity.address, *msg);

    return;
  }

  if (!lockcount || !lockpendingcount)
  {
    // deadlock over, send message to dmgr to that effect
    currentCmdState.ispossibledeadlock = false;

    // send message to dmgr
    class MessageDeadlock *msg = new class MessageDeadlock;
    class MessageDeadlock &msgref = *msg;
    msgref.topic = TOPIC_DEADLOCKREMOVE;
    msgref.transactionid = transactionid;

    //    taPtr->mboxes.deadlockMgr.send(msgsnd, true);
    taPtr->mboxes.toDeadlockMgr(taPtr->myIdentity.address, *msg);
    return;
  }
}

void Transaction::commit()
{
  pendingcmdid = getnextpendingcmdid();
  pendingcmd = COMMIT;
  //dispatch a message to the secondary node(s), then execute the continueCommit
  waitfordispatched = taPtr->myTopology.numreplicas-1;

  switch (taPtr->myTopology.numreplicas)
  {
    case 1:
      waitfordispatched = 1;
      continueCommitTransaction(1);
      break;

    case 2:
    {
      // make message, then send it
      class MessageDispatch *msg = makeMessageDispatch();

      if (msg != NULL)
      {
        taPtr->mboxes.toActor(taPtr->myIdentity.address, taPtr->replicaAddress,
                              *msg);
      }
    }
    break;

    default: // more than 2 replicas
    {
      // make message then copy and send for each replica
      class MessageDispatch *msg = makeMessageDispatch();

      if (msg != NULL)
      {
        for (size_t n=1; n < taPtr->replicaAddresses.size(); n++)
        {
          class MessageDispatch *nmsg = new class MessageDispatch;
          *nmsg = *msg;
          taPtr->mboxes.toActor(taPtr->myIdentity.address,
                                taPtr->replicaAddresses[n], *nmsg);
        }

        taPtr->mboxes.toActor(taPtr->myIdentity.address,
                              taPtr->replicaAddresses[taPtr->replicaAddresses.size()-1], *msg);
      }
    }
  }
}
