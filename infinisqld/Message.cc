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

Message::Message()
{
}

Message::~Message()
{
}

size_t Message::size()
{
  return SerializedMessage::sersize(messageStruct);
}

string *Message::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void Message::package(class SerializedMessage &serobj)
{
  serobj.ser(messageStruct);
}

void Message::unpack(SerializedMessage &serobj)
{
  serobj.des(messageStruct);
}

void Message::clear()
{
  messageStruct={};
}

class Message *Message::des(string *serstr)
{
  message_s tmpheader;
  memcpy(&tmpheader, &serstr->at(0), sizeof(tmpheader));

  class Message *msg;
  class SerializedMessage serobj(serstr);
  switch(serobj.getpayloadtype())
  {
    case PAYLOADMESSAGE:
      msg = (class Message *)new class Message;
      msg->unpack(serobj);
      break;

    case PAYLOADSOCKET:
      msg = (class Message *)new class MessageSocket;
      ((class MessageSocket *)msg)->unpack(serobj);
      break;

    case PAYLOADUSERSCHEMA:
      msg = (class Message *)new class MessageUserSchema;
      ((class MessageUserSchema *)msg)->unpack(serobj);
      break;

    case PAYLOADDEADLOCK:
      msg = (class Message *)new class MessageDeadlock;
      ((class MessageDeadlock *)msg)->unpack(serobj);
      break;

    case PAYLOADSUBTRANSACTION:
      msg = (class Message *)new class MessageSubtransactionCmd;
      ((class MessageSubtransactionCmd *)msg)->unpack(serobj);
      break;

    case PAYLOADCOMMITROLLBACK:
      msg = (class Message *)new class MessageCommitRollback;
      ((class MessageCommitRollback *)msg)->unpack(serobj);
      break;

    case PAYLOADDISPATCH:
      msg = (class Message *)new class MessageDispatch;
      ((class MessageDispatch *)msg)->unpack(serobj);
      break;

    case PAYLOADACKDISPATCH:
      msg = (class Message *)new class MessageAckDispatch;
      ((class MessageAckDispatch *)msg)->unpack(serobj);
      break;

    case PAYLOADAPPLY:
      msg = (class Message *)new class MessageApply;
      ((class MessageApply *)msg)->unpack(serobj);
      break;

    case PAYLOADACKAPPLY:
      msg = (class Message *)new class MessageAckApply;
      ((class MessageAckApply *)msg)->unpack(serobj);
      break;

    default:
      printf("%s %i anomaly %i\n", __FILE__, __LINE__, tmpheader.payloadtype);
      delete serstr;
      return NULL;
  }

  delete serstr;
  return msg;
}

string *Message::sermsg()
{
  string *serstr;
  
  switch (messageStruct.payloadtype)
  {
    case PAYLOADMESSAGE:
      serstr=((class Message *)this)->ser();
      break;

    case PAYLOADSOCKET:
      serstr=((class MessageSocket *)this)->ser();
      break;

    case PAYLOADUSERSCHEMA:
      serstr=((class MessageUserSchema *)this)->ser();
      break;

    case PAYLOADDEADLOCK:
      serstr=((class MessageDeadlock *)this)->ser();
      break;

    case PAYLOADSUBTRANSACTION:
      serstr=((class MessageSubtransactionCmd *)this)->ser();
      break;

    case PAYLOADCOMMITROLLBACK:
      serstr=((class MessageCommitRollback *)this)->ser();
      break;

    case PAYLOADDISPATCH:
      serstr=((class MessageDispatch *)this)->ser();
      break;

    case PAYLOADACKDISPATCH:
      serstr=((class MessageAckDispatch *)this)->ser();
      break;

    case PAYLOADAPPLY:
      serstr=((class MessageApply *)this)->ser();
      break;

    case PAYLOADACKAPPLY:
      serstr=((class MessageAckApply *)this)->ser();
      break;

    default:
      printf("%s %i anomaly %i\n", __FILE__, __LINE__,
              messageStruct.payloadtype);
      serstr=NULL;
  }

  return serstr;
}

void Message::setEnvelope(const Topology::addressStruct &source,
                          const Topology::addressStruct &dest, class Message &msg)
{
  msg.messageStruct.sourceAddr = source;
  msg.messageStruct.destAddr = dest;
}

MessageSocket::MessageSocket()
{
}

