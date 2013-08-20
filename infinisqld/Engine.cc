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

#include "infinisql_Engine.h"
#line 28 "Engine.cc"

Engine::Engine(Topology::partitionAddress *myIdentityArg) :
  myIdentity(*myIdentityArg)
{
  delete myIdentityArg;
  instance = myIdentity.instance;

#ifdef PROFILE
  rid = 0; // profiling
  profilecount = 0; //profiling
  profiles = new PROFILERENGINECOMPREHENSIVE[PROFILEENGINEENTRIES];
#endif
  class Mbox &mymbox = *myIdentity.mbox;
  mboxes.nodeid = myIdentity.address.nodeid;
  mboxes.update(myTopology, instance);
  getMyPartitionid();

  int64_t builtincmd = 0;
  int waitfor = 100;

  /** enter message receive event loop */
  while (1)
  {
#ifdef PROFILE
    gettimeofday(&inboundProfile[0].tv, NULL);
    inboundProfile[0].tag = 0;
#endif

    for (size_t inmsg=0; inmsg < 50; inmsg++)
    {
      msgrcv = mymbox.receive(waitfor);

      if (msgrcv==NULL)
      {
        waitfor = 100;
        break;
      }

      waitfor = 0;

      if (msgrcv->payloadtype==PAYLOADUSERSCHEMA)
      {
        class MessageUserSchema &msgrcvRef =
              *(class MessageUserSchema *)msgrcv;

        argsize = msgrcvRef.argsize;
        taAddr = msgrcvRef.sourceAddr;
        //        tainstance = msgrcvRef.instance;
        operationid = msgrcvRef.operationid;
        domainid = msgrcvRef.domainid;
        userid = msgrcvRef.userid;
        builtincmd = msgrcvRef.builtincmd;
      }

      switch (msgrcv->topic)
    {
        case TOPIC_SCHEMAREQUEST:
          switch (builtincmd)
          {
            case BUILTINCREATESCHEMA:
              createschema();
              break;

            case BUILTINCREATETABLE:
              createtable();
              break;

            case BUILTINADDCOLUMN:
              addcolumn();
              break;

            case BUILTINDELETEINDEX:
              deleteindex();
              break;

            case BUILTINDELETETABLE:
              deletetable();
              break;

            case BUILTINDELETESCHEMA:
              deleteschema();
              break;

            default:
              fprintf(logfile, "Engine bad schema builtin %li %s %i\n",
                      builtincmd, __FILE__, __LINE__);
          }

          break;

        case TOPIC_TRANSACTION:
        {
          class MessageTransaction &msgrcvRef =
                *(class MessageTransaction *)msgrcv;
#ifdef PROFILE
          gettimeofday(&inboundProfile[1].tv, NULL);
          inboundProfile[1].tag = 1;
#endif

          // create SubTransaction if no subtransaction
          if (msgrcvRef.subtransactionid <= 0)
        {
            class SubTransaction *subTransactionidPtr =
                new class SubTransaction(msgrcvRef.sourceAddr,
                                             msgrcvRef.transactionid, msgrcvRef.domainid, this);
            subTransactionidPtr->processTransactionMessage(msgrcv);
          }
          else if (SubTransactions.count(msgrcvRef.subtransactionid))
        {
            SubTransactions[msgrcvRef.subtransactionid]->processTransactionMessage(msgrcv);
          }
        }
        break;

        case TOPIC_ENDSUBTRANSACTION:
        {
          class MessageTransaction &msgrcvRef =
                *(class MessageTransaction *)msgrcv;

          if (SubTransactions.count(msgrcvRef.subtransactionid))
        {
#ifdef PROFILE
            class SubTransaction &subTransactionRef =
                  *SubTransactions[msgrcv.subtransactionid];
            subTransactionRef.profileEntry(999999);

            PROFILERENGINECOMPREHENSIVE p;
            p.rid = subTransactionRef.rid;
            p.subtransactionid = msgrcv.subtransactionid;
            p.subtransactionpointcount =
              subTransactionRef.subtransactionpointcount;
            p.subtransactionpoints =
              subTransactionRef.subtransactionpoints;

            profiles[profilecount++ % PROFILEENGINEENTRIES] = p;
#endif
            delete SubTransactions[msgrcvRef.subtransactionid];
          }
        }
        break;
#ifdef PROFILE

        case TOPIC_PROFILE:
      {
          int start = profilecount < PROFILEENGINEENTRIES ? 0 :
                      (profilecount+1) % PROFILEENGINEENTRIES;
          int stop = profilecount < PROFILEENGINEENTRIES ? profilecount :
                     profilecount % PROFILEENGINEENTRIES;
          std::stringstream fname;
          fname << "profile/engine_" << instance << ".txt";
          FILE *fp = fopen((char *)fname.str().c_str(), "w");

          for (int n=start; n % PROFILEENGINEENTRIES != stop; n++)
          {
            int pos = n % PROFILEENGINEENTRIES;
            std::stringstream outstream;
            outstream << instance << "\t" << profiles[pos].subtransactionid <<
                      "\t" << profiles[pos].rid << "\t";

            for (int m=0; m < profiles[pos].subtransactionpointcount-1; m++)
            {
              outstream << profiles[pos].subtransactionpoints[m].tag << ",";
            }

            if (profiles[pos].subtransactionpointcount)
            {
              outstream << profiles[pos].subtransactionpoints[profiles[pos].
                        subtransactionpointcount-1].tag;
            }

            outstream << "\t";

            for (int m=0; m < profiles[pos].subtransactionpointcount-1; m++)
            {
              outstream <<
                        profiles[pos].subtransactionpoints[m].tv.tv_sec*1000000 +
                        profiles[pos].subtransactionpoints[m].tv.tv_usec << ",";
            }

            if (profiles[pos].subtransactionpointcount)
            {
              outstream << profiles[pos].subtransactionpoints[profiles[pos].
                        subtransactionpointcount-1].tv.tv_sec*1000000 +
                        profiles[pos].subtransactionpoints[profiles[pos].
                            subtransactionpointcount-1].tv.tv_usec;
            }

            outstream << std::endl;
            fputs(outstream.str().c_str(), fp);
          }

          fclose(fp);
          mboxes.topologyMgrProducer->send(msgsnd, true);

          while (1)
          {
            sleep(10);
          }
        }
        break;
#endif

        case TOPIC_TOPOLOGY:
          mboxes.update(myTopology);
          getMyPartitionid();
          break;

        case TOPIC_APPLY:
        {
          apply();
        }
        break;

        default:
          printf("%s %i Engine bad topic %i\n", __FILE__, __LINE__,
                 msgrcv->topic);
      }
    }
  }
}

