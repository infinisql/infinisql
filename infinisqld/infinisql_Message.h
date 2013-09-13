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

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "infinisql_gch.h"
#include "infinisql_defs.h"
#include "infinisql_Topology.h"

class Message
{
public:
  Message();
  virtual ~Message();

  void setEnvelope(const Topology::addressStruct &,
                   const Topology::addressStruct &, class Message &);
  void serialize(msgpack::packer<msgpack::sbuffer> &);
  bool deserialize(msgpack::unpacker &);
  static msgpack::object *getitem(msgpack::unpacker &, msgpack::object &);
  static void serFieldValue(fieldValue_s &, msgpack::packer<msgpack::sbuffer> &);
  static void deserFieldValue(msgpack::unpacker &, fieldValue_s &);
  static void serFieldValue(fieldValue_s &, string &);
  static void deserFieldValue(string &, fieldValue_s &);
  void ser(msgpack::packer<msgpack::sbuffer> &);
  static class Message *deser(msgpack::unpacker &);
  void print(int64_t);
  void output(int64_t);

  payloadtype_e payloadtype;
  topic_e topic;

  Topology::addressStruct sourceAddr;
  Topology::addressStruct destAddr;

  class Message *next;
  __int128 nextmsg;
};

class MessageSocket : public Message
{
public:
  MessageSocket();
  MessageSocket(int, uint32_t, listenertype_e);
  virtual ~MessageSocket();

  void serialize(msgpack::packer<msgpack::sbuffer> &);
  void deserialize(msgpack::unpacker &, class Message &);

  int socket;
  uint32_t events;
  listenertype_e listenertype;
};

class MessageUserSchema : public Message
{
public:
  MessageUserSchema();
  MessageUserSchema(topic_e);
  virtual ~MessageUserSchema();

  void serialize(msgpack::packer<msgpack::sbuffer> &);
  void deserialize(msgpack::unpacker &, class Message &);

  int operationtype;
  int caller;
  int callerstate;

  string username;
  string domainname;
  string password;

  int64_t argsize;
  int64_t instance;
  int64_t operationid;
  int64_t domainid;
  int64_t userid;
  int64_t tableid;
  int64_t fieldlen;
  int64_t builtincmd;
  int64_t indexid;
  int64_t tableindexid;
  int64_t simple;
  int64_t fieldid;
  int64_t numfields;
  int64_t intdata;
  int64_t status;
  indextype_e indextype;
  fieldtype_e fieldtype;
  string argstring;
  procedures_s procs;
  string pathname;
  string procname;
};

class MessageDeadlock : public Message
{
public:
  MessageDeadlock();
  virtual ~MessageDeadlock();

  void serialize(msgpack::packer<msgpack::sbuffer> &);
  void deserialize(msgpack::unpacker &, class Message &);

  int64_t transactionid;
  int64_t tainstance;
  int64_t transaction_pendingcmdid;
  int64_t deadlockchange;
  string deadlockNode;
  newDeadLockLists_s nodes;
};

class MessageTransaction : public Message
{
public:
  MessageTransaction();
  virtual ~MessageTransaction();

  void serialize(msgpack::packer<msgpack::sbuffer> &);
  void deserialize(msgpack::unpacker &, class Message &);
  void print(int64_t);

  int64_t transactionid;
  int64_t subtransactionid;
  int64_t previoussubtransactionid;
  int64_t tainstance;
  int64_t domainid;
  int64_t transaction_enginecmd;
  int64_t transaction_pendingcmdid;
  int64_t transaction_tacmdentrypoint;
  int64_t engineinstance;
};

class MessageSubtransactionCmd : public MessageTransaction
{
public:
  MessageSubtransactionCmd();
  virtual ~MessageSubtransactionCmd();

  void serialize(msgpack::packer<msgpack::sbuffer> &);
  void deserialize(msgpack::unpacker &, class Message &);
  static void serIndexHits(vector<nonLockingIndexEntry_s> &,
                           msgpack::packer<msgpack::sbuffer> &);
  static void deserIndexHits(msgpack::unpacker &,
                             vector<nonLockingIndexEntry_s> &);
  static void serSearchParams(searchParams_s &,
                              msgpack::packer<msgpack::sbuffer> &);
  static void deserSearchParams(msgpack::unpacker &, searchParams_s &);
  static void serReturnRows(vector<returnRow_s> &,
                            msgpack::packer<msgpack::sbuffer> &);
  static void deserReturnRows(msgpack::unpacker &, vector<returnRow_s> &);
  void print(int64_t);

  subtransactionCmd_s cmd;
};

class MessageCommitRollback : public MessageTransaction
{
public:
  MessageCommitRollback();
  virtual ~MessageCommitRollback();

  void serialize(msgpack::packer<msgpack::sbuffer> &);
  void deserialize(msgpack::unpacker &, class Message &);
  static void serRofs(vector<rowOrField_s> &,
                      msgpack::packer<msgpack::sbuffer> &);
  static void deserRofs(msgpack::unpacker &, vector<rowOrField_s> &);

  vector<rowOrField_s> rofs;
};

class MessageDispatch : public Message
{
public:
  struct record_s
  {
    int64_t rowid;
    pendingprimitive_e primitive;
    int64_t tableid;
    int64_t previoussubtransactionid;
    string row;
    string oldrow;
  };

  MessageDispatch();
  virtual ~MessageDispatch();

  void serialize(msgpack::packer<msgpack::sbuffer> &);
  void deserialize(msgpack::unpacker &, class Message &);

  int64_t transactionid;
  int64_t domainid;
  // pidsids[partitionid] = subtransactionid
  map<int64_t, int64_t> pidsids;
  // records[partitionid][#] = {recordinfo}
  map< int64_t, vector<record_s> > records;
};

class MessageAckDispatch : public Message
{
public:
  MessageAckDispatch();
  MessageAckDispatch(int64_t, int);
  virtual ~MessageAckDispatch();

  void serialize(msgpack::packer<msgpack::sbuffer> &);
  void deserialize(msgpack::unpacker &, class Message &);

  int64_t transactionid;
  int status;
};

#define ADDFLAG 0

class MessageApply : public Message
{
public:
  struct applyindex_s
  {
    fieldValue_s fieldVal;
    nonLockingIndexEntry_s entry;
    char flags;
    int64_t tableid;
    int64_t fieldid;
  };

  MessageApply();
  MessageApply(int64_t, int64_t, int64_t);
  virtual ~MessageApply();
  static void setisaddflag(char *);
  static char getisaddflag(char);
  static void cleariaddflag(char *);

  void serialize(msgpack::packer<msgpack::sbuffer> &);
  void deserialize(msgpack::unpacker &, class Message &);

  int64_t subtransactionid;
  int64_t applierid;
  int64_t domainid;
  vector<MessageDispatch::record_s> rows;
  vector<applyindex_s> indices;
};

class MessageAckApply : public Message
{
public:
  MessageAckApply();
  MessageAckApply(int64_t, int64_t, int64_t, int);
  virtual ~MessageAckApply();

  void serialize(msgpack::packer<msgpack::sbuffer> &);
  void deserialize(msgpack::unpacker &, class Message &);

  int64_t subtransactionid;
  int64_t applierid;
  int64_t partitionid;
  int status;
};

#endif  /* MESSAGE_HPP */