MessageSocket::MessageSocket(int socketarg, uint32_t eventsarg,
    listenertype_e listenertypearg, int64_t nodeidarg, topic_e topicarg)
{
  messageStruct.topic=topicarg;
  messageStruct.payloadtype=PAYLOADSOCKET;
  messageStruct.destAddr.nodeid=nodeidarg;
  socketStruct={socketarg, eventsarg, listenertypearg};
}

MessageSocket::~MessageSocket()
{
}

size_t MessageSocket::size()
{
  return Message::size() + SerializedMessage::sersize(socketStruct);
}

string *MessageSocket::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void MessageSocket::package(class SerializedMessage &serobj)
{
  Message::package(serobj);
  serobj.ser(socketStruct);
}

void MessageSocket::unpack(SerializedMessage &serobj)
{
  Message::unpack(serobj);
  serobj.des(messageStruct);
}

void MessageSocket::clear()
{
  Message::clear();
  socketStruct={};
}

MessageUserSchema::MessageUserSchema()
{
}

MessageUserSchema::MessageUserSchema(topic_e topicarg)
{
  messageStruct.topic = topicarg;
  messageStruct.payloadtype = PAYLOADUSERSCHEMA;
}

MessageUserSchema::~MessageUserSchema()
{
}

size_t MessageUserSchema::size()
{
  return (size_t) (Message::size() + SerializedMessage::sersize(userschemaStruct) +
          SerializedMessage::sersize(procs) +
          SerializedMessage::sersize(argstring) +
          SerializedMessage::sersize(pathname) +
          SerializedMessage::sersize(procname) +
          SerializedMessage::sersize(username) +
          SerializedMessage::sersize(domainname) +
          SerializedMessage::sersize(password));
}

string *MessageUserSchema::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void MessageUserSchema::package(class SerializedMessage &serobj)
{
  Message::package(serobj);
  serobj.ser(userschemaStruct);
  serobj.ser(procs);
  serobj.ser(argstring);
  serobj.ser(pathname);
  serobj.ser(procname);
  serobj.ser(username);
  serobj.ser(domainname);
  serobj.ser(password);
}

void MessageUserSchema::unpack(SerializedMessage &serobj)
{
  Message::unpack(serobj);
  serobj.des(userschemaStruct);
  serobj.des(procs);
  serobj.des(argstring);
  serobj.des(pathname);
  serobj.des(procname);
  serobj.des(username);
  serobj.des(domainname);
  serobj.des(password);
}

void MessageUserSchema::clear()
{
  Message::clear();
  userschemaStruct={};
  procs={};
  pathname.clear();
  procname.clear();
  username.clear();
  domainname.clear();
  password.clear();
}

MessageDeadlock::MessageDeadlock()
{
  messageStruct.payloadtype = PAYLOADDEADLOCK;
}

MessageDeadlock::~MessageDeadlock()
{
}

size_t MessageDeadlock::size()
{
  return Message::size() + SerializedMessage::sersize(deadlockStruct) +
          SerializedMessage::sersize(deadlockNode) +
          SerializedMessage::sersize(nodes);
}

string *MessageDeadlock::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void MessageDeadlock::package(class SerializedMessage &serobj)
{
  Message::package(serobj);
  serobj.ser(deadlockStruct);
  serobj.ser(deadlockNode);
  serobj.ser(nodes);
}

void MessageDeadlock::unpack(SerializedMessage &serobj)
{
  Message::unpack(serobj);
  serobj.des(deadlockStruct);
  serobj.des(deadlockNode);
  serobj.des(nodes);
}

void MessageDeadlock::clear()
{
  Message::clear();
  deadlockStruct={};
  deadlockNode.clear();
  nodes={};
}

MessageTransaction::MessageTransaction()
{
}

MessageTransaction::~MessageTransaction()
{
}


size_t MessageTransaction::size()
{
  return Message::size() + SerializedMessage::sersize(transactionStruct);
}

string *MessageTransaction::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void MessageTransaction::package(class SerializedMessage &serobj)
{
  Message::package(serobj);
  serobj.ser(transactionStruct);
}

void MessageTransaction::unpack(SerializedMessage &serobj)
{
  Message::unpack(serobj);
  serobj.des(transactionStruct);
}

void MessageTransaction::clear()
{
  Message::clear();
  transactionStruct={};
}

