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

#include "infinisql_Message.h"
#line 28 "Message.cc"

Message::Message() : next(NULL)
{
}

Message::~Message()
{
}

void Message::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  pack.pack_int(payloadtype);
  pack.pack_int(topic);
  pack.pack_raw(sizeof(sourceAddr));
  pack.pack_raw_body((const char *)&sourceAddr, sizeof(sourceAddr));
  pack.pack_raw(sizeof(destAddr));
  pack.pack_raw_body((const char *)&destAddr, sizeof(destAddr));
  pack.pack_int64((int64_t)next);
}

bool Message::deserialize(msgpack::unpacker &unpack)
{
  msgpack::object obj;

  if (getitem(unpack, obj)==NULL)
  {
    return false;
  }

  obj.convert((int *)&payloadtype);
  getitem(unpack, obj)->convert((int *)&topic);
  string str(sizeof(sourceAddr), 0);
  getitem(unpack, obj)->convert(&str);
  memcpy(&sourceAddr, str.c_str(), sizeof(sourceAddr));
  getitem(unpack, obj)->convert(&str);
  memcpy(&destAddr, str.c_str(), sizeof(destAddr));
  getitem(unpack, obj)->convert((int64_t *)&next);

  return true;
}

void Message::setEnvelope(const Topology::addressStruct &source,
                          const Topology::addressStruct &dest, class Message &msg)
{
  msg.sourceAddr = source;
  msg.destAddr = dest;
}

msgpack::object *Message::getitem(msgpack::unpacker &unpack,
                                  msgpack::object &obj)
{
  msgpack::unpacked unpacked;

  if (unpack.next(&unpacked))
  {
    obj = unpacked.get();
    return &obj;
  }
  else
  {
    return NULL;
  }
}

void Message::serFieldValue(fieldValue_s &fieldVal,
                            msgpack::packer<msgpack::sbuffer> &pack)
{
  pack.pack_raw(sizeof(fieldVal.value));
  pack.pack_raw_body((const char *)&fieldVal.value, sizeof(fieldVal.value));

  if (fieldVal.isnull==true)
  {
    pack.pack_true();
  }
  else
  {
    pack.pack_false();
  }

  pack.pack(fieldVal.str);
}

void Message::deserFieldValue(msgpack::unpacker &unpack, fieldValue_s &fieldVal)
{
  msgpack::object obj;

  string str(sizeof(fieldVal.value), 0);
  getitem(unpack, obj)->convert(&str);
  memcpy(&fieldVal.value, str.c_str(), sizeof(fieldVal.value));
  getitem(unpack, obj)->convert((bool *)&fieldVal.isnull);
  getitem(unpack, obj)->convert(&fieldVal.str);
}

void Message::serFieldValue(fieldValue_s &fieldVal, string &str)
{
  msgpack::sbuffer sbuf;
  msgpack::packer<msgpack::sbuffer> pack(sbuf);

  pack.pack_raw(sizeof(fieldVal.value));
  pack.pack_raw_body((const char *)&fieldVal.value, sizeof(fieldVal.value));

  if (fieldVal.isnull==true)
  {
    pack.pack_true();
  }
  else
  {
    pack.pack_false();
  }

  pack.pack(fieldVal.str);

  str.assign((const char *)sbuf.data(), sbuf.size());
}

void Message::deserFieldValue(string &str, fieldValue_s &fieldVal)
{
  msgpack::sbuffer sbuf;
  sbuf.write(str.c_str(), str.size());
  msgpack::unpacker unpack;
  unpack.reserve_buffer(sbuf.size());
  memcpy(unpack.buffer(), sbuf.data(), sbuf.size());
  unpack.buffer_consumed(sbuf.size());

  msgpack::object obj;

  string valuestr(sizeof(fieldVal.value), 0);
  getitem(unpack, obj)->convert(&valuestr);
  memcpy(&fieldVal.value, valuestr.c_str(), sizeof(fieldVal.value));
  getitem(unpack, obj)->convert((bool *)&fieldVal.isnull);
  getitem(unpack, obj)->convert(&fieldVal.str);
}

void Message::ser(msgpack::packer<msgpack::sbuffer> &pack)
{
  switch (payloadtype)
  {
    case PAYLOADMESSAGE:
      ((class Message *)this)->serialize(pack);
      break;

    case PAYLOADSOCKET:
      ((class MessageSocket *)this)->serialize(pack);
      break;

    case PAYLOADUSERSCHEMA:
      ((class MessageUserSchema *)this)->serialize(pack);
      break;

    case PAYLOADDEADLOCK:
      ((class MessageDeadlock *)this)->serialize(pack);
      break;

    case PAYLOADSUBTRANSACTION:
      ((class MessageSubtransactionCmd *)this)->serialize(pack);
      break;

    case PAYLOADCOMMITROLLBACK:
      ((class MessageCommitRollback *)this)->serialize(pack);
      break;

    case PAYLOADDISPATCH:
      ((class MessageDispatch *)this)->serialize(pack);
      break;

    case PAYLOADACKDISPATCH:
      ((class MessageAckDispatch *)this)->serialize(pack);
      break;

    case PAYLOADAPPLY:
      ((class MessageApply *)this)->serialize(pack);
      break;

    case PAYLOADACKAPPLY:
      ((class MessageAckApply *)this)->serialize(pack);
      break;

    default:
      printf("%s %i anomaly %i\n", __FILE__, __LINE__, payloadtype);
  }
}

