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

#include "infinisql_Pg.h"
#include "pgoids.h"
#include "infinisql_TransactionAgent.h"
#include "infinisql_ConnectionHandler.h"
#line 31 "Pg.cc"

/* implemented based on http://www.postgresql.org/docs/9.2/static/protocol.html */

Pg::Pg(class TransactionAgent *taPtrarg, int sockfdarg,
    int64_t connectionhandlerinstancearg) : state(STATE_BEGIN),
    sockfd(sockfdarg), pgcmdtype('\0'), size(0),
    outcmd('\0'), userid(-1), schemaPtr(NULL),
    session_isautocommit(true), isintransactionblock(false),
    connectionhandlerinstance (connectionhandlerinstancearg)
{
  domainid=-1;
  taPtr = taPtrarg;
  statementPtr=NULL;
  transactionPtr=NULL;

  taPtr->Pgs[sockfd]=this;

  cont();
}

Pg::~Pg()
{
  taPtr->Pgs.erase(sockfd);
}

// processing logic for Pg, state machine
void Pg::cont()
{
  if (state==STATE_EXITING)
  {
    return;
  }

  class MessageSocket &msgrcvref = *((class MessageSocket *)taPtr->msgrcv);

  string newdata;

  if ((msgrcvref.events & EPOLLERR) || (msgrcvref.events & EPOLLHUP))
  {
    closesocket(*taPtr);
    return;
  }

  if (msgrcvref.events & EPOLLIN)
  {
    // read stuff from the socket
    if (readsocket(newdata)==false)
    {
      closesocket(*taPtr);
      return;
    }

    if (!newdata.size())
    {
      // must've been spurious event from epoll
      return;
    }
  }
  else if (msgrcvref.events & EPOLLOUT)
  {
    if (rewritesocket()==-1)
    {
      closesocket(*taPtr);
    }

    return;
  }
  else
  {
    printf("%s %i anomaly events %u\n", __FILE__, __LINE__, msgrcvref.events);
    return;
  }

  short retval = initcmd(newdata);

  switch (retval)
  {
    case -1: // bogus input
      closesocket(*taPtr);
      return;
      break;

    case 0: // command not completely received
      return;
      break;

    case 1: // command completely received
      pos = 0;
      outbuf.clear();
      break;

    default:
      printf("%s %i anomaly retval %i\n", __FILE__, __LINE__, retval);
  }

  // now, type & size are set.

  switch (state)
  {
    case STATE_BEGIN:
    {
      if (size==8) // ssl request
      {
        int32_t sslreq;

        if (get(&sslreq)==false)
        {
          closesocket(*taPtr);
          return;
        }

        if (sslreq != 80877103)
        {
          closesocket(*taPtr);
          return;
        }

        outbuf.assign("N");

        if (writesocket()==-1)
        {
          closesocket(*taPtr);
        }

        size = 0;
        return;
      }
      else // normal start packet
      {
        int32_t protvers;

        if (get(&protvers)==false)
        {
          closesocket(*taPtr);
          return;
        }

        if (protvers != 196608)
        {
          closesocket(*taPtr);
          return;
        }

        while (inbuf[pos] != '\0')
        {
          string paramstr;

          if (get(paramstr)==false)
          {
            closesocket(*taPtr);
            return;
          }

          string valstr;

          if (get(valstr)==false)
          {
            closesocket(*taPtr);
            return;
          }

          startupArgs[paramstr] = valstr;
        }

        if (!startupArgs.count("user"))
        {
          closesocket(*taPtr);
          return;
        }

        if (!startupArgs.count("database"))
        {
          startupArgs["database"] = startupArgs["user"];
        }

        outcmd = 'R';
        put((int32_t)3);
        replymsg();

        if (writesocket()==-1)
        {
          closesocket(*taPtr);
          return;
        }

        state = STATE_AUTH;
        size = 0;
        inbuf.clear();
      }
    }
    break;

    case STATE_AUTH:
    {
      if (pgcmdtype != 'p')
      {
        closesocket(*taPtr);
        return;
      }

      string password;

      if (get(password)==false)
      {
        closesocket(*taPtr);
        return;
      }

      // send credentials to USM
      class Operation &operationRef = *(new class Operation(OP_PGLOGIN, taPtr,
                                            -1, -1));
      operationRef.sockfd = sockfd;

      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_OPERATION);
      msg->caller = 1;
      msg->callerstate = 1;
      msg->operationid = operationRef.operationid;
      msg->operationtype = OPERATION_LOGIN;

      msg->username = startupArgs["user"];
      msg->domainname = startupArgs["database"];
      msg->password = password;
      taPtr->mboxes.toUserSchemaMgr(taPtr->myIdentity.address, *msg);
    }
    break;

    case STATE_ESTABLISHED:
  {
      switch (pgcmdtype)
      {
        case 'Q':
        {
          string query;
          get(query);
          inbuf.clear();
          size=0;
          executeStatement(query);
        }
        break;

        case 'X':
          closesocket(*taPtr);
          break;

        default:
          printf("%s %i don't know how to handle cmdtype %c\n", __FILE__,
                 __LINE__, pgcmdtype);
      }
    }
    break;

    case STATE_ABORTED:
    {
      switch (pgcmdtype)
      {
        case 'Q':
        {
          string query;
          get(query);
          inbuf.clear();
          size=0;
          // executeStatement(query);

          class Larxer lx((char *)query.c_str(), taPtr, schemaPtr);

          if (lx.statementPtr==NULL)
          {
            putErrorResponse("ERROR", "42601", "syntax error, aborted");
            return;
          }

          statementPtr=lx.statementPtr;

          if (statementPtr->queries[0].type==CMD_COMMIT ||
              statementPtr->queries[0].type==CMD_ROLLBACK)
          {
            putCommandComplete("ROLLBACK");
            state=STATE_ESTABLISHED;
            isintransactionblock=false;

            if (writesocket()==-1)
            {
              closesocket(*taPtr);
              return;
            }
          }
          else
          {
            putErrorResponse("ERROR", "25P02", "current transaction is aborted, commands ignored until end of transaction block");
          }

          delete statementPtr;
        }
        break;

        case 'X':
          closesocket(*taPtr);
          break;

        default:
          printf("%s %i don't know how to handle cmdtype %c\n", __FILE__,
                 __LINE__, pgcmdtype);
      }
    }
    break;

    default:
      printf("%s %i anomaly state %i\n", __FILE__, __LINE__, state);
  }
}