MessageSubtransactionCmd::MessageSubtransactionCmd()
{
}

MessageSubtransactionCmd::~MessageSubtransactionCmd()
{
}

size_t MessageSubtransactionCmd::size()
{
  return MessageTransaction::size() +
          SerializedMessage::sersize(subtransactionStruct) +
          SerializedMessage::sersize(row) +
          SerializedMessage::sersize(fieldVal) +
          SerializedMessage::sersize(indexHits) +
          SerializedMessage::sersize(searchParameters) +
          SerializedMessage::sersize(rowids) +
          SerializedMessage::sersize(returnRows);
}

string *MessageSubtransactionCmd::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void MessageSubtransactionCmd::package(class SerializedMessage &serobj)
{
  MessageTransaction::package(serobj);
  serobj.ser(subtransactionStruct);
  serobj.ser(row);
  serobj.ser(fieldVal);
  serobj.ser(indexHits);
  serobj.ser(searchParameters);
  serobj.ser(rowids);
  serobj.ser(returnRows);
}

void MessageSubtransactionCmd::unpack(SerializedMessage &serobj)
{
  MessageTransaction::unpack(serobj);
  serobj.des(subtransactionStruct);
  serobj.des(row);
  serobj.des(fieldVal);
  serobj.des(indexHits);
  serobj.des(searchParameters);
  serobj.des(rowids);
  serobj.des(returnRows);
}

void MessageSubtransactionCmd::clear()
{
  MessageTransaction::clear();
  subtransactionStruct={};
  row.clear();
  fieldVal={};
  indexHits.clear();
  searchParameters={};
  rowids.clear();
  returnRows.clear();
}

MessageCommitRollback::MessageCommitRollback()
{
}

MessageCommitRollback::~MessageCommitRollback()
{
}

size_t MessageCommitRollback::size()
{
  return MessageTransaction::size() +
          SerializedMessage::sersize(rofs);
}

string *MessageCommitRollback::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void MessageCommitRollback::package(class SerializedMessage &serobj)
{
  MessageTransaction::package(serobj);
  serobj.ser(rofs);
}

void MessageCommitRollback::unpack(SerializedMessage &serobj)
{
  MessageTransaction::unpack(serobj);
  serobj.des(rofs);
}

void MessageCommitRollback::clear()
{
  MessageTransaction::clear();
  rofs.clear();
}

MessageDispatch::MessageDispatch()
{
  messageStruct.topic = TOPIC_DISPATCH;
  messageStruct.payloadtype = PAYLOADDISPATCH;
}

MessageDispatch::~MessageDispatch()
{
}

size_t MessageDispatch::size()
{
  return Message::size() + SerializedMessage::sersize(dispatchStruct) +
          SerializedMessage::sersize(pidsids) +
          SerializedMessage::sersize(records);
}

string *MessageDispatch::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void MessageDispatch::package(class SerializedMessage &serobj)
{
  Message::package(serobj);
  serobj.ser(dispatchStruct);
  serobj.ser(pidsids);
  serobj.ser(records);
}

void MessageDispatch::unpack(SerializedMessage &serobj)
{
  Message::unpack(serobj);
  serobj.des(dispatchStruct);
  serobj.des(pidsids);
  serobj.des(records);
}

