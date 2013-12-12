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

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "infinisql_gch.h"
#include "infinisql_Table.h"
#include "infinisql_TransactionAgent.h"
#include "infinisql_SubTransaction.h"

class Engine
{
public:
  struct background_s
  {
    int64_t applierid;
    Topology::addressStruct taAddress;

    vector<MessageDispatch::record_s> rows;
    vector<MessageApply::applyindex_s> indices;
  };

  Engine(Topology::partitionAddress *);
  virtual ~Engine();

  bool applyItem(int64_t, class Schema &, MessageDispatch::record_s &);
  bool applyItem(int64_t, class Schema &, MessageApply::applyindex_s &);

  friend class SubTransaction;

  // public for replyTa:
  class Message *msgsnd;
  int64_t operationid;
  int64_t domainid;
  int64_t userid;
  int64_t status;
  Topology::addressStruct taAddr;
  //public for createSchema:
  class Message *msgrcv;
  REUSEMESSAGES
  domainidToSchemaMap domainidsToSchemata;
  class Mboxes mboxes;
  Topology::partitionAddress myIdentity;
  int64_t partitionid;

private:
  int64_t getnextsubtransactionid();
  void createschema();
  void createtable();
  void addcolumn();
  void deleteindex();
  void deletetable();
  void deleteschema();
  void getMyPartitionid();
  void apply();
  void background(class MessageApply &, MessageDispatch::record_s &);
  void background(class MessageApply &, MessageApply::applyindex_s &);

  class Topology myTopology;

  class Mbox *mymboxPtr;
  int64_t argsize;
  int64_t nextsubtransactionid;
  int64_t instance;
  boost::unordered_map<int64_t, class SubTransaction *> SubTransactions;
  map<int64_t, background_s> backgrounded;

#ifdef PROFILE
  // profiling:
  int rid;
  PROFILERENGINE inboundProfile[2];
  PROFILERENGINECOMPREHENSIVE *profiles;
  int profilecount;
#endif
};

void *engine(void *);

#endif  /* ENGINE_HPP */