class Message *Message::deser(msgpack::unpacker &unpack)
{
  class Message tmpmsg;

  if (tmpmsg.deserialize(unpack)==false)
  {
    return NULL;
  }

  class Message *msg;

  switch (tmpmsg.payloadtype)
  {
    case PAYLOADMESSAGE:
      msg = (class Message *)new class Message;
      *msg = tmpmsg;
      break;

    case PAYLOADSOCKET:
      msg = (class Message *)new class MessageSocket;
      ((class MessageSocket *)msg)->deserialize(unpack, tmpmsg);
      break;

    case PAYLOADUSERSCHEMA:
      msg = (class Message *)new class MessageUserSchema;
      ((class MessageUserSchema *)msg)->deserialize(unpack, tmpmsg);
      break;

    case PAYLOADDEADLOCK:
      msg = (class Message *)new class MessageDeadlock;
      ((class MessageDeadlock *)msg)->deserialize(unpack, tmpmsg);
      break;

    case PAYLOADSUBTRANSACTION:
      msg = (class Message *)new class MessageSubtransactionCmd;
      ((class MessageSubtransactionCmd *)msg)->deserialize(unpack, tmpmsg);
      break;

    case PAYLOADCOMMITROLLBACK:
      msg = (class Message *)new class MessageCommitRollback;
      ((class MessageCommitRollback *)msg)->deserialize(unpack, tmpmsg);
      break;

    case PAYLOADDISPATCH:
      msg = (class Message *)new class MessageDispatch;
      ((class MessageDispatch *)msg)->deserialize(unpack, tmpmsg);
      break;

    case PAYLOADACKDISPATCH:
      msg = (class Message *)new class MessageAckDispatch;
      ((class MessageAckDispatch *)msg)->deserialize(unpack, tmpmsg);
      break;

    case PAYLOADAPPLY:
      msg = (class Message *)new class MessageApply;
      ((class MessageApply *)msg)->deserialize(unpack, tmpmsg);
      break;

    case PAYLOADACKAPPLY:
      msg = (class Message *)new class MessageAckApply;
      ((class MessageAckApply *)msg)->deserialize(unpack, tmpmsg);
      break;

    default:
      printf("%s %i anomaly %i\n", __FILE__, __LINE__, tmpmsg.payloadtype);
      return NULL;
  }

  return msg;
}

void Message::print(int64_t nodeid)
{
  printf("%s %i Message: %li %i %i %li %li %li %li %p\n", __FILE__, __LINE__,
         nodeid, payloadtype, topic, sourceAddr.nodeid, sourceAddr.actorid,
         destAddr.nodeid, destAddr.actorid, next);
}

void Message::output(int64_t nodeid)
{
  switch (payloadtype)
  {
    case PAYLOADMESSAGE:
      ((class Message *)this)->print(nodeid);
      break;

    case PAYLOADSOCKET:
      ((class MessageSocket *)this)->print(nodeid);
      break;

    case PAYLOADUSERSCHEMA:
      ((class MessageUserSchema *)this)->print(nodeid);
      break;

    case PAYLOADDEADLOCK:
      ((class MessageDeadlock *)this)->print(nodeid);
      break;

    case PAYLOADSUBTRANSACTION:
      ((class MessageSubtransactionCmd *)this)->print(nodeid);
      break;

    case PAYLOADCOMMITROLLBACK:
      ((class MessageCommitRollback *)this)->print(nodeid);
      break;

    default:
      printf("%s %i anomaly %i\n", __FILE__, __LINE__, payloadtype);
  }
}

MessageSocket::MessageSocket()
{
}

MessageSocket::MessageSocket(int socketarg, uint32_t eventsarg,
                             listenertype_e listenertypearg) : socket(socketarg), events(eventsarg),
  listenertype(listenertypearg)
{
  topic = TOPIC_SOCKET;
  payloadtype = PAYLOADSOCKET;
}

MessageSocket::~MessageSocket()
{
}

void MessageSocket::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  Message::serialize(pack);

  pack.pack_int(socket);
  pack.pack_uint32(events);
  pack.pack_int(listenertype);
}