void MessageDispatch::clear()
{
  Message::clear();
  dispatchStruct={};
  pidsids.clear();
  records.clear();
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

MessageAckDispatch::MessageAckDispatch()
{
}

MessageAckDispatch::MessageAckDispatch(int64_t transactionidarg, int statusarg)
{
  messageStruct.topic = TOPIC_ACKDISPATCH;
  messageStruct.payloadtype = PAYLOADACKDISPATCH;
  ackdispatchStruct={transactionidarg, statusarg};
}

MessageAckDispatch::~MessageAckDispatch()
{
}

size_t MessageAckDispatch::size()
{
  return Message::size() + SerializedMessage::sersize(ackdispatchStruct);
}

string *MessageAckDispatch::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void MessageAckDispatch::package(class SerializedMessage &serobj)
{
  Message::package(serobj);
  serobj.ser(ackdispatchStruct);
}

void MessageAckDispatch::unpack(SerializedMessage &serobj)
{
  Message::unpack(serobj);
  serobj.des(ackdispatchStruct);
}

void MessageAckDispatch::clear()
{
  Message::clear();
  ackdispatchStruct={};
}

MessageApply::MessageApply()
{
}

MessageApply::MessageApply(int64_t subtransactionidarg, int64_t applieridarg,
                           int64_t domainidarg)
{
  messageStruct.topic = TOPIC_APPLY;
  messageStruct.payloadtype = PAYLOADAPPLY;
  applyStruct={subtransactionidarg, applieridarg, domainidarg};
}

MessageApply::~MessageApply()
{
}

size_t MessageApply::size()
{
  return Message::size() + SerializedMessage::sersize(applyStruct) +
          SerializedMessage::sersize(rows) +
          SerializedMessage::sersize(indices);
}

string *MessageApply::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void MessageApply::package(class SerializedMessage &serobj)
{
  Message::package(serobj);
  serobj.ser(applyStruct);
  serobj.ser(rows);
  serobj.ser(indices);
}

void MessageApply::unpack(SerializedMessage &serobj)
{
  Message::unpack(serobj);
  serobj.des(applyStruct);
  serobj.des(rows);
  serobj.des(indices);
}

void MessageApply::clear()
{
  Message::clear();
  applyStruct={};
  rows.clear();
  indices={};
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
        int64_t applieridarg, int64_t partitionidarg,
        int statusarg)
{
  messageStruct.topic = TOPIC_ACKAPPLY;
  messageStruct.payloadtype = PAYLOADACKAPPLY;
  ackapplyStruct={subtransactionidarg, applieridarg, partitionidarg, statusarg};
}

MessageAckApply::~MessageAckApply()
{
}

size_t MessageAckApply::size()
{
  return Message::size() + SerializedMessage::sersize(ackapplyStruct);
}

string *MessageAckApply::ser()
{
  class SerializedMessage serobj(size());
  package(serobj);
  if (serobj.data->size() != serobj.pos)
  {
    fprintf(logfile, "%s %i ser %i size %lu pos %lu\n", __FILE__, __LINE__, serobj.getpayloadtype(), serobj.data->size(), serobj.pos);
  }
  return serobj.data;
}

void MessageAckApply::package(class SerializedMessage &serobj)
{
  Message::package(serobj);
  serobj.ser(ackapplyStruct);
}

void MessageAckApply::unpack(SerializedMessage &serobj)
{
  Message::unpack(serobj);
  serobj.des(ackapplyStruct);
}

void MessageAckApply::clear()
{
  Message::clear();
  ackapplyStruct={};
}

SerializedMessage::SerializedMessage(size_t sizearg) : size(sizearg), pos(0)
{
  data=new string(size, char(0));
}

SerializedMessage::SerializedMessage(string *dataarg) : pos(0), data(dataarg)
{
  size=data->size();
}

SerializedMessage::~SerializedMessage()
{
}

payloadtype_e SerializedMessage::getpayloadtype()
{
  Message::message_s tmpheader;
  memcpy(&tmpheader, data->c_str(), sizeof(tmpheader));
  return tmpheader.payloadtype;
}

// raw
void SerializedMessage::ser(size_t s, void *dataptr)
{
  memcpy(&data->at(pos), dataptr, s);
  pos += s;
}

void SerializedMessage::des(size_t s, void *dataptr)
{
  memcpy(dataptr, &data->at(pos), s);
  pos += s;
}

//pods
void SerializedMessage::ser(int64_t d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(int64_t d)
{
  return sizeof(d);
}

void SerializedMessage::des(int64_t *d)
{
  memcpy(d, &data->at(pos), sizeof(*d));
  pos += sizeof(*d);
}

void SerializedMessage::ser(int32_t d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);  
}

size_t SerializedMessage::sersize(int32_t d)
{
  return sizeof(d);  
}

void SerializedMessage::des(int32_t *d)
{
  memcpy(d, &data->at(pos), sizeof(*d));
  pos += sizeof(*d);  
}

void SerializedMessage::ser(int16_t d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);  
}

size_t SerializedMessage::sersize(int16_t d)
{
  return sizeof(d);  
}

void SerializedMessage::des(int16_t *d)
{
  memcpy(d, &data->at(pos), sizeof(*d));
  pos += sizeof(*d);  
}

void SerializedMessage::ser(int8_t d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);  
}

size_t SerializedMessage::sersize(int8_t d)
{
  return sizeof(d);  
}