/* false: error or EOF, true: something read (or spurious poll event) */
bool Pg::readsocket(string &buf)
{
  char in[8192];

  while (1)
  {
    ssize_t readed = recv(sockfd, in, 8192, 0);

    if (readed > 0)
    {
      buf.append(in, readed);
    }
    else if (readed == -1)
    {
      if (errno==EAGAIN || errno==EWOULDBLOCK)
      {
        return true;
      }

      return false;
    }
    else
    {
      // EOF
      return false;
    }
  }
}

void Pg::closesocket(class TransactionAgent &taRef)
{
  state=STATE_EXITING;
  taRef.Pgs.erase(sockfd);
  pgclosesocket(taRef, sockfd, connectionhandlerinstance);
  sockfd=-1;

  if (transactionPtr != NULL)
  {
    transactionPtr->reentryObject = NULL;
    transactionPtr->reentryFuncPtr = &ApiInterface::continuePgCommitimplicit;
    transactionPtr->reentryCmd = 1;
    transactionPtr->reentryState = NULL;
    transactionPtr->rollback();
    return;
  }

  if (statementPtr==NULL)
  {
    delete this;
  }
}

void Pg::pgclosesocket(class TransactionAgent &taRef, int socketfd,
        int64_t chinstance)
{
  taRef.Pgs.erase(socketfd);
  // NEW WAY
  class ConnectionHandler &chRef=*taRef.myTopology.connectionHandlers[chinstance];
  epoll_ctl(chRef.epollfd, EPOLL_CTL_DEL, socketfd, NULL);
  if (socketfd <= NUMSOCKETS)
  {
    pthread_mutex_lock(&chRef.connectionsMutex);
    chRef.socketAffinity[socketfd]=0;
    chRef.listenerTypes[socketfd]=LISTENER_NONE;
    pthread_mutex_unlock(&chRef.connectionsMutex);
  }
  else
  {
    fprintf(logfile, "%s %i fd %i > %i\n", __FILE__, __LINE__, socketfd,
            NUMSOCKETS);
  }
  close(socketfd);

  // OLD WAY
  /*
  class MessageSocket msg(socket, 0, LISTENER_PG);
  msg.topic=TOPIC_CLOSESOCKET;

  for (size_t n=0; n < taRef.mboxes.allActors[taRef.myTopology.nodeid].size();
       n++)
  {
    if (taRef.mboxes.allActors[taRef.myTopology.nodeid][n]==ACTOR_OBGATEWAY)
    {
      class MessageSocket *nmsg = new class MessageSocket;
      *nmsg=msg;
      taRef.mboxes.actoridToProducers[n]->sendMsg(*nmsg);
      break;
    }
  }
   */
}