Engine::~Engine()
{
}

// launcher, regular function
void *engine(void *identity)
{
  Engine((Topology::partitionAddress *)identity);
  return NULL;
}

/* builtins for schema, no need for args, since abort is a function of status,
 * either figure it out in the function itself or outside in the main loop */
void Engine::createschema(void)
{
  createSchema(this);
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  TransactionAgent::usmReply(this, msgrcv->sourceAddr, *msg);
}

void Engine::createtable(void)
{
  // should check if map is ok
  class MessageUserSchema &msgrcvRef = *(class MessageUserSchema *)msgrcv;
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  status =
    domainidsToSchemata[msgrcvRef.domainid]->createTable(msgrcvRef.tableid);
  msg->tableid = msgrcvRef.tableid;
  //  replyTa(this, TOPIC_SCHEMAREPLY, msg);
  TransactionAgent::usmReply(this, msgrcv->sourceAddr, *msg);
}

void Engine::addcolumn(void)
{
  class MessageUserSchema &msgrcvRef = *(class MessageUserSchema *)msgrcv;
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  // either succeeds or fails :-)
  class Schema &schemaRef = *domainidsToSchemata[domainid];
  class Table &tableRef = *schemaRef.tables[msgrcvRef.tableid];
  msg->fieldid = tableRef.addfield((fieldtype_e) msgrcvRef.fieldtype,
                                   msgrcvRef.fieldlen, "", (indextype_e) msgrcvRef.indextype);
  status = BUILTIN_STATUS_OK;
  //  replyTa(this, TOPIC_SCHEMAREPLY, msg);
  TransactionAgent::usmReply(this, msgrcv->sourceAddr, *msg);
}