void SerializedMessage::des(int8_t *d)
{
  memcpy(d, &data->at(pos), sizeof(*d));
  pos += sizeof(*d);
}

// containers
void SerializedMessage::ser(const string &d)
{
  ser((int64_t)d.size());
  if (d.size())
  {
    memcpy(&data->at(pos), d.c_str(), d.size());
    pos += d.size();
  }
}

size_t SerializedMessage::sersize(const string &d)
{
  return sizeof(int64_t)+d.size();
}

void SerializedMessage::des(string &d)
{
  size_t s;
  des((int64_t *)&s);
  if (s)
  {
    d.assign((const char *)&data->at(pos), s);
    pos += s;
  }
}

void SerializedMessage::ser(vector<int64_t> &d)
{
  size_t s=d.size();
  memcpy(&data->at(pos), &s, sizeof(s));
  pos += sizeof(s);
  for (size_t n=0; n<s; n++)
  {
    ser(d[n]);
  }
}

size_t SerializedMessage::sersize(vector<int64_t> &d)
{
  return sizeof(size_t) + (d.size() * sizeof(int64_t));
}

void SerializedMessage::des(vector<int64_t> &d)
{
  size_t s;
  memcpy(&s, &data->at(pos), sizeof(s));
  pos += sizeof(s);
  d.reserve(s);
  for (size_t n=0; n<s; n++)
  {
    int64_t val;
    des(&val);
    d.push_back(val);
  }
}

void SerializedMessage::ser(boost::unordered_map<int64_t, int64_t> &d)
{
  ser((int64_t)d.size());
  boost::unordered_map<int64_t, int64_t>::const_iterator it;
  for (it=d.begin(); it != d.end(); it++)
  {
    ser(it->first);
    ser(it->second);
  }
}

size_t SerializedMessage::sersize(boost::unordered_map<int64_t, int64_t> &d)
{
  return d.size() + (d.size() * 2 * sizeof(int64_t));
}

void SerializedMessage::des(boost::unordered_map<int64_t, int64_t> &d)
{
  size_t s;
  memcpy(&s, &data->at(pos), sizeof(s));
  pos += sizeof(s);
  for (size_t n=0; n<s; n++)
  {
    int64_t val1, val2;
    des(&val1);
    des(&val2);
    d[val1]=val2;
  }
}