/* -1: error, 0: not enough data, 1: enough data */
short Pg::initcmd(string &newdata)
{
  if (!size) // i don't have initial data yet
  {
    if (inbuf.size())
    {
      inbuf.append(newdata);
      newdata.swap(inbuf);
      inbuf.clear();
    }

    if (newdata.size() < (state==STATE_BEGIN ? sizeof(size) : sizeof(size)+1))
    {
      // wait for more data
      inbuf.append(newdata);
      return 0;
    }

    // got enough data initially, now set the header vars
    if (state==STATE_BEGIN)
    {
      memcpy(&size, &newdata[0], sizeof(size));
      size=be32toh(size);
      inbuf = newdata.substr(sizeof(size), string::npos);
    }
    else
    {
      pgcmdtype = newdata[0];
      memcpy(&size, &newdata[1], sizeof(size));
      size=be32toh(size);
      inbuf = newdata.substr(sizeof(size)+1, string::npos);
    }
  }
  else
  {
    inbuf.append(newdata);
  }

  //  already have initial data, see if the whole request received
  if (inbuf.size() + sizeof(size) == size)
  {
    // got whole thing
    return 1;
  }

  if (inbuf.size() + sizeof(size) < size)
  {
    // wait for more
    printf("%s %i pgcmdtype %c inbuf.size() %lu sizeof(size) %lu size %u\n", __FILE__, __LINE__, pgcmdtype, inbuf.size(), sizeof(size), size);
    return 0;
  }

  // bogus data

  printf("%s %i inbuf.size() %lu sizeof(size) %lu size %u\n", __FILE__, __LINE__, inbuf.size(), sizeof(size), size);
  return -1;
}

bool Pg::get(int16_t *val)
{
  if (pos + sizeof(int16_t) > inbuf.size())
  {
    // not enough room in inbuf to get requested data
    return false;
  }

  memcpy(val, &inbuf[pos], sizeof(int16_t));
  *val = be16toh(*val);
  pos += sizeof(int16_t);

  return true;
}

bool Pg::get(int32_t *val)
{
  if (pos + sizeof(int32_t) > inbuf.size())
  {
    // not enough room in inbuf to get requested data
    return false;
  }

  memcpy(val, &inbuf[pos], sizeof(int32_t));
  *val = be32toh(*val);
  pos += sizeof(int32_t);

  return true;
}

bool Pg::get(int64_t *val)
{
  if (pos + sizeof(int64_t) > inbuf.size())
  {
    // not enough room in inbuf to get requested data
    return false;
  }

  memcpy(val, &inbuf[pos], sizeof(int64_t));
  *val = be64toh(*val);
  pos += sizeof(int64_t);

  return true;
}

bool Pg::get(vector<int16_t> &val, size_t nelem)
{
  if (pos + sizeof(int16_t)*nelem > inbuf.size())
  {
    // not enough room in inbuf to get requested data
    return false;
  }

  for (size_t n=0; n < nelem; n++)
  {
    int16_t elem;
    memcpy(&elem, &inbuf[pos], sizeof(int16_t));
    elem = be16toh(elem);
    val.push_back(elem);
    pos += sizeof(int16_t);
  }

  return true;
}

bool Pg::get(vector<int32_t> &val, size_t nelem)
{
  if (pos + sizeof(int32_t)*nelem > inbuf.size())
  {
    // not enough room in inbuf to get requested data
    return false;
  }

  for (size_t n=0; n < nelem; n++)
  {
    int32_t elem;
    memcpy(&elem, &inbuf[pos], sizeof(int32_t));
    elem = be32toh(elem);
    val.push_back(elem);
    pos += sizeof(int32_t);
  }

  return true;
}

bool Pg::get(vector<int64_t> &val, size_t nelem)
{
  if (pos + sizeof(int64_t)*nelem > inbuf.size())
  {
    // not enough room in inbuf to get requested data
    return false;
  }

  for (size_t n=0; n < nelem; n++)
  {
    int64_t elem;
    memcpy(&elem, &inbuf[pos], sizeof(int64_t));
    elem = be64toh(elem);
    val.push_back(elem);
    pos += sizeof(int64_t);
  }

  return true;
}

// get Byte1
bool Pg::get(char *val)
{
  if (pos + sizeof(char) > inbuf.size())
  {
    // not enough room in inbuf to get requested data
    return false;
  }

  *val = inbuf[pos];
  pos += sizeof(char);

  return true;
}

// get ByteN where N>1
bool Pg::get(string &val, size_t nelem)
{
  if (pos + sizeof(char)*nelem > inbuf.size())
  {
    // not enough room in inbuf to get requested data
    return false;
  }

  val = inbuf.substr(pos, nelem);
  pos += sizeof(char)*nelem;

  return true;
}