void Engine::deleteindex(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  status = BUILTIN_STATUS_OK;
  //  replyTa(this, TOPIC_SCHEMAREPLY, msg);
  TransactionAgent::usmReply(this, msgrcv->sourceAddr, *msg);
}

void Engine::deletetable(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  status = BUILTIN_STATUS_OK;
  //  replyTa(this, TOPIC_SCHEMAREPLY, msg);
  TransactionAgent::usmReply(this, msgrcv->sourceAddr, *msg);
}

void Engine::deleteschema(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  status = BUILTIN_STATUS_OK;
  //  replyTa(this, TOPIC_SCHEMAREPLY, msg);
  TransactionAgent::usmReply(this, msgrcv->sourceAddr, *msg);
}

int64_t Engine::getnextsubtransactionid(void)
{
  return ++nextsubtransactionid;
}

void Engine::getMyPartitionid()
{
  for (size_t n=0; n < myTopology.partitionListThisReplica.size(); n++)
  {
    if (myIdentity.mbox==myTopology.partitionListThisReplica[n].mbox)
    {
      partitionid = n;
    }
  }
}

// records
bool Engine::applyItem(int64_t subtransactionid, class Schema &schemaRef,
                       MessageDispatch::record_s &record)
{
  class Table &tableRef = *schemaRef.tables[record.tableid];
  //  class Table &tableRef = *domainidsToSchemata[domainid]->tables[record.tableid];

  switch (record.primitive)
  {
    case INSERT:
    {
      if (tableRef.rows.count(record.rowid))
      {
        printf("%s %i anomaly should not be an existing rowid %li\n", __FILE__,
               __LINE__, record.rowid);
        return false;
      }

      rowdata_s *row = new rowdata_s();
      //      row->writelockHolder = 0;
      row->previoussubtransactionid = subtransactionid;
      //      row->flags = 0;
      //      row->readlockHolders = NULL;
      row->row = record.row;
      tableRef.rows[record.rowid] = row;
    }
    break;

    case UPDATE:
    {
      if (tableRef.rows.count(record.rowid))
      {
        printf("%s %i anomaly should be an existing rowid %li\n", __FILE__,
               __LINE__, record.rowid);
        return false;
      }

      if (tableRef.rows[record.rowid]->previoussubtransactionid !=
          subtransactionid)
      {
        return false;
      }

      rowdata_s *row = new rowdata_s();
      //      row.writelockHolder = 0;
      row->previoussubtransactionid = subtransactionid;
      //      row.flags = 0;
      //      row->readlockHolders = NULL;
      row->row = record.row;
      tableRef.rows[record.rowid] = row;
    }
    break;

    case DELETE:
    {
      if (!tableRef.rows.count(record.rowid))
      {
        return false;
      }

      if (tableRef.rows[record.rowid]->previoussubtransactionid !=
          subtransactionid)
      {
        return false;
      }

      delete tableRef.rows[record.rowid];
      tableRef.rows.erase(record.rowid);
    }
    break;

    default:
      printf("%s %i anomaly primitive %i\n", __FILE__, __LINE__,
             record.primitive);
      return false;
  }

  return true;
}