void MessageSocket::deserialize(msgpack::unpacker &unpack, class Message &msg)
{
  payloadtype = msg.payloadtype;
  topic = msg.topic;
  sourceAddr = msg.sourceAddr;
  destAddr = msg.destAddr;
  next = msg.next;
  msgpack::object obj;

  getitem(unpack, obj)->convert(&socket);
  getitem(unpack, obj)->convert(&events);
  getitem(unpack, obj)->convert((int *)&listenertype);
}

MessageUserSchema::MessageUserSchema()
{
}

MessageUserSchema::MessageUserSchema(topic_e topicarg)
{
  topic = topicarg;
  payloadtype = PAYLOADUSERSCHEMA;
}

MessageUserSchema::~MessageUserSchema()
{
}

void MessageUserSchema::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  Message::serialize(pack);

  pack.pack_int(operationtype);
  pack.pack_int(caller);
  pack.pack_int(callerstate);

  pack.pack(username);
  pack.pack(domainname);
  pack.pack(password);

  pack.pack_int64(argsize);
  pack.pack_int64(instance);
  pack.pack_int64(operationid);
  pack.pack_int64(domainid);
  pack.pack_int64(userid);
  pack.pack_int64(tableid);
  pack.pack_int64(fieldlen);
  pack.pack_int64(builtincmd);
  pack.pack_int64(indexid);
  pack.pack_int64(tableindexid);
  pack.pack_int64(simple);
  pack.pack_int64(fieldid);
  pack.pack_int64(numfields);
  pack.pack_int64(intdata);
  pack.pack_int64(status);
  pack.pack_int(indextype);
  pack.pack_int(fieldtype);
  pack.pack(argstring);
  pack.pack_raw(sizeof(procs));
  pack.pack_raw_body((const char *)&procs, sizeof(procs));
  pack.pack(pathname);
  pack.pack(procname);
}

void MessageUserSchema::deserialize(msgpack::unpacker &unpack,
                                    class Message &msg)
{
  payloadtype = msg.payloadtype;
  topic = msg.topic;
  sourceAddr = msg.sourceAddr;
  destAddr = msg.destAddr;
  next = msg.next;
  msgpack::object obj;

  getitem(unpack, obj)->convert(&operationtype);
  getitem(unpack, obj)->convert(&caller);
  getitem(unpack, obj)->convert(&callerstate);

  getitem(unpack, obj)->convert(&username);
  getitem(unpack, obj)->convert(&domainname);
  getitem(unpack, obj)->convert(&password);

  getitem(unpack, obj)->convert(&argsize);
  getitem(unpack, obj)->convert(&instance);
  getitem(unpack, obj)->convert(&operationid);
  getitem(unpack, obj)->convert(&domainid);
  getitem(unpack, obj)->convert(&userid);
  getitem(unpack, obj)->convert(&tableid);
  getitem(unpack, obj)->convert(&fieldlen);
  getitem(unpack, obj)->convert(&builtincmd);
  getitem(unpack, obj)->convert(&indexid);
  getitem(unpack, obj)->convert(&tableindexid);
  getitem(unpack, obj)->convert(&simple);
  getitem(unpack, obj)->convert(&fieldid);
  getitem(unpack, obj)->convert(&numfields);
  getitem(unpack, obj)->convert(&intdata);
  getitem(unpack, obj)->convert(&status);
  getitem(unpack, obj)->convert((int *)&indextype);
  getitem(unpack, obj)->convert((int *)&fieldtype);
  getitem(unpack, obj)->convert(&argstring);
  string str(sizeof(procs), 0);
  getitem(unpack, obj)->convert(&str);
  memcpy(&procs, str.c_str(), sizeof(procs));
  getitem(unpack, obj)->convert(&pathname);
  getitem(unpack, obj)->convert(&procname);
}

MessageDeadlock::MessageDeadlock()
{
  payloadtype = PAYLOADDEADLOCK;
}

MessageDeadlock::~MessageDeadlock()
{
}

void MessageDeadlock::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  Message::serialize(pack);

  pack.pack_int64(transactionid);
  pack.pack_int64(tainstance);
  pack.pack_int64(transaction_pendingcmdid);
  pack.pack_int64(deadlockchange);
  pack.pack(deadlockNode);

  boost::unordered_set<string>::iterator it;
  map<string, int> locked;

  for (it=nodes.locked.begin(); it != nodes.locked.end(); it++)
  {
    locked[*it] = 0;
  }

  pack.pack(locked);
  map<string, int> waiting;

  for (it=nodes.waiting.begin(); it != nodes.waiting.end(); it++)
  {
    waiting[*it] = 0;
  }

  pack.pack(waiting);
}