// get string
bool Pg::get(string &val)
{
  size_t endpos = inbuf.find('\0', pos);

  if (endpos == string::npos)
  {
    return false;
  }

  val = inbuf.substr(pos, endpos-pos);
  pos = endpos+1;

  return 1;
}

void Pg::put(int16_t val)
{
  size_t curpos = outmsg.size();
  outmsg.resize(curpos + sizeof(int16_t));
  val = htobe16(val);
  memcpy(&outmsg[curpos], &val, sizeof(int16_t));
}

void Pg::put(int32_t val)
{
  size_t curpos = outmsg.size();
  outmsg.resize(curpos + sizeof(int32_t));
  val = htobe32(val);
  memcpy(&outmsg[curpos], &val, sizeof(int32_t));
}

void Pg::put(int64_t val)
{
  size_t curpos = outmsg.size();
  outmsg.resize(curpos + sizeof(int64_t));
  val = htobe64(val);
  memcpy(&outmsg[curpos], &val, sizeof(int64_t));
}

void Pg::put(vector<int16_t> &val)
{
  size_t curpos = outmsg.size();
  size_t nelem = val.size();
  outmsg.resize(curpos + sizeof(int16_t)*nelem);

  for (size_t n=0; n < nelem; n++)
  {
    int16_t elem = htobe16(val[n]);
    memcpy(&outmsg[curpos], &elem, sizeof(int16_t));
    curpos += sizeof(int16_t);
  }
}

void Pg::put(vector<int32_t> &val)
{
  size_t curpos = outmsg.size();
  size_t nelem = val.size();
  outmsg.resize(curpos + sizeof(int32_t)*nelem);

  for (size_t n=0; n < nelem; n++)
  {
    int32_t elem = htobe32(val[n]);
    memcpy(&outmsg[curpos], &elem, sizeof(int32_t));
    curpos += sizeof(int32_t);
  }
}

void Pg::put(vector<int64_t> &val)
{
  size_t curpos = outmsg.size();
  size_t nelem = val.size();
  outmsg.resize(curpos + sizeof(int64_t)*nelem);

  for (size_t n=0; n < nelem; n++)
  {
    int64_t elem = htobe64(val[n]);
    memcpy(&outmsg[curpos], &elem, sizeof(int64_t));
    curpos += sizeof(int64_t);
  }
}

// Byte1
void Pg::put(char val)
{
  size_t curpos = outmsg.size();
  outmsg.resize(curpos + sizeof(char));
  outmsg[curpos] = val;
}

// ByteN where N>1
void Pg::put(char *val, size_t nelem)
{
  outmsg.append(val, nelem);
}

void Pg::put(string &val)
{
  outmsg.append(val);
  outmsg.append(1, '\0');
}

void Pg::put(char *val)
{
  outmsg.append(val);
  outmsg.append(1, '\0');
}

/* -1: socket error, 0: send complete, 1: data remaining & backgrounded */
void Pg::replymsg()
{
  uint32_t osize = htobe32((uint32_t)outmsg.size() + sizeof(osize));
  string prependstr(sizeof(osize) + sizeof(outcmd), '\0');
  prependstr[0] = outcmd;
  memcpy(&prependstr[1], &osize, sizeof(osize));
  outmsg.insert(0, prependstr);
  outbuf.append(outmsg);

  outmsg.clear();
}

// put ReadyForQuery at the end
short Pg::writesocket()
{
  if (state==STATE_ESTABLISHED)
  {
    outcmd='Z';

    if (transactionPtr==NULL)
    {
      put('I');
    }
    else
    {
      put('T');
    }

    replymsg();
  }
  else if (state==STATE_ABORTED)
  {
    outcmd='Z';
    put('E');
    replymsg();
  }

  size_t curpos = 0;
  size_t outbuflen = outbuf.size();

  while (curpos < outbuflen)
  {
    ssize_t sent = send(sockfd, &outbuf[curpos],
                        (outbuflen-curpos) >= 8192 ? 8192 : outbuflen-curpos, 0);

    if (sent == -1)
    {
      if (errno==EAGAIN || errno==EWOULDBLOCK)
      {
        string backgroundstr = outbuf.substr(curpos, string::npos);
        outbuf.swap(backgroundstr);
        struct epoll_event epevent;
        epevent.events = EPOLLOUT | EPOLLHUP | EPOLLET;
        //        epevent.events = EPOLLOUT | EPOLLRDHUP | EPOLLHUP | EPOLLET;
        epevent.data.fd = sockfd;
        epoll_ctl(taPtr->myIdentity.epollfd, EPOLL_CTL_MOD, sockfd, &epevent);
        return 1;
      }

      // some real error, so return & close socket
      return -1;
    }

    curpos += sent;
  }

  outbuf.clear();
  return 0;
}