// indices
bool Engine::applyItem(int64_t subtransactionid, class Schema &schemaRef,
                       MessageApply::applyindex_s &indexinfo)
{
  class Table &tableRef = *schemaRef.tables[indexinfo.tableid];
  class Index &indexRef = tableRef.fields[indexinfo.fieldid].index;

  if (indexRef.indextype==UNIQUE || indexRef.indextype==UNORDERED ||
      indexRef.indextype==UNIQUENOTNULL || indexRef.indextype==UNORDEREDNOTNULL)
  {
    // is unique
    if (indexinfo.fieldVal.isnull==true)
    {
      // is unique is null
      if (MessageApply::getisaddflag(indexinfo.flags)==true)
      {
        // is unique is null add
        vector<int64_t> v(2);
        v[0] = indexinfo.entry.rowid;
        v[1] = indexinfo.entry.engineid;
        indexRef.nulls.insert(v);
      }
      else
      {
        // is unique is null delete
        vector<int64_t> v(2);
        v[0] = indexinfo.entry.rowid;
        v[1] = indexinfo.entry.engineid;

        if (!indexRef.nulls.count(v))
        {
          return false;
        }

        indexRef.nulls.erase(v);
      }
    }
    else
    {
      // is unique not null
      if (MessageApply::getisaddflag(indexinfo.flags)==true)
      {
        // is unique not null add
        if (indexRef.addifnotthere(indexinfo.fieldVal, indexinfo.entry.rowid,
                                   indexinfo.entry.engineid, subtransactionid)==false)
        {
          if (indexRef.getprevioussubtransactionid(indexinfo.fieldVal) >
              subtransactionid)
          {
            return false;
          }
        }
      }
      else
      {
        // is unique not null delete
        if (indexRef.checkifthere(indexinfo.fieldVal)==true)
        {
          if (indexRef.checkifmatch(indexinfo.fieldVal, indexinfo.entry.rowid,
                                    indexinfo.entry.engineid)==true)
          {
            indexRef.rm(indexinfo.fieldVal);
          }
          else
          {
            if (indexRef.getprevioussubtransactionid(indexinfo.fieldVal) <
                subtransactionid)
            {
              return false;
            }
          }
        }
        else
        {
          return false;
        }
      }
    }
  }
  else
  {
    // not unique
    if (indexinfo.fieldVal.isnull==true)
    {
      // not unique is null
      if (MessageApply::getisaddflag(indexinfo.flags)==true)
      {
        // not unique is null add
        vector<int64_t> v(2);
        v[0] = indexinfo.entry.rowid;
        v[1] = indexinfo.entry.engineid;
        indexRef.nulls.insert(v);
      }
      else
      {
        // not unique is null delete
        vector<int64_t> v(2);
        v[0] = indexinfo.entry.rowid;
        v[1] = indexinfo.entry.engineid;

        if (!indexRef.nulls.count(v))
        {
          return false;
        }

        indexRef.nulls.erase(v);
      }
    }
    else
    {
      // not unique not null
      if (MessageApply::getisaddflag(indexinfo.flags)==true)
      {
        // not unique not null add
        switch (indexRef.indexmaptype)
        {
          case nonuniqueint:
          {
            indexRef.nonuniqueIntIndex->insert(pair<int64_t,
                                               nonLockingIndexEntry_s>(indexinfo.fieldVal.value.integer,
                                                   indexinfo.entry));
          }
          break;

          case nonuniqueuint:
          {
            indexRef.nonuniqueUintIndex->insert(pair<uint64_t,
                                                nonLockingIndexEntry_s>(indexinfo.fieldVal.value.uinteger,
                                                    indexinfo.entry));
          }
          break;

          case nonuniquebool:
          {
            indexRef.nonuniqueBoolIndex->insert(pair<bool,
                                                nonLockingIndexEntry_s>(indexinfo.fieldVal.value.boolean,
                                                    indexinfo.entry));
          }
          break;

          case nonuniquefloat:
          {
            indexRef.nonuniqueFloatIndex->insert(pair<long double,
                                                 nonLockingIndexEntry_s>(indexinfo.fieldVal.value.floating,
                                                     indexinfo.entry));
          }
          break;

          case nonuniquechar:
          {
            indexRef.nonuniqueCharIndex->insert(pair<char,
                                                nonLockingIndexEntry_s>(indexinfo.fieldVal.value.character,
                                                    indexinfo.entry));
          }
          break;

          case nonuniquecharx:
          {
            indexRef.nonuniqueStringIndex->insert(pair<string,
                                                  nonLockingIndexEntry_s>(indexinfo.fieldVal.str,
                                                      indexinfo.entry));
          }
          break;

          case nonuniquevarchar:
          {
            indexRef.nonuniqueStringIndex->insert(pair<string,
                                                  nonLockingIndexEntry_s>(indexinfo.fieldVal.str,
                                                      indexinfo.entry));
          }
          break;

          default:
            printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
                   indexRef.indexmaptype);
        }
      }
      else
      {
        // not unique not null delete
        switch (indexRef.indexmaptype)
        {
          case nonuniqueint:
          {
            pair<multimap<int64_t, nonLockingIndexEntry_s>::iterator,
                 multimap<int64_t, nonLockingIndexEntry_s>::iterator> iteratorRange;
            nonuniqueIntMap::iterator it;

            iteratorRange = indexRef.nonuniqueIntIndex->equal_range(indexinfo.fieldVal.value.integer);

            for (it=iteratorRange.first; it != iteratorRange.second; it++)
            {
              if (it->second.rowid==indexinfo.entry.rowid  &&
                  it->second.engineid==indexinfo.entry.engineid)
              {
                indexRef.nonuniqueIntIndex->erase(it);
                return true;
              }
            }

            return false; // found no entry, so it's not there, so background
          }
          break;

          case nonuniqueuint:
          {
            pair<multimap<uint64_t, nonLockingIndexEntry_s>::iterator,
                 multimap<uint64_t, nonLockingIndexEntry_s>::iterator> iteratorRange;
            nonuniqueUintMap::iterator it;

            iteratorRange = indexRef.nonuniqueUintIndex->equal_range(indexinfo.fieldVal.value.uinteger);

            for (it=iteratorRange.first; it != iteratorRange.second; it++)
            {
              if (it->second.rowid==indexinfo.entry.rowid  &&
                  it->second.engineid==indexinfo.entry.engineid)
              {
                indexRef.nonuniqueUintIndex->erase(it);
                return true;
              }
            }

            return false; // found no entry, so it's not there, so background
          }
          break;

          case nonuniquebool:
          {
            pair<multimap<bool, nonLockingIndexEntry_s>::iterator,
                 multimap<bool, nonLockingIndexEntry_s>::iterator> iteratorRange;
            nonuniqueBoolMap::iterator it;

            iteratorRange = indexRef.nonuniqueBoolIndex->equal_range(indexinfo.fieldVal.value.boolean);

            for (it=iteratorRange.first; it != iteratorRange.second; it++)
            {
              if (it->second.rowid==indexinfo.entry.rowid  &&
                  it->second.engineid==indexinfo.entry.engineid)
              {
                indexRef.nonuniqueBoolIndex->erase(it);
                return true;
              }
            }

            return false; // found no entry, so it's not there, so background
          }
          break;

          case nonuniquefloat:
          {
            pair<multimap<long double, nonLockingIndexEntry_s>::iterator,
                 multimap<long double, nonLockingIndexEntry_s>::iterator> iteratorRange;
            nonuniqueFloatMap::iterator it;

            iteratorRange = indexRef.nonuniqueFloatIndex->equal_range(indexinfo.fieldVal.value.floating);

            for (it=iteratorRange.first; it != iteratorRange.second; it++)
            {
              if (it->second.rowid==indexinfo.entry.rowid  &&
                  it->second.engineid==indexinfo.entry.engineid)
              {
                indexRef.nonuniqueFloatIndex->erase(it);
                return true;
              }
            }

            return false; // found no entry, so it's not there, so background
          }
          break;

          case nonuniquechar:
          {
            pair<multimap<char, nonLockingIndexEntry_s>::iterator,
                 multimap<char, nonLockingIndexEntry_s>::iterator> iteratorRange;
            nonuniqueCharMap::iterator it;

            iteratorRange = indexRef.nonuniqueCharIndex->equal_range(indexinfo.fieldVal.value.character);

            for (it=iteratorRange.first; it != iteratorRange.second; it++)
            {
              if (it->second.rowid==indexinfo.entry.rowid  &&
                  it->second.engineid==indexinfo.entry.engineid)
              {
                indexRef.nonuniqueCharIndex->erase(it);
                return true;
              }
            }

            return false; // found no entry, so it's not there, so background
          }
          break;

          case nonuniquecharx:
          {
            pair<multimap<string, nonLockingIndexEntry_s>::iterator,
                 multimap<string, nonLockingIndexEntry_s>::iterator> iteratorRange;
            nonuniqueStringMap::iterator it;

            iteratorRange = indexRef.nonuniqueStringIndex->equal_range(indexinfo.fieldVal.str);

            for (it=iteratorRange.first; it != iteratorRange.second; it++)
            {
              if (it->second.rowid==indexinfo.entry.rowid  &&
                  it->second.engineid==indexinfo.entry.engineid)
              {
                indexRef.nonuniqueStringIndex->erase(it);
                return true;
              }
            }

            return false; // found no entry, so it's not there, so background
          }
          break;

          case nonuniquevarchar:
          {
            pair<multimap<string, nonLockingIndexEntry_s>::iterator,
                 multimap<string, nonLockingIndexEntry_s>::iterator> iteratorRange;
            nonuniqueStringMap::iterator it;

            iteratorRange = indexRef.nonuniqueStringIndex->equal_range(indexinfo.fieldVal.str);

            for (it=iteratorRange.first; it != iteratorRange.second; it++)
            {
              if (it->second.rowid==indexinfo.entry.rowid  &&
                  it->second.engineid==indexinfo.entry.engineid)
              {
                indexRef.nonuniqueStringIndex->erase(it);
                return true;
              }
            }

            return false; // found no entry, so it's not there, so background
          }
          break;

          default:
            printf("%s %i anomaly indexmaptype %i\n", __FILE__, __LINE__,
                   indexRef.indexmaptype);
        }
      }
    }
  }

  return true;
}