void MessageDeadlock::deserialize(msgpack::unpacker &unpack, class Message &msg)
{
  payloadtype = msg.payloadtype;
  topic = msg.topic;
  sourceAddr = msg.sourceAddr;
  destAddr = msg.destAddr;
  next = msg.next;
  msgpack::object obj;

  getitem(unpack, obj)->convert(&transactionid);
  getitem(unpack, obj)->convert(&tainstance);
  getitem(unpack, obj)->convert(&transaction_pendingcmdid);
  getitem(unpack, obj)->convert(&deadlockchange);
  getitem(unpack, obj)->convert(&deadlockNode);

  map<string, int>::iterator it;
  map<string, int> locked;
  getitem(unpack, obj)->convert(&locked);

  for (it=locked.begin(); it != locked.end(); it++)
  {
    nodes.locked.insert(it->first);
  }

  map<string, int> waiting;
  getitem(unpack, obj)->convert(&waiting);

  for (it=waiting.begin(); it != waiting.end(); it++)
  {
    nodes.waiting.insert(it->first);
  }
}

MessageTransaction::MessageTransaction()
{
}

MessageTransaction::~MessageTransaction()
{
}

void MessageTransaction::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  Message::serialize(pack);

  pack.pack_int64(transactionid);
  pack.pack_int64(subtransactionid);
  pack.pack_int64(previoussubtransactionid);
  pack.pack_int64(tainstance);
  pack.pack_int64(domainid);
  pack.pack_int64(transaction_enginecmd);
  pack.pack_int64(transaction_pendingcmdid);
  pack.pack_int64(transaction_tacmdentrypoint);
  pack.pack_int64(engineinstance);
}

void MessageTransaction::deserialize(msgpack::unpacker &unpack,
                                     class Message &msg)
{
  payloadtype = msg.payloadtype;
  topic = msg.topic;
  sourceAddr = msg.sourceAddr;
  destAddr = msg.destAddr;
  next = msg.next;
  msgpack::object obj;

  getitem(unpack, obj)->convert(&transactionid);
  getitem(unpack, obj)->convert(&subtransactionid);
  getitem(unpack, obj)->convert(&previoussubtransactionid);
  getitem(unpack, obj)->convert(&tainstance);
  getitem(unpack, obj)->convert(&domainid);
  getitem(unpack, obj)->convert(&transaction_enginecmd);
  getitem(unpack, obj)->convert(&transaction_pendingcmdid);
  getitem(unpack, obj)->convert(&transaction_tacmdentrypoint);
  getitem(unpack, obj)->convert(&engineinstance);
}

void MessageTransaction::print(int64_t nodeid)
{
  Message::print(nodeid);
  printf("%s %i MessageTransaction: %li %li %li %li %li %li %li %li\n", __FILE__, __LINE__,
         transactionid, subtransactionid, tainstance, domainid, transaction_enginecmd,
         transaction_pendingcmdid, transaction_tacmdentrypoint, engineinstance);
}


MessageSubtransactionCmd::MessageSubtransactionCmd()
{
}

MessageSubtransactionCmd::~MessageSubtransactionCmd()
{
}

void MessageSubtransactionCmd::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  MessageTransaction::serialize(pack);

  pack.pack_int64(cmd.status);

  if (cmd.isrow==true)
  {
    pack.pack_true();
  }
  else
  {
    pack.pack_false();
  }

  pack.pack_int64(cmd.rowid);
  pack.pack_int64(cmd.tableid);
  pack.pack(cmd.row);
  pack.pack_int((int)cmd.locktype);
  pack.pack_int64(cmd.forward_rowid);
  pack.pack_int64(cmd.forward_engineid);
  pack.pack_int64(cmd.fieldid);
  pack.pack_int64(cmd.engineid);
  serFieldValue(cmd.fieldVal, pack);
  serIndexHits(cmd.indexHits, pack);
  serSearchParams(cmd.searchParameters, pack);
  pack.pack(cmd.rowids);
  serReturnRows(cmd.returnRows, pack);
}

void MessageSubtransactionCmd::deserialize(msgpack::unpacker &unpack,
    class Message &msg)
{
  MessageTransaction::deserialize(unpack, msg);
  msgpack::object obj;

  getitem(unpack, obj)->convert(&cmd.status);
  getitem(unpack, obj)->convert(&cmd.isrow);
  getitem(unpack, obj)->convert(&cmd.rowid);
  getitem(unpack, obj)->convert(&cmd.tableid);
  getitem(unpack, obj)->convert(&cmd.row);
  getitem(unpack, obj)->convert((int *)&cmd.locktype);
  getitem(unpack, obj)->convert(&cmd.forward_rowid);
  getitem(unpack, obj)->convert(&cmd.forward_engineid);
  getitem(unpack, obj)->convert(&cmd.fieldid);
  getitem(unpack, obj)->convert(&cmd.engineid);
  deserFieldValue(unpack, cmd.fieldVal);
  deserIndexHits(unpack, cmd.indexHits);
  deserSearchParams(unpack, cmd.searchParameters);
  getitem(unpack, obj)->convert(&cmd.rowids);
  deserReturnRows(unpack, cmd.returnRows);
}