short Pg::rewritesocket()
{
  switch (writesocket())
  {
    case -1:
      return -1;
      break;

    case 0:
    {
      struct epoll_event epevent;
      epevent.events = EPOLLIN | EPOLLHUP | EPOLLET;
      //      epevent.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLET;
      epevent.data.fd=sockfd;
      epoll_ctl(taPtr->myIdentity.epollfd, EPOLL_CTL_MOD, sockfd, &epevent);
      return 0;
    }
    break;

    case 1:
      return 1;
      break;

    default:
      printf("%s %i anomaly WTF\n", __FILE__, __LINE__);
      return -1;
  }
}

void Pg::continueLogin(int cmdstate, class MessageUserSchema &msgrcvref)
{
  if (msgrcvref.status==STATUS_OK)
  {
    userid = msgrcvref.userid;
    domainid = msgrcvref.domainid;
    procedureprefix.assign(storedprocprefix);
    procedureprefix.append(msgrcvref.domainname);
    procedureprefix.append("_");
    schemaPtr = taPtr->domainidsToSchemata[domainid];

    putAuthenticationOk();
  }
  else // STATUS_NOTOK
  {
    putErrorResponse("FATAL", "28P01", "authenticationfailure");
  }
}

void Pg::executeStatement(string &stmtstr)
{
  class Larxer lx((char *)stmtstr.c_str(), taPtr, schemaPtr);

  if (lx.statementPtr==NULL)
  {
    putErrorResponse("ERROR", "42601", "syntax error");
    return;
  }

  statementPtr=lx.statementPtr;

  if (statementPtr->resolveTableFields()==false)
  {
    sqlrollbackimplicit();
    putErrorResponse("ERROR", "42704", "table or column does not exist");
    delete statementPtr;
    return;
  }

  if (statementPtr->queries[0].type==CMD_STOREDPROCEDURE)
  {
    statementPtr->queries[0].storedProcedure.insert(0, procedureprefix);
  }

  results = results_s();

  if (transactionPtr==NULL)
  {
    command_autocommit=true;
  }
  else
  {
    command_autocommit=false;
  }

  statementPtr->execute(this, &ApiInterface::continuePgFunc, 1, transactionPtr,
                        transactionPtr, vector<string>());
}

