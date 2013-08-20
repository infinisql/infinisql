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

#ifndef TABLE_HPP
#define TABLE_HPP

#include "infinisql_gch.h"
#include "infinisql_Field.h"

typedef struct
{
  int64_t rowid;
  int64_t engineid;
} forwarderEntry;

// this per-row to say which transactionid created/updated transaction last,
// for replication so that the corresponding engine(s) will know the order
// to apply changes. also says whether the row is locked by somebody
typedef struct
{
  int64_t writelockHolder; // this is also the subtransactionid
  int64_t previoussubtransactionid;
  char flags; // commit & rollback set this to 0
  boost::unordered_set<int64_t> *readlockHolders;
  string row;
} rowdata_s;

typedef struct
{
  int64_t pendingcmdid;
  int64_t tacmdentrypoint;
  int64_t subtransactionid;
  locktype_e locktype;
} lockQueueRowEntry;

class Table
{
public:
  Table(int64_t);
  virtual ~Table(void)
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
  void setname(string);
  string *getname(void);
  int64_t addfield(fieldtype_e, int64_t, string, indextype_e);
  bool makerow(vector<fieldValue_s> *, string *);
  bool unmakerow(string *, vector<fieldValue_s> *);
  // for fetch (cursor)
  void getrows(vector<int64_t>, locktype_e, int64_t, int64_t,
               vector<returnRow_s> *, vector<int64_t> *, int64_t);
  // stage. return rowid. take command (insert|update|delete), subtransactionid,
  // row, rowid
  //  int64_t stage(pendingprimitive_e, int64_t, string *, int64_t);
  // commit, i guess just the rowid is enough to find it
  //  void commit(int64_t, int64_t);
  // rollback
  //  void rollback(int64_t, int64_t);
  int64_t getnextrowid(void);
  void newrow(int64_t, int64_t, string &);
  int64_t updaterow(int64_t, int64_t, string *);
  int64_t deleterow(int64_t, int64_t);
  int64_t deleterow(int64_t, int64_t, int64_t, int64_t);
  // for select
  void selectrows(vector<int64_t> *, locktype_e, int64_t, int64_t,
                  vector<returnRow_s> *, int64_t);
  locktype_e assignToLockQueue(int64_t, locktype_e, int64_t, int64_t, int64_t);
  void commitRollbackUnlock(int64_t, int64_t, enginecmd_e);

  //private:
  int64_t id;
  string name;
  int64_t nextindexid;
  vector<class Field> fields;
  boost::unordered_map< int64_t, std::queue<lockQueueRowEntry> > lockQueue;
  class Table *shadowTable;
  boost::unordered_map<string, int64_t> columnaNameToFieldMap;
  //  std::unordered_map<int64_t, rowdata_s *> rows; // this is the actual data
  boost::unordered_map<int64_t, rowdata_s *> rows; // this is the actual data
  //  boost::unordered_map<int64_t, rowdata_s> rows; // this is the actual data
  int64_t rowsize;
  int64_t nextrowid; // do not mess with this directly
  // this is for the delete component of a replacement
  boost::unordered_map<int64_t, forwarderEntry> forwarderMap;
};

#endif  /* TABLE_HPP */
