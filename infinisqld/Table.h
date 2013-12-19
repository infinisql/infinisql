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
 * @file   Table.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:52:59 2013
 * 
 * @brief  Table class. Has Fields and Indices.
 */

#ifndef INFINISQLTABLE_H
#define INFINISQLTABLE_H

#include "gch.h"
#include "Field.h"

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
    Table(int64_t idarg);
    virtual ~Table()
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
    void setname(string namearg);
    std::string *getname();
    int64_t addfield(fieldtype_e type, int64_t length, std::string name,
                     indextype_e indextype);
    bool makerow(vector<fieldValue_s> *fieldVal, std::string *res);
    bool unmakerow(std::string *rowstring,
                   vector<fieldValue_s> *resultFields);
    // for fetch (cursor)
    void getrows(vector<int64_t> rowids, locktype_e locktype,
                 int64_t subtransactionid, int64_t pendingcmdid,
                 vector<returnRow_s> *returnRows,
                 vector<int64_t> *lockPendingRowids, int64_t tacmdentrypoint);
    // stage. return rowid. take command (insert|update|delete),
    // subtransactionid,
    // row, rowid
    //  int64_t stage(pendingprimitive_e, int64_t, string *, int64_t);
    // commit, i guess just the rowid is enough to find it
    //  void commit(int64_t, int64_t);
    // rollback
    //  void rollback(int64_t, int64_t);
    int64_t getnextrowid();
    void newrow(int64_t newrowid, int64_t subtransactionid, string &row);
    int64_t updaterow(int64_t rowid, int64_t subtransactionid, string *row);
    int64_t deleterow(int64_t rowid, int64_t subtransactionid);
    int64_t deleterow(int64_t rowid, int64_t subtransactionid,
                      int64_t forward_rowid, int64_t forward_engineid);
    // for select
    void selectrows(vector<int64_t> *rowids, locktype_e locktype,
                    int64_t subtransactionid, int64_t pendingcmdid,
                    vector<returnRow_s> *returnRows, int64_t tacmdentrypoint);
    locktype_e assignToLockQueue(int64_t rowid, locktype_e locktype,
                                 int64_t subtransactionid,
                                 int64_t pendingcmdid,
                                 int64_t tacmdentrypoint);
    void commitRollbackUnlock(int64_t rowid, int64_t subtransactionid,
                              enginecmd_e cmd);

    //private:
    int64_t id;
    std::string name;
    int64_t nextindexid;
    std::vector<class Field> fields;
    boost::unordered_map< int64_t, std::queue<lockQueueRowEntry> > lockQueue;
    class Table *shadowTable;
    boost::unordered_map<std::string, int64_t> columnaNameToFieldMap;
    //  std::unordered_map<int64_t, rowdata_s *> rows; // this is the actual data
    boost::unordered_map<int64_t, rowdata_s *> rows; // this is the actual data
    //  boost::unordered_map<int64_t, rowdata_s> rows; // this is the actual data
    int64_t rowsize;
    int64_t nextrowid; // do not mess with this directly
    // this is for the delete component of a replacement
    boost::unordered_map<int64_t, forwarderEntry> forwarderMap;
};

#endif  /* INFINISQLTABLE_H */
