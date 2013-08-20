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

#ifndef DEADLOCKMGR_HPP
#define DEADLOCKMGR_HPP

#include "infinisql_gch.h"

typedef struct
{
  //  int64_t taid;
  Topology::addressStruct addr;
  int64_t pendingcmdid;
} taCmd;

class DeadlockMgr
{
public:
  DeadlockMgr(Topology::partitionAddress *);
  virtual ~DeadlockMgr();

  friend class Transaction; // uses makeLockedItem

private:
  static void makeLockedItem(bool, int64_t, int64_t, int64_t, int64_t, int64_t,
                             long double, string *, string *);
  void algorithm(void);
  void deadlock(int64_t);
  bool walk(int64_t);
  bool walk(boost::unordered_set<string>::iterator);
  void removeTransaction(int64_t);
  class Mbox *mymboxPtr;

  Topology::partitionAddress myIdentity;
  class Mboxes mboxes;
  class Topology myTopology;

  // maps:
  // transactions with the locks it holds
  boost::unordered_map< int64_t, boost::unordered_set<string> >
  transactionLocksMap;
  // transactions with what it waits on
  boost::unordered_map< int64_t, boost::unordered_set<string> >
  transactionWaitsMap;
  // items whose locks are held by transactions
  boost::unordered_map< string, boost::unordered_set<int64_t> >
  locksTransactionMap;
  // items whose locks are waited for by transactions
  boost::unordered_map< string, boost::unordered_set<int64_t> >
  waitsTransactionMap;

  boost::unordered_map<int64_t, taCmd> returnMap;

  boost::unordered_set<int64_t> skipTransactionSet;
  boost::unordered_set<string> skipItemSet;

  // duplicates here mean deadlock:
  boost::unordered_set<int64_t> transactionGraphSet;

};

void *deadlockMgr(void *);

#endif  /* DEADLOCKMGR_HPP */