void MessageSubtransactionCmd::serIndexHits(vector<nonLockingIndexEntry_s> &indexHits,
    msgpack::packer<msgpack::sbuffer> &pack)
{
  size_t s = indexHits.size();
  vector<string> v(s);

  for (size_t n=0; n < s; n++)
  {
    string str(sizeof(nonLockingIndexEntry_s), 0);
    memcpy(&str[0], &indexHits[n], sizeof(nonLockingIndexEntry_s));
    v[n] = str;
  }

  pack.pack(v);
}

void MessageSubtransactionCmd::deserIndexHits(msgpack::unpacker &unpack,
    vector<nonLockingIndexEntry_s> &indexHits)
{
  msgpack::object obj;

  vector<string> v;
  getitem(unpack, obj)->convert(&v);
  size_t s = v.size();
  indexHits.resize(s);

  for (size_t n=0; n < s; n++)
  {
    memcpy(&indexHits[n], (nonLockingIndexEntry_s *)v[n].c_str(),
           sizeof(nonLockingIndexEntry_s));
  }
}

void MessageSubtransactionCmd::serSearchParams(searchParams_s &searchParameters,
    msgpack::packer<msgpack::sbuffer> &pack)
{
  pack.pack_int((int)searchParameters.op);
  size_t s = searchParameters.values.size();
  vector<string> v(s);

  for (size_t n=0; n < s; n++)
  {
    serFieldValue(searchParameters.values[n], v[n]);
  }

  pack.pack(v);
  pack.pack(searchParameters.regexString);
}

void MessageSubtransactionCmd::deserSearchParams(msgpack::unpacker &unpack,
    searchParams_s &searchParameters)
{
  msgpack::object obj;

  getitem(unpack, obj)->convert((int *)&searchParameters.op);
  vector<string> v;
  getitem(unpack, obj)->convert(&v);
  size_t s = v.size();
  searchParameters.values.resize(s);


  for (size_t n=0; n < s; n++)
  {
    deserFieldValue(v[n], searchParameters.values[n]);
  }

  getitem(unpack, obj)->convert(&searchParameters.regexString);
}

void MessageSubtransactionCmd::serReturnRows(vector<returnRow_s> &returnRows,
    msgpack::packer<msgpack::sbuffer> &pack)
{
  size_t s = returnRows.size();
  vector<string> v(s);

  for (size_t n=0; n < s; n++)
  {
    msgpack::sbuffer sbuf;
    msgpack::packer<msgpack::sbuffer> pack2(sbuf);
    pack2.pack_int64(returnRows[n].rowid);
    pack2.pack_int64(returnRows[n].previoussubtransactionid);
    pack2.pack_int((int)returnRows[n].locktype);
    pack2.pack(returnRows[n].row);
    v[n].assign((const char *)sbuf.data(), sbuf.size());
  }

  pack.pack(v);
}

void MessageSubtransactionCmd::deserReturnRows(msgpack::unpacker &unpack,
    vector<returnRow_s> &returnRows)
{
  msgpack::object obj;

  vector<string> v;
  getitem(unpack, obj)->convert(&v);
  size_t s = v.size();
  returnRows.resize(s);

  for (size_t n=0; n < s; n++)
  {
    msgpack::sbuffer sbuf;
    sbuf.write(v[n].c_str(), v[n].size());
    msgpack::unpacker unpack2;
    unpack2.reserve_buffer(sbuf.size());
    memcpy(unpack2.buffer(), sbuf.data(), sbuf.size());
    unpack2.buffer_consumed(sbuf.size());
    msgpack::object obj2;

    getitem(unpack2, obj2)->convert(&returnRows[n].rowid);
    getitem(unpack2, obj2)->convert(&returnRows[n].previoussubtransactionid);
    getitem(unpack2, obj2)->convert((int *)&returnRows[n].locktype);
    getitem(unpack2, obj2)->convert(&returnRows[n].row);
  }
}

void MessageSubtransactionCmd::print(int64_t nodeid)
{
  MessageTransaction::print(nodeid);
}

MessageCommitRollback::MessageCommitRollback()
{
}

MessageCommitRollback::~MessageCommitRollback()
{
}

void MessageCommitRollback::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  MessageTransaction::serialize(pack);

  serRofs(rofs, pack);
}

void MessageCommitRollback::deserialize(msgpack::unpacker &unpack,
                                        class Message &msg)
{
  MessageTransaction::deserialize(unpack, msg);
  msgpack::object obj;

  deserRofs(unpack, rofs);
}