void Pg::continuePgFunc(int64_t entrypoint, void *statePtr)
{
  /* based on the statement type and transaction state, a variety of things
   * if session_isautocommit==true, and is SELECT, INSERT, UPDATE, DELETE,
   * then output
   * If autocommit==false, and SELECT, INSERT, UPDATE, DELETE, then prepare
   * output but don't output
   * if COMMIT (END), then commit open transaction and output results already
   * prepared
   * if ROLLBACK, then rollback open transaction and output results
   * CommandComplete at the end of everything returned
   * If set, then set whatever
   */
  transactionPtr=results.transactionPtr;

  if (state==STATE_EXITING)
  {
    if (transactionPtr==NULL)
    {
      fprintf(logfile, "%s %i deleting this %p\n", __FILE__, __LINE__, this);
      delete this;
      return;
    }
    else
    {
      sqlrollbackexplicit();
      return;
    }
  }

  switch (results.cmdtype)
  {
    case CMD_SELECT:
    {
      if (results.statementStatus==STATUS_OK)
      {
        putRowDescription();
        putDataRows();

        std::stringstream tag;
        tag << "SELECT " << results.selectResults.size();
        putCommandComplete((char *)tag.str().c_str());
      }
      else
      {
        sqlrollbackimplicit();
        errorStatus(results.statementStatus);
        return;
      }

      if (isintransactionblock==false && (session_isautocommit==true || command_autocommit==true))
      {
        sqlcommitimplicit();
      }
      else
      {
        if (writesocket()==-1)
        {
          closesocket(*taPtr);
        }
      }
    }
    break;

    case CMD_INSERT:
      if (results.statementStatus==STATUS_OK)
      {
        putCommandComplete("INSERT 0 1");
      }
      else
      {
        sqlrollbackimplicit();
        errorStatus(results.statementStatus);
        return;
      }

      if (isintransactionblock==false && (session_isautocommit==true || command_autocommit==true))
      {
        sqlcommitimplicit();
      }
      else
      {
        if (writesocket()==-1)
        {
          closesocket(*taPtr);
        }
      }

      break;

    case CMD_UPDATE:
    {
      if (results.statementStatus==STATUS_OK)
      {
        std::stringstream tag;
        tag << "UPDATE " << results.statementResults.size();
        putCommandComplete((char *)tag.str().c_str());
      }
      else
      {
        sqlrollbackimplicit();
        errorStatus(STATUS_NOTOK);
        return;
      }

      if (isintransactionblock==false && (session_isautocommit==true || command_autocommit==true))
      {
        /*
        sqlrollbackimplicit();
        errorStatus(STATUS_NOTOK);
        return;
         */
        sqlcommitimplicit();
      }
      else
      {
        if (writesocket()==-1)
        {
          closesocket(*taPtr);
        }
      }
    }
    break;

    case CMD_DELETE:
    {
      if (results.statementStatus==STATUS_OK)
      {
        std::stringstream tag;
        tag << "DELETE " << results.statementResults.size();
        putCommandComplete((char *)tag.str().c_str());
      }
      else
      {
        sqlrollbackimplicit();
        errorStatus(results.statementStatus);
        return;
      }

      if (isintransactionblock==false && (session_isautocommit==true || command_autocommit==true))
      {
        sqlcommitimplicit();
      }
      else
      {
        if (writesocket()==-1)
        {
          closesocket(*taPtr);
        }
      }
    }
    break;

    case CMD_BEGIN:
      if (transactionPtr==NULL)
      {
        transactionPtr = new class Transaction(taPtr, schemaPtr->domainid);
        isintransactionblock=true;
        putCommandComplete("BEGIN");

        if (writesocket()==-1)
        {
          closesocket(*taPtr);
        }
      }
      else
      {
        putNoticeResponse("WARNING", "25001", "there is already a transaction in progress");
        putCommandComplete("BEGIN");

        if (writesocket()==-1)
        {
          closesocket(*taPtr);
        }
      }

      break;

    case CMD_COMMIT:
      if (transactionPtr != NULL)
      {
        isintransactionblock=false;
        sqlcommitexplicit();
      }
      else
      {
        putNoticeResponse("WARNING", "25P01", "there is no transaction in progress");
        putCommandComplete("COMMIT");

        if (writesocket()==-1)
        {
          closesocket(*taPtr);
        }
      }

      break;

    case CMD_ROLLBACK:
      if (transactionPtr != NULL)
      {
        isintransactionblock=false;
        sqlrollbackexplicit();
      }
      else
      {
        putNoticeResponse("WARNING", "25P01", "there is no transaction in progress");
        putCommandComplete("ROLLBACK");

        if (writesocket()==-1)
        {
          closesocket(*taPtr);
        }
      }

      break;

    case CMD_SET:
      break;

    case CMD_STOREDPROCEDURE:
    {
      if (results.statementStatus==STATUS_OK)
      {
        putRowDescription();
        putDataRows();

        std::stringstream tag;
        tag << "SELECT " << results.selectResults.size();
        putCommandComplete((char *)tag.str().c_str());
      }
      else
      {
        sqlrollbackimplicit();
        errorStatus(results.statementStatus);
        return;
      }

      if (isintransactionblock==false && (session_isautocommit==true || command_autocommit==true))
      {
        sqlcommitimplicit();
      }
      else
      {
        if (writesocket()==-1)
        {
          closesocket(*taPtr);
        }
      }
    }
    break;

    default:
      printf("%s %i anomaly cmdtype %i statementStatus %li\n", __FILE__,
             __LINE__, results.cmdtype, results.statementStatus);
  }
}

void Pg::putCommandComplete(char *tag)
{
  outcmd='C';
  put(tag);
  replymsg();
}

void Pg::putErrorResponse(char *severity, char *code, char *message)
{
  outcmd='E';
  put('S');
  put(severity);
  put('C');
  put(code);
  put('M');
  put(message);
  put(char(0));

  replymsg();

  if (writesocket()==-1)
  {
    closesocket(*taPtr);
  }
}

void Pg::putNoticeResponse(char *severity, char *code, char *message)
{
  outcmd='N';
  put('S');
  put(severity);
  put('C');
  put(code);
  put('M');
  put(message);
  put(char(0));

  replymsg();
}

void Pg::sqlcommitimplicit()
{
  if (transactionPtr==NULL)
  {
    continuePgCommitimplicit(1, NULL);
    return;
  }

  transactionPtr->reentryObject = this;
  transactionPtr->reentryFuncPtr = &ApiInterface::continuePgCommitimplicit;
  transactionPtr->reentryCmd = 1;
  transactionPtr->reentryState = NULL;

  transactionPtr->commit();
}

void Pg::sqlcommitexplicit()
{
  if (transactionPtr==NULL)
  {
    continuePgCommitexplicit(1, NULL);
    return;
  }

  transactionPtr->reentryObject = this;
  transactionPtr->reentryFuncPtr = &ApiInterface::continuePgCommitexplicit;
  transactionPtr->reentryCmd = 1;
  transactionPtr->reentryState = NULL;

  transactionPtr->commit();
}

