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

#ifndef SUBTRANSACTION_HPP
#define SUBTRANSACTION_HPP

#include "infinisql_gch.h"
#include "infinisql_Engine.h"

// both SubTransaction and Transaction use this, for different purposes, so
// not all fields have meaning for both classes.

class SubTransaction
{
public:
  //  SubTransaction(int64_t, int64_t, int64_t, class Engine *);
  SubTransaction(Topology::addressStruct &, int64_t, int64_t, class Engine *);
  virtual ~SubTransaction();

  friend class Engine;

private:
  void processTransactionMessage(class Message *);
  void commitRollbackUnlock(vector<rowOrField_s> *, enginecmd_e);
  void processRowLockQueue(int64_t, int64_t); // keep
  void drainRowLockQueue(int64_t, int64_t); // keep
  void processIndexLockQueue(int64_t, int64_t, fieldValue_s *);
  void drainIndexLockQueue(int64_t, int64_t, fieldValue_s *);
  int64_t newrow(int64_t tableid, string row);
  locktype_e uniqueIndex(int64_t, int64_t, int64_t, int64_t, fieldValue_s *);
  int64_t updaterow(int64_t, int64_t, string *);
  int64_t deleterow(int64_t, int64_t);
  int64_t deleterow(int64_t, int64_t, int64_t, int64_t);
  void indexSearch(int64_t, int64_t, searchParams_s *,
                   vector<nonLockingIndexEntry_s> *);
  void selectrows(int64_t, vector<int64_t> *, locktype_e, int64_t,
                  vector<returnRow_s> *);
  void searchReturn1(int64_t, int64_t, locktype_e, searchParams_s &,
                     vector<returnRow_s> &);
  void replyTransaction(void *);
  void replyTransaction(class MessageTransaction &, class MessageTransaction &);

  //private:
  int64_t subtransactionid;
  Topology::addressStruct taAddr;
  int64_t transactionid;
  int64_t domainid;
  class Message *msgrcv;
  class Engine *enginePtr;
  class Schema *schemaPtr;
  //  vector<locked_s> lockedItems;

#ifdef PROFILE
  int rid;
  PROFILERENGINE *subtransactionpoints;
  int subtransactionpointcount;

  void profileEntry(int tag)
  {
    gettimeofday(&subtransactionpoints[subtransactionpointcount].tv, NULL);
    subtransactionpoints[subtransactionpointcount++].tag = tag;
  }
#endif

};

#endif  /* SUBTRANSACTION_HPP */