void MessageCommitRollback::serRofs(vector<rowOrField_s> &rofs,
                                    msgpack::packer<msgpack::sbuffer> &pack)
{
  size_t s = rofs.size();
  vector<string> v(s);

  for (size_t n=0; n < s; n++)
  {
    msgpack::sbuffer sbuf;
    msgpack::packer<msgpack::sbuffer> pack2(sbuf);

    if (rofs[n].isrow==true)
    {
      pack2.pack_true();
    }
    else
    {
      pack2.pack_false();
    }

    pack2.pack_int64(rofs[n].tableid);
    pack2.pack_int64(rofs[n].rowid);
    pack2.pack_int64(rofs[n].fieldid);
    serFieldValue(rofs[n].fieldVal, pack2);
    pack2.pack_int64(rofs[n].engineid);

    if (rofs[n].deleteindexentry==true)
    {
      pack2.pack_true();
    }
    else
    {
      pack2.pack_false();
    }

    if (rofs[n].isnotaddunique==true)
    {
      pack2.pack_true();
    }
    else
    {
      pack2.pack_false();
    }

    if (rofs[n].isreplace==true)
    {
      pack2.pack_true();
    }
    else
    {
      pack2.pack_false();
    }

    pack2.pack_int64(rofs[n].newrowid);
    pack2.pack_int64(rofs[n].newengineid);

    v[n].assign((const char *)sbuf.data(), sbuf.size());
  }

  pack.pack(v);
}

void MessageCommitRollback::deserRofs(msgpack::unpacker &unpack,
                                      vector<rowOrField_s> &rofs)
{
  msgpack::object obj;

  vector<string> v;
  getitem(unpack, obj)->convert(&v);
  size_t s = v.size();
  rofs.resize(s);

  for (size_t n=0; n < s; n++)
  {
    msgpack::sbuffer sbuf;
    sbuf.write(v[n].c_str(), v[n].size());
    msgpack::unpacker unpack2;
    unpack2.reserve_buffer(sbuf.size());
    memcpy(unpack2.buffer(), sbuf.data(), sbuf.size());
    unpack2.buffer_consumed(sbuf.size());
    msgpack::object obj2;

    getitem(unpack2, obj2)->convert(&rofs[n].isrow);
    getitem(unpack2, obj2)->convert(&rofs[n].tableid);
    getitem(unpack2, obj2)->convert(&rofs[n].rowid);
    getitem(unpack2, obj2)->convert(&rofs[n].fieldid);
    // fieldValue fieldVal
    deserFieldValue(unpack2, rofs[n].fieldVal);
    getitem(unpack2, obj2)->convert(&rofs[n].engineid);
    getitem(unpack2, obj2)->convert(&rofs[n].deleteindexentry);
    getitem(unpack2, obj2)->convert(&rofs[n].isnotaddunique);
    getitem(unpack2, obj2)->convert(&rofs[n].isreplace);
    getitem(unpack2, obj2)->convert(&rofs[n].newrowid);
    getitem(unpack2, obj2)->convert(&rofs[n].newengineid);
  }
}

MessageDispatch::MessageDispatch()
{
  topic = TOPIC_DISPATCH;
  payloadtype = PAYLOADDISPATCH;
}

MessageDispatch::~MessageDispatch()
{
}

/* order is as follows, after base Message class:
 * transactionid (int64_t)
 * pidsids is map of partitionid->subtransactionid
 *
 * based on size of that vector, 3 sets of vectors:
 * 1 with record's rowid & primitive & tableid & previoussubtransactionid as
 * string, the others with row & oldrow, which are strings
 * serialize the key (partitionid) of records, then the 3 vectors
 */
void MessageDispatch::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  Message::serialize(pack);

  pack.pack_int64(transactionid);
  pack.pack_int64(domainid);
  pack.pack(pidsids);

  string str(sizeof(int64_t)*3+sizeof(pendingprimitive_e), 0);
  map< int64_t, vector<record_s> >::iterator it;

  for (it = records.begin(); it != records.end(); it++)
  {
    vector<record_s> &vrRef = it->second;
    vector<string> vr1;
    vector<string> vr2;
    vector<string> vr3;

    for (size_t n=0; n < vrRef.size(); n++)
    {
      memcpy(&str[0], &vrRef[n].rowid, sizeof(vrRef[n].rowid));
      memcpy(&str[sizeof(vrRef[n].rowid)], &vrRef[n].primitive,
             sizeof(vrRef[n].primitive));
      memcpy(&str[sizeof(vrRef[n].rowid)+sizeof(vrRef[n].primitive)],
             &vrRef[n].tableid, sizeof(vrRef[n].tableid));
      memcpy(&str[sizeof(vrRef[n].rowid)+sizeof(vrRef[n].primitive)+sizeof(vrRef[n].tableid)],
             &vrRef[n].previoussubtransactionid,
             sizeof(vrRef[n].previoussubtransactionid));
      vr1.push_back(str);
      vr2.push_back(vrRef[n].row);
      vr3.push_back(vrRef[n].oldrow);
    }

    pack.pack_int64(it->first);
    pack.pack(vr1);
    pack.pack(vr2);
    pack.pack(vr3);
  }
}