void Pg::sqlrollbackimplicit()
{
  if (isintransactionblock==true)
  {
    state=STATE_ABORTED;
  }

  if (transactionPtr==NULL)
  {
    continuePgRollbackimplicit(1, NULL);
    return;
  }

  transactionPtr->reentryObject = this;
  transactionPtr->reentryFuncPtr = &ApiInterface::continuePgRollbackimplicit;
  transactionPtr->reentryCmd = 1;
  transactionPtr->reentryState = NULL;

  transactionPtr->rollback();
}

void Pg::sqlrollbackexplicit()
{
  if (transactionPtr==NULL)
  {
    continuePgRollbackexplicit(1, NULL);
    return;
  }

  transactionPtr->reentryObject = this;
  transactionPtr->reentryFuncPtr = &ApiInterface::continuePgRollbackexplicit;
  transactionPtr->reentryCmd = 1;
  transactionPtr->reentryState = NULL;

  transactionPtr->rollback();
}

bool Pg::sqlbegin()
{
  if (transactionPtr != NULL)
  {
    return false;
  }

  transactionPtr = new class Transaction(taPtr, domainid);
  return true;
}

void Pg::putRowDescription()
{
  outcmd='T';
  int16_t numfields = (int16_t)results.selectFields.size();
  put(numfields);

  for (int16_t n=0; n < numfields; n++)
  {
    put(results.selectFields[n].name);
    put((int32_t)0);
    put((int16_t)0);

    switch (results.selectFields[n].type)
    {
      case INT:
        put((int32_t)INT8OID);
        put((int16_t)8);
        break;

      case UINT:
        put((int32_t)INT8OID);
        put((int16_t)8);
        break;

      case BOOL:
        put((int32_t)BOOLOID);
        put((int16_t)1);
        break;

      case FLOAT:
        put((int32_t)FLOAT8OID);
        put((int16_t)8);
        break;

      case CHAR:
        put((int32_t)CHAROID);
        put((int16_t)1);
        break;

      case CHARX:
        put((int32_t)BPCHAROID);
        put((int16_t)-2);
        break;

      case VARCHAR:
        put((int32_t)VARCHAROID);
        put((int16_t)-2);
        break;

      default:
        printf("%s %i anomaly %i\n", __FILE__, __LINE__,
               results.selectFields[n].type);
        return;
    }

    put((int32_t)0);
    put((int16_t)0); // text format code
  }

  replymsg();
}

void Pg::putDataRows()
{
  boost::unordered_map< uuRecord_s, vector<fieldValue_s> >::const_iterator it;

  for (it = results.selectResults.begin(); it != results.selectResults.end();
       it++)
  {
    outcmd='D';
    int16_t numfields = (int16_t)results.selectFields.size();
    put(numfields);

    const vector<fieldValue_s> &fieldValues = it->second;

    for (int16_t n=0; n < numfields; n++)
    {
      if (fieldValues[n].isnull==true)
      {
        put((int32_t)-1);
        continue;
      }

      switch (results.selectFields[n].type)
      {
        case INT:
        {
          char val[21]; // length of smallest int64_t + \0
          int32_t len=sprintf(val, "%li", fieldValues[n].value.integer);
          put(len);
          put(val, len);
        }
        break;

        case UINT:
        {
          char val[21]; // length of largest uint64_t + \0
          int32_t len=sprintf(val, "%lu", fieldValues[n].value.uinteger);
          put(len);
          put(val, len);
        }
        break;

        case BOOL:
          put((int32_t)1);

          if (fieldValues[n].value.boolean==true)
          {
            put('t');
          }
          else
          {
            put('f');
          }

          break;

        case FLOAT:
        {
          std::stringstream val;
          val << (double)fieldValues[n].value.floating;

          if ((double)fieldValues[n].value.floating / (int64_t)fieldValues[n].value.floating == 1)
          {
            val << ".0";
          }

          int32_t len=val.str().size();
          put((int32_t)len);
          put((char *)val.str().c_str(), len);
        }
        break;

        case CHAR:
          put((int32_t)1);
          put(fieldValues[n].value.character);
          break;

        case CHARX:
        {
          int32_t len = fieldValues[n].str.size();
          put((int32_t)len);
          put((char *)fieldValues[n].str.c_str(), len);
        }
        break;

        case VARCHAR:
        {
          int32_t len = fieldValues[n].str.size();
          put((int32_t)len);
          put((char *)fieldValues[n].str.c_str(), len);
        }
        break;

        default:
          printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                 results.selectFields[n].type);
          return;
      }
    }

    replymsg();
  }
}