// pod structs
void SerializedMessage::ser(Message::message_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(Message::message_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(Message::message_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}


void SerializedMessage::ser(MessageSocket::socket_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(MessageSocket::socket_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(MessageSocket::socket_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

void SerializedMessage::ser(MessageUserSchema::userschema_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(MessageUserSchema::userschema_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(MessageUserSchema::userschema_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

void SerializedMessage::ser(procedures_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(procedures_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(procedures_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

void SerializedMessage::ser(MessageDeadlock::deadlock_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(MessageDeadlock::deadlock_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(MessageDeadlock::deadlock_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

void SerializedMessage::ser(MessageTransaction::transaction_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(MessageTransaction::transaction_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(MessageTransaction::transaction_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

void SerializedMessage::ser(MessageSubtransactionCmd::subtransaction_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(MessageSubtransactionCmd::subtransaction_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(MessageSubtransactionCmd::subtransaction_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

void SerializedMessage::ser(nonLockingIndexEntry_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(nonLockingIndexEntry_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(nonLockingIndexEntry_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

void SerializedMessage::ser(MessageDispatch::dispatch_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(MessageDispatch::dispatch_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(MessageDispatch::dispatch_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

void SerializedMessage::ser(MessageAckDispatch::ackdispatch_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(MessageAckDispatch::ackdispatch_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(MessageAckDispatch::ackdispatch_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

void SerializedMessage::ser(MessageApply::apply_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(MessageApply::apply_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(MessageApply::apply_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

void SerializedMessage::ser(MessageAckApply::ackapply_s &d)
{
  memcpy(&data->at(pos), &d, sizeof(d));
  pos += sizeof(d);
}

size_t SerializedMessage::sersize(MessageAckApply::ackapply_s &d)
{
  return sizeof(d);
}

void SerializedMessage::des(MessageAckApply::ackapply_s &d)
{
  memcpy(&d, &data->at(pos), sizeof(d));
  pos += sizeof(d);
}

// level 1
void SerializedMessage::ser(boost::unordered_set<string> &d)
{
  size_t s=d.size();
  ser((int64_t)s);
  boost::unordered_set<string>::const_iterator it;
  for (it=d.begin(); it != d.end(); it++)
  {
    ser(*it);
  }
}

size_t SerializedMessage::sersize(boost::unordered_set<string> &d)
{
  size_t retval=sizeof(size_t);
  boost::unordered_set<string>::const_iterator it;
  for (it=d.begin(); it != d.end(); it++)
  {
    retval += sersize(*it);
  }
  return retval;
}

void SerializedMessage::des(boost::unordered_set<string> &d)
{
  size_t s;
  des((int64_t *)&s);
  for (size_t n=0; n<s; n++)
  {
    string val;
    des(val);
    d.insert(val);
  }
}

void SerializedMessage::ser(fieldValue_s &d)
{
  memcpy(&data->at(pos), &d.value, sizeof(d.value));
  pos += sizeof(d.value);
  ser(d.str);
  ser((int8_t)d.isnull);
}

size_t SerializedMessage::sersize(fieldValue_s &d)
{
  return sizeof(d.value)+sersize(d.str)+sersize((int8_t)d.isnull);
}

void SerializedMessage::des(fieldValue_s &d)
{
  memcpy(&d.value, &data->at(pos), sizeof(d.value));
  pos += sizeof(d.value);
  des(d.str);
  des((int8_t *)&d.isnull);
}

void SerializedMessage::ser(returnRow_s &d)
{
  ser(d.rowid);
  ser(d.previoussubtransactionid);
  ser((int8_t)d.locktype);
  ser(d.row);
}

size_t SerializedMessage::sersize(returnRow_s &d)
{
  return sersize(d.rowid)+sersize(d.previoussubtransactionid)+
          sersize((int8_t)d.locktype)+sersize(d.row);
}

void SerializedMessage::des(returnRow_s &d)
{
  des(&d.rowid);
  des(&d.previoussubtransactionid);
  des((int8_t *)&d.locktype);
  des(d.row);
}

void SerializedMessage::ser(MessageDispatch::record_s &d)
{
  ser(d.rowid);
  ser((int8_t)d.primitive);
  ser(d.tableid);
  ser(d.previoussubtransactionid);
  ser(d.row);
  ser(d.oldrow);
}

size_t SerializedMessage::sersize(MessageDispatch::record_s &d)
{
  return sersize(d.rowid)+sersize((int8_t)d.primitive)+sersize(d.tableid)+
          sersize(d.previoussubtransactionid)+sersize(d.row)+sersize(d.oldrow);
}

void SerializedMessage::des(MessageDispatch::record_s &d)
{
  des(&d.rowid);
  des((int8_t *)&d.primitive);
  des(&d.tableid);
  des(&d.previoussubtransactionid);
  des(d.row);
  des(d.oldrow);
}

void SerializedMessage::ser(vector<nonLockingIndexEntry_s> &d)
{
  ser((int64_t)d.size());
  vector<nonLockingIndexEntry_s>::iterator it;
  for (it=d.begin(); it != d.end(); it++)
  {
    ser(*it);
  }
}

size_t SerializedMessage::sersize(vector<nonLockingIndexEntry_s> &d)
{
  size_t s=d.size();
  return sizeof(int64_t) + (s *sizeof(nonLockingIndexEntry_s));
}

void SerializedMessage::des(vector<nonLockingIndexEntry_s> &d)
{
  size_t s;
  des((int64_t *)&s);
  d.reserve(s);
  for (size_t n=0; n<s; n++)
  {
    nonLockingIndexEntry_s val;
    des(val);
    d.push_back(val);
  }
}

// level 2
void SerializedMessage::ser(newDeadLockLists_s &d)
{
  ser(d.locked);
  ser(d.waiting);
}

size_t SerializedMessage::sersize(newDeadLockLists_s &d)
{
  return sersize(d.locked)+sersize(d.waiting);
}

void SerializedMessage::des(newDeadLockLists_s &d)
{
  des(d.locked);
  des(d.waiting);
}

void SerializedMessage::ser(vector<fieldValue_s> &d)
{
  ser((int64_t)d.size());
  vector<fieldValue_s>::iterator it;
  for (it=d.begin(); it != d.end(); it++)
  {
    ser(*it);
  }
}

size_t SerializedMessage::sersize(vector<fieldValue_s> &d)
{
  size_t retval=sizeof(int64_t);
  vector<fieldValue_s>::iterator it;
  for (it = d.begin(); it != d.end(); it++)
  {
    retval += sersize(*it);
  }
  return retval;
}

void SerializedMessage::des(vector<fieldValue_s> &d)
{
  size_t s;
  des((int64_t *)&s);
  d.reserve(s);
  for (size_t n=0; n<s; n++)
  {
    fieldValue_s val;
    des(val);
    d.push_back(val);
  }
}

void SerializedMessage::ser(vector<returnRow_s> &d)
{
  ser((int64_t)d.size());
  vector<returnRow_s>::iterator it;
  for (it=d.begin(); it != d.end(); it++)
  {
    ser(*it);
  }
}

size_t SerializedMessage::sersize(vector<returnRow_s> &d)
{
  size_t retval=sizeof(int64_t);
  vector<returnRow_s>::iterator it;
  for (it = d.begin(); it != d.end(); it++)
  {
    retval += sersize(*it);
  }
  return retval;
}

void SerializedMessage::des(vector<returnRow_s> &d)
{
  size_t s;
  des((int64_t *)&s);
  d.reserve(s);
  for (size_t n=0; n<s; n++)
  {
    returnRow_s val;
    des(val);
    d.push_back(val);
  }
}

void SerializedMessage::ser(vector<MessageDispatch::record_s> &d)
{
  ser((int64_t)d.size());
  vector<MessageDispatch::record_s>::iterator it;
  for (it=d.begin(); it != d.end(); it++)
  {
    ser(*it);
  }
}

size_t SerializedMessage::sersize(vector<MessageDispatch::record_s> &d)
{
  size_t retval=sizeof(int64_t);
  vector<MessageDispatch::record_s>::iterator it;
  for (it = d.begin(); it != d.end(); it++)
  {
    retval += sersize(*it);
  }
  return retval;
}

void SerializedMessage::des(vector<MessageDispatch::record_s> &d)
{
  size_t s;
  des((int64_t *)&s);
  d.reserve(s);
  for (size_t n=0; n<s; n++)
  {
    MessageDispatch::record_s val;
    des(val);
    d.push_back(val);
  }
}

void SerializedMessage::ser(rowOrField_s &d)
{
  ser((int8_t)d.isrow);
  ser(d.tableid);
  ser(d.rowid);
  ser(d.fieldid);
  ser(d.engineid);
  ser((int8_t)d.deleteindexentry);
  ser((int8_t)d.isnotaddunique);
  ser((int8_t)d.isreplace);
  ser(d.newrowid);
  ser(d.newengineid);
  ser(d.fieldVal);
}

size_t SerializedMessage::sersize(rowOrField_s &d)
{
  return sersize((int8_t)d.isrow)+sersize(d.tableid)+sersize(d.rowid)+
          sersize(d.fieldid)+sersize(d.engineid)+sersize((int8_t)d.deleteindexentry)+
          sersize((int8_t)d.isnotaddunique)+sersize((int8_t)d.isreplace)+sersize(d.newrowid)+
          sersize(d.newengineid)+sersize(d.fieldVal);
}

void SerializedMessage::des(rowOrField_s &d)
{
  des((int8_t *)&d.isrow);
  des(&d.tableid);
  des(&d.rowid);
  des(&d.fieldid);
  des(&d.engineid);
  des((int8_t *)&d.deleteindexentry);
  des((int8_t *)&d.isnotaddunique);
  des((int8_t *)&d.isreplace);
  des(&d.newrowid);
  des(&d.newengineid);
  des(d.fieldVal);
}

void SerializedMessage::ser(MessageApply::applyindex_s &d)
{
  ser(d.fieldVal);
  ser(d.entry);
  data[pos++]=d.flags;
  ser(d.tableid);
  ser(d.fieldid);
}

size_t SerializedMessage::sersize(MessageApply::applyindex_s &d)
{
  return sersize(d.fieldVal)+sersize(d.entry)+1+sersize(d.tableid)+
          sersize(d.fieldid);
}

void SerializedMessage::des(MessageApply::applyindex_s &d)
{
  des(d.fieldVal);
  des(d.entry);
  d.flags=data->at(pos++);
  des(&d.tableid);
  des(&d.fieldid);
}

// level 3
void SerializedMessage::ser(searchParams_s &d)
{
  ser((int8_t)d.op);
  ser(d.values);
  ser(d.regexString);
}

size_t SerializedMessage::sersize(searchParams_s &d)
{
  return sersize((int8_t)d.op)+sersize(d.values)+sersize(d.regexString);
}

void SerializedMessage::des(searchParams_s &d)
{
  des((int8_t *)&d.op);
  des(d.values);
  des(d.regexString);
}

void SerializedMessage::ser(vector<rowOrField_s> &d)
{
  ser((int64_t)d.size());
  vector<rowOrField_s>::iterator it;
  for (it=d.begin(); it != d.end(); it++)
  {
    ser(*it);
  }
}

size_t SerializedMessage::sersize(vector<rowOrField_s> &d)
{
  size_t retval=sizeof(int64_t);
  vector<rowOrField_s>::iterator it;
  for (it = d.begin(); it != d.end(); it++)
  {
    retval += sersize(*it);
  }
  return retval;
}

void SerializedMessage::des(vector<rowOrField_s> &d)
{
  size_t s;
  des((int64_t *)&s);
  d.reserve(s);
  for (size_t n=0; n<s; n++)
  {
    rowOrField_s val;
    des(val);
    d.push_back(val);
  }
}

void SerializedMessage::ser(vector<MessageApply::applyindex_s> &d)
{
  ser((int64_t)d.size());
  vector<MessageApply::applyindex_s>::iterator it;
  for (it=d.begin(); it != d.end(); it++)
  {
    ser(*it);
  }
}

size_t SerializedMessage::sersize(vector<MessageApply::applyindex_s> &d)
{
  size_t retval=sizeof(int64_t);
  vector<MessageApply::applyindex_s>::iterator it;
  for (it = d.begin(); it != d.end(); it++)
  {
    retval += sersize(*it);
  }
  return retval;
}

void SerializedMessage::des(vector<MessageApply::applyindex_s> &d)
{
  size_t s;
  des((int64_t *)&s);
  d.reserve(s);
  for (size_t n=0; n<s; n++)
  {
    MessageApply::applyindex_s val;
    des(val);
    d.push_back(val);
  }
}

void SerializedMessage::ser(boost::unordered_map< int64_t, vector<MessageDispatch::record_s> > &d)
{
  ser((int64_t)d.size());
  boost::unordered_map< int64_t, vector<MessageDispatch::record_s> >::iterator it;
  for (it=d.begin(); it != d.end(); it++)
  {
    ser(it->first);
    ser(it->second);
  }
}

size_t SerializedMessage::sersize(boost::unordered_map< int64_t, vector<MessageDispatch::record_s> > &d)
{
  return d.size() + (d.size() * 2 * sizeof(int64_t));
  size_t retval=sizeof(int64_t) + (d.size()*sizeof(int64_t));
  boost::unordered_map< int64_t, vector<MessageDispatch::record_s> >::iterator it;
  for (it = d.begin(); it != d.end(); it++)
  {
    retval +=sersize(it->second);
  }
}

void SerializedMessage::des(boost::unordered_map< int64_t, vector<MessageDispatch::record_s> > &d)
{
  size_t s;
  memcpy(&s, &data->at(pos), sizeof(s));
  pos += sizeof(s);
  for (size_t n=0; n<s; n++)
  {
    int64_t val1;
    des(&val1);
    vector<MessageDispatch::record_s> val2;
    des(val2);
    d[val1]=val2;
  }
}

MessageSerialized::MessageSerialized(string *dataarg)
{
  data = dataarg;
  memcpy(&messageStruct, data->c_str(), sizeof(messageStruct));
  messageStruct.topic=TOPIC_SERIALIZED;
  messageStruct.payloadtype=PAYLOADSERIALIZED;
}

MessageSerialized::~MessageSerialized()
{
}

MessageBatchSerialized::MessageBatchSerialized(int16_t nodeidarg) : nmsgs(0)
{
  messageStruct.destAddr.nodeid=nodeidarg;
  messageStruct.topic=TOPIC_BATCHSERIALIZED;
  messageStruct.payloadtype=PAYLOADBATCHSERIALIZED;
}

MessageBatchSerialized::~MessageBatchSerialized()
{
}