void MessageDispatch::deserialize(msgpack::unpacker &unpack, class Message &msg)
{
  payloadtype = msg.payloadtype;
  topic = msg.topic;
  sourceAddr = msg.sourceAddr;
  destAddr = msg.destAddr;
  next = msg.next;
  msgpack::object obj;

  getitem(unpack, obj)->convert(&transactionid);
  getitem(unpack, obj)->convert(&domainid);
  getitem(unpack, obj)->convert(&pidsids);

  int64_t partitionid;
  vector<string> v1;
  vector<string> v2;
  vector<string> v3;
  record_s record;

  for (size_t n=0; n < pidsids.size(); n++)
  {
    getitem(unpack, obj)->convert(&partitionid);
    getitem(unpack, obj)->convert(&v1);
    getitem(unpack, obj)->convert(&v2);
    getitem(unpack, obj)->convert(&v3);

    vector<record_s> vr;

    for (size_t m=0; m < v2.size(); m++)
    {
      memcpy(&record.rowid, &v1[m][0], sizeof(record.rowid));
      memcpy(&record.primitive, &v1[m][sizeof(record.rowid)],
             sizeof(record.primitive));
      memcpy(&record.tableid,
             &v1[m][sizeof(record.rowid)+sizeof(record.primitive)],
             sizeof(record.tableid));
      memcpy(&record.previoussubtransactionid,
             &v1[m][sizeof(record.rowid)+sizeof(record.primitive)+
                    sizeof(record.tableid)], sizeof(record.previoussubtransactionid));
      record.row  = v2[m];
      record.oldrow = v3[m];
      vr.push_back(record);
    }

    records[partitionid]=vr;
  }
}

MessageAckDispatch::MessageAckDispatch()
{
}

MessageAckDispatch::MessageAckDispatch(int64_t transactionidarg, int statusarg)
  : transactionid(transactionidarg), status(statusarg)
{
  topic = TOPIC_ACKDISPATCH;
  payloadtype = PAYLOADACKDISPATCH;
}

MessageAckDispatch::~MessageAckDispatch()
{
}

void MessageAckDispatch::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  Message::serialize(pack);

  pack.pack_int64(transactionid);
  pack.pack_int(status);
}

void MessageAckDispatch::deserialize(msgpack::unpacker &unpack,
                                     class Message &msg)
{
  payloadtype = msg.payloadtype;
  topic = msg.topic;
  sourceAddr = msg.sourceAddr;
  destAddr = msg.destAddr;
  next = msg.next;
  msgpack::object obj;

  getitem(unpack, obj)->convert(&transactionid);
  getitem(unpack,obj)->convert(&status);
}

MessageApply::MessageApply()
{
}

MessageApply::MessageApply(int64_t subtransactionidarg, int64_t applieridarg,
                           int64_t domainidarg) : subtransactionid(subtransactionidarg),
  applierid(applieridarg), domainid(domainidarg)
{
  topic = TOPIC_APPLY;
  payloadtype = PAYLOADAPPLY;
}

MessageApply::~MessageApply()
{
}

void MessageApply::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  Message::serialize(pack);

  // vectors of record_s rows
  pack.pack_int64(subtransactionid);
  pack.pack_int64(applierid);
  pack.pack_int64(domainid);
  // 3 vectors of strings
  vector<string> vr1;
  vector<string> vr2;
  vector<string> vr3;
  string str(sizeof(int64_t)*3+sizeof(pendingprimitive_e), 0);

  for (size_t n=0; n < rows.size(); n++)
  {
    memcpy(&str[0], &rows[n].rowid, sizeof(rows[n].rowid));
    memcpy(&str[sizeof(rows[n].rowid)], &rows[n].primitive,
           sizeof(rows[n].primitive));
    memcpy(&str[sizeof(rows[n].rowid)+sizeof(rows[n].primitive)],
           &rows[n].tableid, sizeof(rows[n].tableid));
    memcpy(&str[sizeof(rows[n].rowid)+sizeof(rows[n].primitive)+sizeof(rows[n].tableid)],
           &rows[n].previoussubtransactionid,
           sizeof(rows[n].previoussubtransactionid));
    vr1.push_back(str);
    vr2.push_back(rows[n].row);
    vr3.push_back(rows[n].oldrow);
  }

  pack.pack(vr1);
  pack.pack(vr2);
  pack.pack(vr3);

  // 2 vectors of strings for indices: 1 with field values, 1 with tableid, entry, fieldid & flags
  vector<string> vr4;
  vector<string> vr5;
  string str4;
  string str5(sizeof(nonLockingIndexEntry_s)+sizeof(char)+2*sizeof(int64_t), 0);

  for (size_t n=0; n < indices.size(); n++)
  {
    serFieldValue(indices[n].fieldVal, str4);
    vr4.push_back(str4);
    memcpy(&str5[0], &indices[n].entry, sizeof(indices[n].entry));
    memcpy(&str5[sizeof(indices[n].entry)], &indices[n].flags,
           sizeof(indices[n].flags));
    memcpy(&str5[sizeof(indices[n].entry)+sizeof(indices[n].flags)],
           &indices[n].tableid, sizeof(indices[n].tableid));
    memcpy(&str5[sizeof(indices[n].entry)+sizeof(indices[n].flags)+
                 sizeof(indices[n].tableid)], &indices[n].fieldid,
           sizeof(indices[n].fieldid));
    vr5.push_back(str5);
  }

  pack.pack(vr4);
  pack.pack(vr5);
}