void Pg::continuePgCommitimplicit(int64_t entrypoint, void *statePtr)
{
  if (transactionPtr != NULL)
  {
    delete transactionPtr;
    transactionPtr=NULL;
  }

  if (state==STATE_EXITING)
  {
    if (statementPtr==NULL)
    {
      fprintf(logfile, "%s %i deleting this %p\n", __FILE__, __LINE__, this);
      delete this;
    }

    return;
  }

  if (writesocket()==-1)
  {
    closesocket(*taPtr);
  }
}

void Pg::continuePgCommitexplicit(int64_t entrypoint, void *statePtr)
{
  putCommandComplete("COMMIT");

  if (transactionPtr != NULL)
  {
    delete transactionPtr;
    transactionPtr=NULL;
  }

  if (state==STATE_EXITING)
  {
    if (statementPtr==NULL)
    {
      fprintf(logfile, "%s %i deleting this %p\n", __FILE__, __LINE__, this);
      delete this;
    }

    return;
  }

  if (writesocket()==-1)
  {
    closesocket(*taPtr);
  }

  outbuf.clear();
}

void Pg::continuePgRollbackimplicit(int64_t entrypoint, void *statePtr)
{
  outbuf.clear();

  if (transactionPtr != NULL)
  {
    delete transactionPtr;
    transactionPtr=NULL;
  }

  if (state==STATE_EXITING)
  {
    if (statementPtr==NULL)
    {
      fprintf(logfile, "%s %i deleting this %p\n", __FILE__, __LINE__, this);
      delete this;
    }

    return;
  }
}

void Pg::continuePgRollbackexplicit(int64_t entrypoint, void *statePtr)
{
  if (transactionPtr != NULL)
  {
    delete transactionPtr;
    transactionPtr=NULL;
  }

  if (state==STATE_EXITING)
  {
    if (statementPtr==NULL)
    {
      fprintf(logfile, "%s %i deleting this %p\n", __FILE__, __LINE__, this);
      delete this;
    }

    return;
  }

  putCommandComplete("ROLLBACK");

  if (writesocket()==-1)
  {
    closesocket(*taPtr);
  }

  outbuf.clear();
}

/* pg 9.2.4 response to perl client ParameterStatus list:
 * application_name
client_encoding UTF8
DateStyle ISO, MDY
integer_datetimes on
IntervalStyle postgres
is_superuser  off
server_encoding UTF8
server_version  9.2.4
session_authorization mtravis
standard_conforming_strings on
TimeZone  US/Pacific
 */
void Pg::putAuthenticationOk()
{
  // AuthenticationOk
  outcmd = 'R';
  put((int32_t)0);
  replymsg();

  putParameterStatus("application_name", "");
  putParameterStatus("client_encoding", "LATIN1");
  putParameterStatus("DateStyle", "ISO, MDY");
  putParameterStatus("integer_datetimes", "on");
  putParameterStatus("IntervalStyle", "postgres");
  putParameterStatus("is_superuser", "off");
  putParameterStatus("server_encoding", "LATIN1");
  putParameterStatus("server_version", "9.2.4");
  putParameterStatus("session_authorization",
                     (char *)startupArgs["username"].c_str());
  putParameterStatus("standard_conforming_strings", "on");
  tzset();
  // long TimeZone name from tzname[2] having problems compiling
  putParameterStatus("TimeZone", tzname[1]);
  //    putParameterStatus("TimeZone", "US/Pacific");

  // forget key data for now, but put something here
  outcmd = 'K';
  put(int32_t(1));
  put(int32_t(2));
  replymsg();

  // ReadyForQuery
  outcmd = 'Z';
  put((char)'I');
  replymsg();

  if (writesocket()==-1)
  {
    closesocket(*taPtr);
    return;
  }

  state = STATE_ESTABLISHED;
  size = 0;
  inbuf.clear();
}

void Pg::putParameterStatus(char *name, char *value)
{
  outcmd='S';
  put(name);
  put(value);
  replymsg();
}

void Pg::errorStatus(int64_t status)
{
  switch (status)
  {
    case APISTATUS_NOTOK:
      putErrorResponse("ERROR", "42000",
                       "generic InfiniSQL error");
      break;

    case APISTATUS_UNIQUECONSTRAINT:
      putErrorResponse("ERROR", "23000", "integrity_constraint_violation");
      break;

    case APISTATUS_LOCK:
      putErrorResponse("ERROR", "55P03", "lock not available");
      break;

    default:
      printf("%s %i anomaly %li\n", __FILE__, __LINE__, status);
      putErrorResponse("ERROR", "42000",
                       "syntax_error_or_access_rule_violation");
  }
}