void Engine::apply()
{
  class MessageApply &inmsg = *(class MessageApply *)msgrcv;
  class Schema &schemaRef = *domainidsToSchemata[inmsg.domainid];

  for (size_t n=0; n < inmsg.rows.size(); n++)
  {
    if (applyItem(inmsg.subtransactionid, schemaRef, inmsg.rows[n])==false)
    {
      background(inmsg, inmsg.rows[n]);
    }
  }

  for (size_t n=0; n < inmsg.indices.size(); n++)
  {
    if (applyItem(inmsg.subtransactionid, schemaRef, inmsg.indices[n])==false)
    {
      background(inmsg, inmsg.indices[n]);
    }
  }

  if (!backgrounded.count(inmsg.subtransactionid))
  {
    class MessageAckApply *ackmsg =
        new class MessageAckApply(inmsg.subtransactionid, inmsg.applierid,
                                      -1, STATUS_OK);
    mboxes.toActor(myIdentity.address, inmsg.sourceAddr, *ackmsg);
  }

  // now, walk through backgrounded items
  map<int64_t, background_s>::iterator itb;

  for (itb = backgrounded.begin(); itb != backgrounded.end(); itb++)
{
    // itb->first: subtransactionid
    background_s &bref = itb->second;
    vector<MessageDispatch::record_s>::iterator itr;

    for (itr = bref.rows.begin(); itr != bref.rows.end(); itr++)
    {
      if (applyItem(itb->first, schemaRef, *itr)==true)
      {
        bref.rows.erase(itr);
      }
    }

    vector<MessageApply::applyindex_s>::iterator iti;

    for (iti = bref.indices.begin(); iti != bref.indices.end(); iti++)
    {
      if (applyItem(itb->first, schemaRef, *iti)==true)
      {
        bref.indices.erase(iti);
      }
    }

    if (!bref.rows.size() && !bref.indices.size())
    {
      class MessageAckApply *ackmsg =
            new class MessageAckApply(itb->first, bref.applierid, -1, STATUS_OK);
      mboxes.toActor(myIdentity.address, bref.taAddress, *ackmsg);
      backgrounded.erase(itb);
    }
  }
}

void Engine::background(class MessageApply &inmsg,
                        MessageDispatch::record_s &item)
{
  if (!backgrounded.count(inmsg.subtransactionid))
  {
    background_s b;
    b.applierid = inmsg.applierid;
    b.taAddress = inmsg.sourceAddr;
    backgrounded[inmsg.subtransactionid]=b;
  }

  backgrounded[inmsg.subtransactionid].rows.push_back(item);
}

void Engine::background(class MessageApply &inmsg,
                        MessageApply::applyindex_s &item)
{
  if (!backgrounded.count(inmsg.subtransactionid))
  {
    background_s b;
    b.applierid = inmsg.applierid;
    b.taAddress = inmsg.sourceAddr;
    backgrounded[inmsg.subtransactionid]=b;
  }

  backgrounded[inmsg.subtransactionid].indices.push_back(item);
}