// an int and 5 vectors of strings
void MessageApply::deserialize(msgpack::unpacker &unpack,
                               class Message &msg)
{
  payloadtype = msg.payloadtype;
  topic = msg.topic;
  sourceAddr = msg.sourceAddr;
  destAddr = msg.destAddr;
  next = msg.next;
  msgpack::object obj;

  getitem(unpack, obj)->convert(&subtransactionid);
  getitem(unpack, obj)->convert(&applierid);
  getitem(unpack, obj)->convert(&domainid);
  vector<string> vr1;
  vector<string> vr2;
  vector<string> vr3;
  vector<string> vr4;
  vector<string> vr5;
  getitem(unpack, obj)->convert(&vr1);
  getitem(unpack, obj)->convert(&vr2);
  getitem(unpack, obj)->convert(&vr3);
  getitem(unpack, obj)->convert(&vr4);
  getitem(unpack, obj)->convert(&vr5);
  MessageDispatch::record_s record;
  applyindex_s indexinfo;

  for (size_t n = 0; n < vr1.size(); n++)
  {
    memcpy(&record.rowid, &vr1[n][0], sizeof(record.rowid));
    memcpy(&record.primitive, &vr1[n][sizeof(record.rowid)],
           sizeof(record.primitive));
    memcpy(&record.tableid,
           &vr1[n][sizeof(record.rowid)+sizeof(record.primitive)],
           sizeof(record.tableid));
    memcpy(&record.previoussubtransactionid,
           &vr1[n][sizeof(record.rowid)+sizeof(record.primitive)+
                   sizeof(record.tableid)], sizeof(record.previoussubtransactionid));
    record.row = vr2[n];
    record.oldrow = vr3[n];
    rows.push_back(record);
  }

  for (size_t n = 0; n < vr4.size(); n++)
  {
    deserFieldValue(vr4[n], indexinfo.fieldVal);
    // entry then flags
    memcpy(&indexinfo.entry, &vr5[n][0], sizeof(indexinfo.entry));
    memcpy(&indexinfo.flags, &vr5[n][sizeof(indexinfo.entry)],
           sizeof(indexinfo.flags));
    memcpy(&indexinfo.tableid, &vr5[n][sizeof(indexinfo.entry)+
                                       sizeof(indexinfo.flags)], sizeof(indexinfo.tableid));
    memcpy(&indexinfo.fieldid, &vr5[n][sizeof(indexinfo.entry)+
                                       sizeof(indexinfo.flags)+sizeof(indexinfo.tableid)],
           sizeof(indexinfo.fieldid));

    indices.push_back(indexinfo);
  }
}

void MessageApply::setisaddflag(char *c)
{
  *c |= 1 << ADDFLAG;
}

char MessageApply::getisaddflag(char c)
{
  return c & 1 << ADDFLAG;
}

void MessageApply::cleariaddflag(char *c)
{
  *c &= ~(1 << ADDFLAG);
}


MessageAckApply::MessageAckApply()
{
}

MessageAckApply::MessageAckApply(int64_t subtransactionidarg,
                                 int64_t applieridarg, int64_t partitionidarg, int statusarg) :
  subtransactionid(subtransactionidarg), applierid(applieridarg),
  partitionid(partitionidarg), status(statusarg)
{
  topic = TOPIC_ACKAPPLY;
  payloadtype = PAYLOADACKAPPLY;
}

MessageAckApply::~MessageAckApply()
{
}

void MessageAckApply::serialize(msgpack::packer<msgpack::sbuffer> &pack)
{
  Message::serialize(pack);

  // vectors of record_s rows
  pack.pack_int64(subtransactionid);
  pack.pack_int64(applierid);
  pack.pack_int64(partitionid);
  pack.pack_int(status);
}

void MessageAckApply::deserialize(msgpack::unpacker &unpack,
                                  class Message &msg)
{
  payloadtype = msg.payloadtype;
  topic = msg.topic;
  sourceAddr = msg.sourceAddr;
  destAddr = msg.destAddr;
  next = msg.next;
  msgpack::object obj;

  getitem(unpack, obj)->convert(&subtransactionid);
  getitem(unpack, obj)->convert(&applierid);
  getitem(unpack, obj)->convert(&partitionid);
  getitem(unpack, obj)->convert(&status);
}
