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

#include "infinisql_TransactionAgent.h"
#include "infinisql_Pg.h"
#include "infinisql_defs.h"
#include "infinisql_log.h"
#line 29 "TransactionAgent.cc"

TransactionAgent::TransactionAgent(Topology::partitionAddress *myIdentityArg) :		// JLH: constructor that takes 1 argument; assign values via init list
  myIdentity(*myIdentityArg), nexttransactionid(0), nextapplierid(0),
  myreplica(-1), mymember(-1)
{
  delete myIdentityArg;				// JLH: prevent memory leak
  myIdentityArg = NULL;				// JLH: http://en.wikipedia.org/wiki/Delete_(C%2B%2B)
  epollfd = myIdentity.epollfd;
  instance = myIdentity.instance;
  mboxes.nodeid = myIdentity.address.nodeid;

  builtincmds_e cmd = NOCMD;
  spclasscreate spC;
  spclassdestroy spD;
  uint32_t events = NO_EVENTS;

#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Calling tryMapBuiltinCmdsToMethods(): trying to map builtin commands to methods.");
#endif
  tryMapBuiltinCmdsToMethods();
  operationid=0;				// JLH: how is operationid used?
  class Mbox &mymbox = *myIdentity.mbox;
  int waitTime = ONE_HUNDRED_MICROSECONDS;	// JLH: timeout in microseconds (*NOT MILLISECONDS*)

  while (true)					// JLH: main event loop
  {
    // clear data from msgrcv
    domainid=INVALID;
    userid=INVALID;
    argsize=INVALID;
    sockfd=INVALID;

    msgrcv = mymbox.receive(waitTime);

    if (msgrcv==NULL)
    {
      waitTime = ONE_HUNDRED_MICROSECONDS;
      //break;
      continue;
    }

    waitTime = ZERO_MICROSECONDS;

    if (msgrcv->payloadtype==PAYLOADUSERSCHEMA)
    {
      MessageUserSchema &msgref = *((MessageUserSchema *)msgrcv);	// JLH: removed 'class' keyword

      if (msgrcv->topic != TOPIC_SCHEMAREQUEST)			// JLH: set up sockfd, userid & domainid if applicable
      {
        // don't want to validate somebody else's operationid or override
        pendingOperationsIterator = pendingOperations.find(msgref.operationid);

        if (pendingOperationsIterator != pendingOperations.end())
        {
          sockfd = pendingOperations[msgref.operationid]->sockfd;
          userid = pendingOperations[msgref.operationid]->userid;
          domainid = pendingOperations[msgref.operationid]->domainid;
        }
      }

      operationid = msgref.operationid;
    }

    switch (msgrcv->topic)
    {
      case TOPIC_SOCKET:
      {
        switch (((MessageSocket *)msgrcv)->listenertype)	// JLH: removed 'class' keyword
        {
          // LISTENER_RAW is the interface that things like login, createtable & other builtins use.
          case LISTENER_RAW:					// JLH: interface that login, createtable & other builtins use
          {
            sockfd = ((MessageSocket *)msgrcv)->socket;		// JLH: removed 'class' keyword
            events = ((MessageSocket *)msgrcv)->events;		// JLH: removed 'class' keyword

            if ((events & EPOLLERR) || (events & EPOLLHUP))
            {
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Calling endConnection(): Attempting to close connection.");
#endif
              endConnection();
              break;
            }

            if (events & EPOLLIN)
            {
              operation = (string *)new string;
#ifndef NDEBUG
  //fprintf(logfile, "DEBUG:\t%s %i Calling readSocket(): Attempting to read data from socket.\n", __FILE__, __LINE__);
  logDebugMessage(__FILE__, __LINE__, "Calling readSocket(): Attempting to read data from socket.");
#endif
              argsize = readSocket();		// JLH: added DEBUG info to readSocket()

              if (argsize < 0)
              {
                delete operation;		// JLH: destructor call
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Calling endConnection(): Attempting to close connection.");
#endif
                endConnection();		// JLH: add DEBUG call here and to endConnection()
                break;
              }

              // ok, if no user logged in, then can only login, else
              // break connection exit
              // if user logged in, then cannot login but everything else
              // login shouldn't be in binFunctions map therefore
              socketAuthInfo::iterator loggedInUsersIterator;		// JLH: where does socketAuthInfo come from?
              loggedInUsersIterator = loggedInUsers.find(sockfd);

              if (loggedInUsersIterator == loggedInUsers.end())
              {
                // this means not logged in
                if (operation->compare("login")==NO_DIFFERENCES)
                {
                  // so, login
                  login(STARTCMD);	
                }
                else if (operation->compare("ping")==NO_DIFFERENCES && __sync_add_and_fetch(&cfgs.anonymousping, 0))
                {
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Calling ping(STARTCMD): Ping request received.");
#endif
                  ping(STARTCMD);					// JLH: add DEBUG info here & to ping()
                }
                  else     // JLH: command is not login or ping and no other command is valid, so close connection
                {
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Calling endConnection(): Attempting to close connection.");
#endif
                  endConnection();					// JLH: add DEBUG info here & to endConnection()
                  break;
                }
              }
                else	// JLH: user is logged in
              {
                // get my domainid & userid
                domainid = loggedInUsersIterator->second.domainid;	// JLH: how are domainid, userid & domainName used?
                userid = loggedInUsersIterator->second.userid;	// JLH: a schemata is comprised of tables & indices (a schema)
                domainName = loggedInUsersIterator->second.domainName;// JLH: each domain has a single schema associated
									// JLH: Thus, each stored proc is associated with a single domain
                // first, check domain operations, when those are built
                if (domainidsToProcedures.count(domainid))		// JLH: domainidsToProcedures is an unordered_map
                {
                  if (domainidsToProcedures[domainid].count(*operation))
                  {
                    spC = (spclasscreate)domainidsToProcedures[domainid]
                          [*operation].procedurecreator;
                    spD = (spclassdestroy)domainidsToProcedures
                          [domainid][*operation].proceduredestroyer;
                    spC(this, NULL, (void *)spD);			// JLH: revisit this
                    continue;
                    break;
                  }
                }

                builtinsMap::iterator builtinsIterator;
                //builtinsIterator = builtins.find(*operation);		// JLH: commented this out to try wrapping
                builtinsIterator = tryFindOperations(*operation);	// JLH: figure this out later; needs a wrap

                if (builtinsIterator != builtins.end())
                {
                  (this->*(builtinsIterator->second))(STARTCMD);
                }
                else
                {
                  // terminate with extreme prejudice
                  endConnection();					// JLH: add DEBUG info here
                }
              }

              delete operation;
            }

            if (events & EPOLLOUT)
            {
              struct epoll_event ev;
              ev.events = EPOLLIN | EPOLLHUP | EPOLLET;
              ev.data.fd = sockfd;

              if (epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev))
              {
                endConnection();		// JLH: add DEBUG info here
                break;
              }

              // write data that's waiting
              sendLaterMap::iterator waitingToSendIterator;
              waitingToSendIterator = waitingToSend.find(sockfd);

              if (waitingToSendIterator != waitingToSend.end())
              {
                responseData response = waitingToSendIterator->second;
                sendResponse(true, response.resultCode, response.sbuf);
              }
            }
          }
          break;

          // JLH: LISTENER_PG is the Postgres wire protocol interface
          case LISTENER_PG:
          {
            MessageSocket &msgrcvref = *(MessageSocket *)msgrcv;			// JLH: removed 'class' keyword

            if (!Pgs.count(msgrcvref.socket))
            {
              if ((msgrcvref.events & EPOLLERR) || (msgrcvref.events & EPOLLHUP))
              {
                fprintf(logfile, "\t%s %i hanging it up\n", __FILE__, __LINE__);
                Pg::pgclosesocket(*this, msgrcvref.socket);
                break;
              }

              new class Pg(this, msgrcvref.socket);
            }
            else
            {
              Pgs[msgrcvref.socket]->cont();
            }
          }
          break;

          default:
            printf("%s %i anomaly listenertype %i\n", __FILE__, __LINE__,
                   ((MessageSocket *)msgrcv)->listenertype);				// JLH: removed 'class' keyword
        }
      }
      break;

      case TOPIC_LOGINOK:
        // set data members based on msgrcv
        login(OKCMD);
        break;

      case TOPIC_LOGINFAIL:
        // set data members based on msgrcv
        login(NOTOKCMD);
        break;

      case TOPIC_CHANGEPASSWORDOK:
        // set data members based on msgrcv
        changepassword(OKCMD);
        break;

      case TOPIC_CHANGEPASSWORDFAIL:
        // set data members based on msgrcv
        changepassword(NOTOKCMD);
        break;

      case TOPIC_CREATEDOMAINOK:
        // set data members based on msgrcv
        createdomain(OKCMD);
        break;

      case TOPIC_CREATEDOMAINFAIL:
        // set data members based on msgrcv
        createdomain(NOTOKCMD);
        break;

      case TOPIC_CREATEUSEROK:
        // set data members based on msgrcv
        createuser(OKCMD);
        break;

      case TOPIC_CREATEUSERFAIL:
        // set data members based on msgrcv
        createuser(NOTOKCMD);
        break;

      case TOPIC_DELETEUSEROK:
        // set data members based on msgrcv
        deleteuser(OKCMD);
        break;

      case TOPIC_DELETEUSERFAIL:
        // set data members based on msgrcv
        deleteuser(NOTOKCMD);
        break;

      case TOPIC_DELETEDOMAINOK:
        // set data members based on msgrcv
        deletedomain(OKCMD);
        break;

      case TOPIC_DELETEDOMAINFAIL:
        // set data members based on msgrcv
        deletedomain(NOTOKCMD);
        break;

      /* schema */
      case TOPIC_SCHEMAREPLY:
        pendingOperationsIterator = pendingOperations.find(operationid);

        if (pendingOperationsIterator == pendingOperations.end())
        {
          fprintf(logfile, "bad operationid %li %s %i\n", operationid, __FILE__, __LINE__);
          break;
        }

        operationPtr = pendingOperationsIterator->second;

        switch (operationPtr->schemaData.state)
        {
          case usm:
            cmd = USMRESPONSECMD;
            break;

          case tasengines:
            cmd = TASENGINESRESPONSECMD;
        }

        switch (operationPtr->schemaData.builtincmd)
        {
          case BUILTINCREATESCHEMA:
            createschema(cmd);
            break;

          case BUILTINCREATETABLE:
            createtable(cmd);
            break;

          case BUILTINADDCOLUMN:
            addcolumn(cmd);
            break;

          case BUILTINDELETEINDEX:
            deleteindex(cmd);
            break;

          case BUILTINDELETETABLE:
            deletetable(cmd);
            break;

          case BUILTINDELETESCHEMA:
            deleteschema(cmd);
            break;

          default:
            fprintf(logfile, "builtincmd %i %s %i\n",					// JLH: DEBUG mode log entry; add non-DEBUG mode entry
                    operationPtr->schemaData.builtincmd, __FILE__, __LINE__);
            /* this would be weird for flow */
        }

        break;

      case TOPIC_SCHEMAREQUEST:
      {
        MessageUserSchema &msgref = *(MessageUserSchema *)msgrcv;			// JLH: removed 'class' keyword
        tainstance = msgref.instance;

        switch (msgref.builtincmd)
        {
          case BUILTINCREATESCHEMA:
            TAcreateschema();
            break;

          case BUILTINCREATETABLE:
            TAcreatetable();
            break;

          case BUILTINADDCOLUMN:
            TAaddcolumn();
            break;

          case BUILTINDELETEINDEX:
            TAdeleteindex();
            break;

          case BUILTINDELETETABLE:
            TAdeletetable();
            break;

          case BUILTINDELETESCHEMA:
            TAdeleteschema();
            break;

          default:
            fprintf(logfile, "builtincmd unrecognized %li %s %i\n",
                    msgref.builtincmd, __FILE__, __LINE__);				// JLH: DEBUG mode log entry; create non-debug mode entry
        }
      }
      break;

      case TOPIC_TRANSACTION:
      {
        MessageTransaction &msgref = *(MessageTransaction *)msgrcv;			// JLH: removed 'class' keyword

        // need pendingTransactions
        if (Transactions.count(msgref.transactionid))
        {
          Transactions[msgref.transactionid]->processTransactionMessage(msgrcv);
        }
        else
        {
          // have to check for a LOCKED message cmd, to bounce back a
          // message to roll it back
          fprintf(logfile, "%s %i transactionid %li\n", __FILE__, __LINE__,		// JLH: DEBUG mode log entry; create non-DEBUG mode entry
                  msgref.transactionid);
          fprintf(logfile, "%s %i thismsg %p next ptr, count %p %lu, payloadtype %i pendingcmdid %li entrypoint %li locktype %i\n", __FILE__, __LINE__, msgrcv, Mbox::getPtr(msgref.nextmsg), Mbox::getCount(msgref.nextmsg), msgref.payloadtype, msgref.transaction_pendingcmdid, msgref.transaction_tacmdentrypoint, ((class MessageSubtransactionCmd *)msgrcv)->cmd.locktype);
          badMessageHandler();
        }

        break;
      }

      case TOPIC_DEADLOCKABORT:
      {
        MessageDeadlock &msgref = *(MessageDeadlock *)msgrcv;

        if (Transactions.count(msgref.transactionid))
        {
          Transactions[msgref.transactionid]->deadlockAbort(msgref);
        }
      }
      break;

      case TOPIC_TOPOLOGY:
        mboxes.update(myTopology, instance);
        updateReplicas();
        break;

      case TOPIC_ACKDISPATCH:
      {
        MessageAckDispatch &msgref = *(MessageAckDispatch *)msgrcv;

        // need pendingTransactions
        if (Transactions.count(msgref.transactionid))
        {
          // for now 4/5/13 don't think about msgref.status
          Transactions[msgref.transactionid]->continueCommitTransaction(1);
        }
      }
      break;

      case TOPIC_PROCEDURE1:
        newprocedure(2);
        break;

      case TOPIC_PROCEDURE2:
        newprocedure(3);
        break;

      case TOPIC_DISPATCH:
        handledispatch();
        break;

      case TOPIC_ACKAPPLY:
      {
        MessageAckApply &msgref = *(MessageAckApply *)msgrcv;

        if (Appliers.count(msgref.applierid))
        {
          Appliers[msgref.applierid]->ackedApply(msgref);
          }
        else
        {
          printf("%s %i no Applier to ack status %i %li,%li,%li\n", __FILE__,			// JLH: DEBUG mode entry; create non-DEBUG mode entry
                 __LINE__, msgref.status, msgref.subtransactionid,
                 msgref.applierid, msgref.partitionid);
        }
      }
      break;

      case TOPIC_OPERATION:
      {
        operationMap::iterator it;
        it = pendingOperations.find(((MessageUserSchema *)msgrcv)->operationid);

        if (it != pendingOperations.end())
        {
          class Operation &operationRef = *it->second;
          operationRef.handleOperation(*((class MessageUserSchema *)msgrcv));
        }
      }
      break;

      case TOPIC_COMPILE:
        newstatement();
        break;

      case TOPIC_TABLENAME:
      {
        MessageUserSchema &msgrcvref = *(MessageUserSchema *)msgrcv;				// JLH: removed 'class' keyword
        domainidsToSchemata[msgrcvref.domainid]->tableNameToId[msgrcvref.argstring] =
          msgrcvref.tableid;
      }
      break;

      case TOPIC_FIELDNAME:
      {
        MessageUserSchema &msgrcvref = *(MessageUserSchema *)msgrcv;				// JLH: removed 'class' keyword
        domainidsToSchemata[msgrcvref.domainid]->fieldNameToId[msgrcvref.tableid][msgrcvref.argstring] = msgrcvref.fieldid;
      }
      break;

      default:						// add DEBUG mode, non-DEBUG mode logging
        fprintf(logfile, "anomaly %i %s %i\n",
                msgrcv->topic, __FILE__, __LINE__);		// DEBUG mode log entry to include __FILE__, __LINE__
    }
  }	// JLH: end while(true) main event loop
}	// JLH: end constructor

TransactionAgent::~TransactionAgent()
{
}

void TransactionAgent::endConnection(void)
{
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Entering endConnection()");
#endif
  epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, NULL);	// JLH: add DEBUG info here & to epoll_ctl()
  close(sockfd);					// JLH: add DEBUG info here
  loggedInUsers.erase(sockfd);				// JLH: add DBEUG info here & to loggedInUsers.erase()
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Leaving endConnection(): reached end of function");
#endif
}

int64_t TransactionAgent::readSocket(void)
{
#ifndef NDEBUG
    fprintf(logfile, "DEBUG:\t%s %i Entering readSocket(): Trying to read data from socket.\n", __FILE__, __LINE__);
#endif

  char inbuf[PAYLOADSIZE];				// JLH: 128
  ssize_t bytesread = read(sockfd, inbuf, PAYLOADSIZE);	// JLH: add DEBUG info 

  if (bytesread < MESSAGE_LENGTH_NUMBER_OF_BYTES)					// JLH: what does 8 represent? 
  {
#ifndef NDEBUG
    fprintf(logfile, "DEBUG:\t%s %i Leaving readSocket(): Read fewer than 8 bytes.\n", __FILE__, __LINE__);
#endif
    return -1;
  }

  uint64_t a;
  memcpy(&a, inbuf, sizeof(a));
  int64_t msgsize = be64toh(a);				// JLH: convert from big-endian to host byte order

  if (bytesread-MESSAGE_LENGTH_NUMBER_OF_BYTES != msgsize)	// JLH: what does 8 represent? Minimum message size?
  {
    printf("%s %i bytesread %li msgsize %li sockfd %i\n", __FILE__, __LINE__, bytesread, msgsize, sockfd);
#ifndef NDEBUG
    fprintf(logfile, "DEBUG:\t%s %i Leaving readSocket(): expecting %li bytes but only read %li.\n", __FILE__, __LINE__, (bytesread-8), bytesread);
#endif
    return -2;						// JLH: convert to SYMBOLIC CONSTANT
  }

  char operationlength = inbuf[MESSAGE_LENGTH_NUMBER_OF_BYTES];		// Should 8 be defined as OPERATION_LENGTH_OFFSET ?

  if ((int)operationlength >= msgsize-1)
  {
    return -3;						// JLH: convert to SYMBOLIC CONSTANT
  }

  int64_t localargsize = msgsize - 1 - (int)operationlength;
  operation->assign(inbuf+9, (int)operationlength);
  memset(&args, 0, PAYLOADSIZE);
  memcpy(args, inbuf+9+(int)operationlength, localargsize);

#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Leaving readSocket(): reached end of method.");
#endif
  return localargsize;
}

// launcher, regular function
void *transactionAgent(void *identity)
{
  TransactionAgent((Topology::partitionAddress *)identity);
  return NULL;
}

// builtins
void TransactionAgent::ping(builtincmds_e cmd)
{
  vector<string> rv;
  sendResponse(false, STATUS_OK, &rv);
}

/*
Client sends login cmd with domainid, username, password
TransactionAgent forwards it to UserSchemaMgr, which compares password with password map.
Then it sends LOGINOK message to TransactionAgent in response.
TransactionAgent replies STATUS_OK to client.
&rv is message contents, which could be an empty vector of strings, as is the case of login response.
*/
void TransactionAgent::login(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      operationPtr = new class Operation(OP_AUTH, this, -1, -1);
      operationid = operationPtr->getid();
      //      pendingOperations[operationid] = operationPtr;

      vector<string> v;
      msgpack2Vector(&v, args, argsize);
      operationPtr->setDomainName(v[0]);
      MessageUserSchema *msg = new class MessageUserSchema(TOPIC_LOGIN);	// JLH: removed 'class' keyword
      MessageUserSchema &msgref = *msg;						// JLH: removed 'class' keyword
      msgref.topic = TOPIC_LOGIN;
      msgref.payloadtype = PAYLOADUSERSCHEMA;
      msgref.argsize = argsize;
      msgref.instance = instance;
      msgref.operationid = operationid;
      msgref.argstring.assign(args, 0, argsize);
      mboxes.toUserSchemaMgr(this->myIdentity.address, msgref);
    }
    break;

    case OKCMD:
    {
      // let's hope I remember to populate the object's domainid & userid before
      // calling this
      MessageUserSchema &msgrcvref =						// JLH: removed 'class' keyword
            *(class MessageUserSchema *)msgrcv;
      operationPtr = pendingOperations[operationid];
      authInfo aInfo;
      aInfo.domainid = msgrcvref.domainid;
      aInfo.userid = msgrcvref.userid;
      aInfo.domainName.assign(operationPtr->domainName);
      loggedInUsers[sockfd] = aInfo;
      vector<string> rv;
      sendResponse(false, STATUS_OK, &rv);
      endOperation();
    }
    break;

    case NOTOKCMD:
    {
      if (__sync_add_and_fetch(&cfgs.badloginmessages, 0))
      {
        vector<string> rv;
        sendResponse(false, STATUS_NOTOK, &rv);
      }

      endOperation();
      endConnection();
    }
    break;

    default:
      fprintf(logfile, "TransactionAgent::login cmd unrecognized %i\n", cmd);
  }
}

void TransactionAgent::logout(builtincmds_e cmd)
{
  vector<string> rv;
  sendResponse(false, STATUS_OK, &rv);
  endConnection();
}

void TransactionAgent::changepassword(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      operationPtr = new class Operation(OP_AUTH, this, userid, domainid);
      operationid = operationPtr->getid();
      //      pendingOperations[operationid] = operationPtr;
      MessageUserSchema *msg =						// JLH: removed 'class' keyword
            new class MessageUserSchema(TOPIC_CHANGEPASSWORD);
      MessageUserSchema &msgref = *msg;					// JLH: removed 'class' keyword
      msgref.topic = TOPIC_CHANGEPASSWORD;
      msgref.payloadtype = PAYLOADUSERSCHEMA;
      msgref.argsize = argsize;
      msgref.instance = instance;
      msgref.operationid = operationid;
      msgref.domainid = domainid;
      msgref.userid = userid;
      msgref.argstring.assign(args, 0, argsize);
      //      mboxes.userSchemaMgr.send(msgsnd, true);
      mboxes.toUserSchemaMgr(this->myIdentity.address, msgref);
    }
    break;

    case OKCMD:
  {
      vector<string> rv;
      sendResponse(false, STATUS_OK, &rv);
      endOperation();
    }
    break;

    case NOTOKCMD:
    {
      vector<string> rv;
      sendResponse(false, STATUS_NOTOK, &rv);
      endOperation();
    }
    break;

    default:
      fprintf(logfile, "TransactionAgent::changepassword cmd unrecognized %i\n",
              cmd);
  }
}

void TransactionAgent::createdomain(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      operationPtr = new class Operation(OP_AUTH, this, userid, domainid);
      operationid = operationPtr->getid();
      //      pendingOperations[operationid] = operationPtr;
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_CREATEDOMAIN);
      class MessageUserSchema &msgref = *msg;
      msgref.topic = TOPIC_CREATEDOMAIN;
      msgref.payloadtype = PAYLOADUSERSCHEMA;
      msgref.argsize = argsize;
      msgref.instance = instance;
      msgref.operationid = operationid;
      msgref.domainid = domainid;
      msgref.userid = userid;
      msgref.argstring.assign(args, 0, argsize);
      //      mboxes.userSchemaMgr.send(msgsnd, true);
      mboxes.toUserSchemaMgr(this->myIdentity.address, msgref);
    }
    break;

    case OKCMD:
    {
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_CREATEUSER);
      class MessageUserSchema &msgref = *msg;
      vector<string> rv;
      // this is created domainid:
      rv.push_back(boost::lexical_cast<string>(msgref.domainid));
      sendResponse(false, STATUS_OK, &rv);
      endOperation();
    }
    break;

    case NOTOKCMD:
    {
      vector<string> rv;
      sendResponse(false, STATUS_NOTOK, &rv);
      endOperation();
    }
    break;

    default:
      fprintf(logfile, "TransactionAgent::createdomain cmd unrecognized %i\n",
              cmd);
  }
}

void TransactionAgent::createuser(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      operationPtr = new class Operation(OP_AUTH, this, userid, domainid);
      operationid = operationPtr->getid();
      //      pendingOperations[operationid] = operationPtr;
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_CREATEUSER);
      class MessageUserSchema &msgref = *msg;
      msgref.topic = TOPIC_CREATEUSER;
      msgref.payloadtype = PAYLOADUSERSCHEMA;
      msgref.argsize = argsize;
      msgref.instance = instance;
      msgref.operationid = operationid;
      msgref.domainid = domainid;
      msgref.userid = userid;
      msgref.argstring.assign(args, 0, argsize);
      mboxes.toUserSchemaMgr(this->myIdentity.address, msgref);
    }
    break;

    case OKCMD:
  {
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_CREATEUSER);
      class MessageUserSchema &msgref = *msg;
      vector<string> rv;
      // this is created userid:
      rv.push_back(boost::lexical_cast<string>(msgref.userid));
      sendResponse(false, STATUS_OK, &rv);
      endOperation();
    }
    break;

    case NOTOKCMD:
  {
      vector<string> rv;
      sendResponse(false, STATUS_NOTOK, &rv);
      endOperation();
    }
    break;

    default:
      fprintf(logfile, "TransactionAgent::createuser cmd unrecognized %i\n",
              cmd);
  }
}

void TransactionAgent::deleteuser(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      operationPtr = new class Operation(OP_AUTH, this, userid, domainid);
      operationid = operationPtr->getid();
      //      pendingOperations[operationid] = operationPtr;
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_DELETEUSER);
      class MessageUserSchema &msgref = *msg;
      msgref.topic = TOPIC_DELETEUSER;
      msgref.payloadtype = PAYLOADUSERSCHEMA;
      msgref.argsize = argsize;
      msgref.instance = instance;
      msgref.operationid = operationid;
      msgref.domainid = domainid;
      msgref.userid = userid;
      msgref.argstring.assign(args, 0, argsize);
      //      mboxes.userSchemaMgr.send(msgsnd, true);
      mboxes.toUserSchemaMgr(this->myIdentity.address, msgref);
    }
    break;

    case OKCMD:
  {
      vector<string> rv;
      sendResponse(false, STATUS_OK, &rv);
      endOperation();
    }
    break;

    case NOTOKCMD:
    {
      vector<string> rv;
      sendResponse(false, STATUS_NOTOK, &rv);
      endOperation();
    }
    break;

    default:
      fprintf(logfile, "TransactionAgent::deleteuser cmd unrecognized %i\n",
              cmd);
  }
}

void TransactionAgent::deletedomain(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      operationPtr = new class Operation(OP_AUTH, this, userid, domainid);
      operationid = operationPtr->getid();
      //      pendingOperations[operationid] = operationPtr;
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_DELETEDOMAIN);
      class MessageUserSchema &msgref = *msg;
      msgref.topic = TOPIC_DELETEDOMAIN;
      msgref.payloadtype = PAYLOADUSERSCHEMA;
      msgref.argsize = argsize;
      msgref.instance = instance;
      msgref.operationid = operationid;
      msgref.domainid = domainid;
      msgref.userid = userid;
      msgref.argstring.assign(args, 0, argsize);
      //      mboxes.userSchemaMgr.send(msgsnd, true);
      mboxes.toUserSchemaMgr(this->myIdentity.address, msgref);
    }
    break;

    case OKCMD:
    {
      vector<string> rv;
      sendResponse(false, STATUS_OK, &rv);
      endOperation();
    }
    break;

    case NOTOKCMD:
    {
      vector<string> rv;
      sendResponse(false, STATUS_NOTOK, &rv);
      endOperation();
    }
    break;

    default:
      fprintf(logfile, "TransactionAgent::deletedomain cmd unrecognized %i\n",
              cmd);
  }
}

// schema builtins
void TransactionAgent::createschema(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
      schemaBoilerplate(cmd, BUILTINCREATESCHEMA);
      break;

    case USMRESPONSECMD:
      schemaBoilerplate(cmd, BUILTINCREATESCHEMA);
      break;

    case TASENGINESRESPONSECMD:
      responseVector.clear();
      schemaBoilerplate(cmd, BUILTINCREATESCHEMA);
      break;

    default:
      fprintf(logfile, "topic unrecognized %i %s %i\n", cmd, __FILE__,
              __LINE__);
  }
}

void TransactionAgent::createtable(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
      schemaBoilerplate(cmd, BUILTINCREATETABLE);
      break;

    case USMRESPONSECMD:
    {
      schemaBoilerplate(cmd, BUILTINCREATETABLE);

      class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;
      class MessageUserSchema msg;
      msg.topic = TOPIC_TABLENAME;
      msg.payloadtype = PAYLOADUSERSCHEMA;
      msg.domainid = msgrcvref.domainid;
      msg.tableid = msgrcvref.tableid;
      msg.argstring = msgrcvref.argstring;

      mboxes.toAllOfType(ACTOR_TRANSACTIONAGENT, myIdentity.address, msg);
    }
    break;

    case TASENGINESRESPONSECMD:
      responseVector.clear();
      responseVector.push_back(boost::lexical_cast<string>
                               (((class MessageUserSchema *)msgrcv)->tableid));
      schemaBoilerplate(cmd, BUILTINCREATETABLE);
      break;

    default:
      fprintf(logfile, "topic unrecognized %i %s %i\n", cmd, __FILE__,
              __LINE__);
  }
}

void TransactionAgent::addcolumn(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
      schemaBoilerplate(cmd, BUILTINADDCOLUMN);
      break;

    case USMRESPONSECMD:
    {
      schemaBoilerplate(cmd, BUILTINADDCOLUMN);

      class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;
      class MessageUserSchema msg;
      msg.topic = TOPIC_FIELDNAME;
      msg.payloadtype = PAYLOADUSERSCHEMA;
      msg.domainid = msgrcvref.domainid;
      msg.tableid = msgrcvref.tableid;
      msg.fieldid = msgrcvref.fieldid;
      msg.argstring = msgrcvref.argstring;

      mboxes.toAllOfType(ACTOR_TRANSACTIONAGENT, myIdentity.address, msg);
    }
    break;

    case TASENGINESRESPONSECMD:
      responseVector.clear();
      responseVector.push_back(boost::lexical_cast<string>
                               (((class MessageUserSchema *)msgrcv)->fieldid));
      schemaBoilerplate(cmd, BUILTINADDCOLUMN);
      break;

    default:
      fprintf(logfile, "topic unrecognized %i %s %i\n", cmd, __FILE__,
              __LINE__);
  }
}

void TransactionAgent::deleteindex(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
      schemaBoilerplate(cmd, BUILTINDELETEINDEX);
      break;

    case USMRESPONSECMD:
      schemaBoilerplate(cmd, BUILTINDELETEINDEX);
      break;

    case TASENGINESRESPONSECMD:
      responseVector.clear();
      schemaBoilerplate(cmd, BUILTINDELETEINDEX);
      break;

    default:
      fprintf(logfile, "topic unrecognized %i %s %i\n", cmd, __FILE__,
              __LINE__);
  }
}

void TransactionAgent::deletetable(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
      schemaBoilerplate(cmd, BUILTINDELETETABLE);
      break;

    case USMRESPONSECMD:
      schemaBoilerplate(cmd, BUILTINDELETETABLE);
      break;

    case TASENGINESRESPONSECMD:
      responseVector.clear();
      schemaBoilerplate(cmd, BUILTINDELETETABLE);
      break;

    default:
      fprintf(logfile, "topic unrecognized %i %s %i\n", cmd, __FILE__,
              __LINE__);
  }
}

void TransactionAgent::deleteschema(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
      schemaBoilerplate(cmd, BUILTINDELETESCHEMA);
      break;

    case USMRESPONSECMD:
      schemaBoilerplate(cmd, BUILTINDELETESCHEMA);
      break;

    case TASENGINESRESPONSECMD:
      responseVector.clear();
      schemaBoilerplate(cmd, BUILTINDELETESCHEMA);
      break;

    default:
      fprintf(logfile, "topic unrecognized %i %s %i\n", cmd, __FILE__,
              __LINE__);
  }
}

// not builtin
void TransactionAgent::endOperation(void)
{
  delete pendingOperations[operationid];
  pendingOperations.erase(operationid);
}

// schema loopback functions
void TransactionAgent::TAcreateschema(void)
{
  createSchema(this);
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  TransactionAgent::usmReply(this, msgrcv->sourceAddr, *msg);
  domainProceduresMap domainProcedures;
  int64_t did = ((class MessageUserSchema *)msgrcv)->domainid;
  domainidsToProcedures[did] = domainProcedures;
}

void TransactionAgent::TAcreatetable(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;
  status =
    domainidsToSchemata[msgrcvref.domainid]->createTable(msgrcvref.tableid);
  class MessageUserSchema *msg =
        new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  class MessageUserSchema &msgref = *msg;
  msgref.tableid = msgrcvref.tableid;
  //    replyTa(this, TOPIC_SCHEMAREPLY);
  //  replyTa(this, TOPIC_SCHEMAREPLY, msg);
  TransactionAgent::usmReply(this,
                             ((class Message *)msgrcv)->sourceAddr, *msg);

}

void TransactionAgent::TAaddcolumn(void)
{
  class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;
  class Schema *schemaPtr = domainidsToSchemata[msgrcvref.domainid];
  class Table *tablePtr = schemaPtr->tables[msgrcvref.tableid];
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  class MessageUserSchema &msgref = *msg;
  msgref.fieldid = tablePtr->addfield((fieldtype_e) msgrcvref.fieldtype,
                                      msgrcvref.fieldlen, msgrcvref.argstring, (indextype_e) msgrcvref.indextype);
  status = BUILTIN_STATUS_OK;
  //  replyTa(this, TOPIC_SCHEMAREPLY, msg);
  TransactionAgent::usmReply(this,
                             ((class Message *)msgrcv)->sourceAddr, *msg);
}

void TransactionAgent::TAdeleteindex(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  status = BUILTIN_STATUS_OK;
  //  replyTa(this, TOPIC_SCHEMAREPLY, msg);
  TransactionAgent::usmReply(this,
                             msgrcv->sourceAddr, *msg);
}

void TransactionAgent::TAdeletetable(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  status = BUILTIN_STATUS_OK;
  //  replyTa(this, TOPIC_SCHEMAREPLY, msg);
  TransactionAgent::usmReply(this,
                             msgrcv->sourceAddr, *msg);
}

void TransactionAgent::TAdeleteschema(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  status = BUILTIN_STATUS_OK;
  //  replyTa(this, TOPIC_SCHEMAREPLY, msg);
  TransactionAgent::usmReply(this,
                             msgrcv->sourceAddr, *msg);
}

void TransactionAgent::schemaBoilerplate(builtincmds_e cmd, int builtin)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      operationPtr = new class Operation(OP_SCHEMA, this, userid, domainid);
      operationPtr->setbuiltincmd(builtin);
      operationid = operationPtr->getid();
      operationPtr->schemaData.state = usm;
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_SCHEMAREQUEST);
      class MessageUserSchema &msgref = *msg;
      msgref.topic = TOPIC_SCHEMAREQUEST;
      msgref.payloadtype = PAYLOADUSERSCHEMA;
      msgref.builtincmd = builtin;
      msgref.argsize = argsize;
      msgref.instance = instance;
      msgref.operationid = operationid;
      msgref.userid = userid;
      msgref.domainid = domainid;
      msgref.argstring.assign(args, 0, argsize);
      mboxes.toUserSchemaMgr(this->myIdentity.address, msgref);
    }
    break;

    case USMRESPONSECMD:
    {
      class MessageUserSchema &msgrcvref =
            *(class MessageUserSchema *)msgrcv;

      if (msgrcvref.status != BUILTIN_STATUS_OK)   // abort
      {
        responseVector.clear();
        sendResponse(false, STATUS_NOTOK, &responseVector);
        endOperation();
        return;
      }

      class MessageUserSchema msg(TOPIC_SCHEMAREQUEST);

      msg.topic = TOPIC_SCHEMAREQUEST;

      msg.payloadtype = PAYLOADUSERSCHEMA;

      msg.tableid = msgrcvref.tableid;

      msg.builtincmd = builtin;

      msg.instance = instance;

      msg.operationid = operationid;

      msg.domainid = domainid;

      msg.fieldtype = msgrcvref.fieldtype;

      msg.fieldlen = msgrcvref.fieldlen;

      if (msgrcvref.argsize)
      {
        msg.argsize = msgrcvref.argsize;
        msg.argstring.assign(args, 0, argsize);
      }
      else
      {
        msg.argstring=msgrcvref.argstring;
      }

      msg.indextype = msgrcvref.indextype;
      msg.indexid = msgrcvref.indexid;
      msg.tableindexid = msgrcvref.tableindexid;
      msg.simple = msgrcvref.simple;
      msg.fieldid = msgrcvref.fieldid;
      msg.numfields = msgrcvref.numfields;

      operationPtr->schemaData.msgwaits = mboxes.toAllOfType(
                                            ACTOR_TRANSACTIONAGENT, myIdentity.address, msg);
      operationPtr->schemaData.msgwaits += mboxes.toAllOfType(
                                             ACTOR_ENGINE, myIdentity.address, msg);

      operationPtr->schemaData.state = tasengines;
    }
    break;

    case TASENGINESRESPONSECMD:
    {
      class MessageUserSchema &msgrcvref =
            *(class MessageUserSchema *)msgrcv;

      if (msgrcvref.status != BUILTIN_STATUS_OK)
    {
        responseVector.clear();
        sendResponse(false, STATUS_NOTOK, &responseVector);
        endOperation();
        return;
      }

      if (--operationPtr->schemaData.msgwaits)
      {
        // not ready yet
        return;
      }

      sendResponse(false, STATUS_OK, &responseVector);
      endOperation();
    }
    break;

    default:
      fprintf(logfile, "bad case %i %s %i\n", cmd, __FILE__, __LINE__);
  }
}

void TransactionAgent::loadprocedure(builtincmds_e cmd)
{
  newprocedure(1);
}

void TransactionAgent::compile(builtincmds_e cmd)
{
  vector<string> resultVector;
  msgpack2Vector(&resultVector, args, argsize);
  //  int64_t sid = atol(resultVector[0].c_str());
  string statementname(resultVector[0]);
  string sqlstatement(resultVector[1]);
  class Larxer lx2((char *)sqlstatement.c_str(), this,
                       domainidsToSchemata[domainid]);

  if (lx2.statementPtr==NULL)
  {
    vector<string> rv;
    sendResponse(false, STATUS_NOTOK, &rv);
    return;
  }

  delete lx2.statementPtr;

  class MessageUserSchema msg;
  msg.topic = TOPIC_COMPILE;
  msg.payloadtype = PAYLOADUSERSCHEMA;
  msg.domainid = domainid;
  msg.procname = statementname;
  msg.argstring = sqlstatement;

  mboxes.toAllOfType(ACTOR_TRANSACTIONAGENT, myIdentity.address, msg);

  vector<string> rv;
  rv.push_back(statementname);
  sendResponse(false, STATUS_OK, &rv);
}

msgpack::sbuffer *makeSbuf(msgpack::sbuffer *sbuf)
{
  return sbuf;
}

msgpack::sbuffer *makeSbuf(vector<string> *v)
{
  msgpack::sbuffer *sbuf = new msgpack::sbuffer;
  msgpack::pack(*sbuf, *v);
  return sbuf;
}

msgpack::sbuffer *makeSbuf(map<string, string> *m)
{
  msgpack::sbuffer *sbuf = new msgpack::sbuffer;
  msgpack::pack(*sbuf, *m);
  return sbuf;
}

int64_t TransactionAgent::getnexttransactionid()
{
  return ++nexttransactionid;
}

int64_t TransactionAgent::getnextapplierid()
{
  return ++nextapplierid;
}

void TransactionAgent::badMessageHandler()
{
  fprintf(logfile, "TA bad message stub %s %i\n", __FILE__, __LINE__); // stubby
}

void TransactionAgent::updateReplicas()
{
  // replicaMembers[replica][member] = nodeid
  //  vector< vector<int64_t> > replicaMembers;
  // tas[nodeid][tainstance] = actorid
  //  vector< vector<int64_t> > tas;

  // get ta instance based on my actorid, then build the replica members
  // if replicas > 2, or just find the other one if numreplicas==2
  // find my replica
  if (myTopology.numreplicas <= 1)
  {
    return;
  }

  for (size_t n=0; n < myTopology.replicaMembers.size(); n++)
  {
    for (size_t m=0; m < myTopology.replicaMembers[n].size(); m++)
    {
      if (myTopology.replicaMembers[n][m]==myIdentity.address.nodeid)
      {
        myreplica = n;
        mymember = m;
      }
    }
  }

  if (myTopology.numreplicas==2)
  {
    size_t otherreplica = myreplica==0 ? 1 : 0;
    replicaAddress.nodeid = myTopology.replicaMembers[otherreplica][mymember];
    int64_t othernodeid = myTopology.replicaMembers[otherreplica][mymember];

    if (othernodeid)
    {
      replicaAddress.actorid = myTopology.tas[othernodeid][myIdentity.instance];
    }

    return;
  }

  vector<Topology::addressStruct> ras;

  for (size_t n=0; n < myTopology.replicaMembers.size(); n++)
  {
    if (myreplica==n)
    {
      continue;
    }

    ras.push_back({myTopology.replicaMembers[n][mymember],
                   myTopology.tas[n][myIdentity.instance]
                  });
  }

  replicaAddresses.swap(ras);
}

void TransactionAgent::newprocedure(int64_t entrypoint)
{
  switch (entrypoint)
  {
    case 1: // client sends loadprocedure command
    {
      vector<string> resultVector;
      msgpack2Vector(&resultVector, args, argsize);

      class MessageUserSchema msg;
      msg.topic = TOPIC_PROCEDURE1;
      msg.payloadtype = PAYLOADUSERSCHEMA;
      msg.domainid = domainid;
      msg.pathname = resultVector[0];
      msg.procname = storedprocprefix;
      //      msg.procname = "DURABLE_";
      msg.procname += domainName;
      msg.procname += "_";
      msg.procname.append(resultVector[1]);

      // get 1 ta from each node and send a copy of that message
      for (size_t n=0; n < myTopology.allActors.size(); n++)
      {
        for (size_t m=0; m < myTopology.allActors[n].size(); m++)
        {
          if (myTopology.allActors[n][m]==ACTOR_TRANSACTIONAGENT)
          {
            class MessageUserSchema *nmsg = new class MessageUserSchema;
            *nmsg = msg;
            mboxes.toActor(myIdentity.address, {(int64_t)n, (int64_t)m}, *nmsg);

            break;
          }
        }
      }

      vector<string> rv;
      sendResponse(false, STATUS_OK, &rv);
    }
    break;

    case 2: // TOPIC_PROCEDURE1, load procedure, then send to all ta's this node
    {
      class MessageUserSchema &inmsg = *((class MessageUserSchema *)msgrcv);

      const char *dlsym_error;
      dlerror();
      void *soPtr = dlopen(inmsg.pathname.c_str(), RTLD_LAZY);

      if (!soPtr)
      {
        dlsym_error = dlerror();
        puts(dlsym_error);
        return;
      }

      dlerror();

      string funcNameCreate = inmsg.procname + "_create";
      spclasscreate call_func1create =
        (spclasscreate) dlsym(soPtr, funcNameCreate.c_str());
      dlsym_error = dlerror();

      if (dlsym_error)
      {
        printf("%s %i anomaly nodeid %li instance %li error %s\n", __FILE__, __LINE__, myTopology.nodeid, myIdentity.instance, dlsym_error);
        return;
      }

      dlerror();

      string funcNameDestroy = inmsg.procname + "_destroy";
      spclassdestroy call_func1destroy =
        (spclassdestroy) dlsym(soPtr, funcNameDestroy.c_str());
      dlsym_error = dlerror();

      if (dlsym_error)
      {
        return;
      }

      dlerror();

      class MessageUserSchema msg;
      msg.topic = TOPIC_PROCEDURE2;
      msg.payloadtype = PAYLOADUSERSCHEMA;
      msg.domainid = inmsg.domainid;
      msg.procs.procedurecreator = (void *)call_func1create;
      msg.procs.proceduredestroyer = (void *)call_func1destroy;
      msg.intdata = inmsg.procname.length();
      msg.argstring.assign(inmsg.procname, 0, msg.intdata);

      for (size_t n=0; n < myTopology.allActors[myTopology.nodeid].size(); n++)
      {
        if (myTopology.allActors[myTopology.nodeid][n]==ACTOR_TRANSACTIONAGENT)
        {
          class MessageUserSchema *nmsg = new class MessageUserSchema;
          *nmsg = msg;
          mboxes.toActor(myIdentity.address, {myTopology.nodeid, (int64_t)n},
                         *nmsg);
        }
      }
    }
    break;

    case 3:
    {
      class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;
      domainidsToProcedures[msgrcvref.domainid][msgrcvref.argstring] =
        msgrcvref.procs;
    }
    break;

    default:
      fprintf(logfile, "anomaly: %lu %s %i\n", entrypoint, __FILE__, __LINE__);
  }
}

void TransactionAgent::handledispatch()
{
  class MessageDispatch &msgrcvref = *(class MessageDispatch *)msgrcv;
  domainid = msgrcvref.domainid;
  class MessageAckDispatch *msg =
        new class MessageAckDispatch(msgrcvref.transactionid, STATUS_OK);
  mboxes.toActor(myIdentity.address, msgrcvref.sourceAddr, *msg);

  int64_t partitioncount=0;
  boost::unordered_map<int64_t, class MessageApply *> msgs;
class Applier *applierPtr = new class Applier(this, domainid,
            msgrcvref.sourceAddr, partitioncount);

  map< int64_t, vector<MessageDispatch::record_s> >::iterator it;

  for (it = msgrcvref.records.begin(); it != msgrcvref.records.end(); it++)
{
    // it->first int64_t partitionid, it->second vector of records
    vector<MessageDispatch::record_s> &recordsref = it->second;

    if (!msgs.count(it->first))
    {
      msgs[it->first] = new class MessageApply(msgrcvref.pidsids[it->first],
              applierPtr->applierid, domainid);
    }

    msgs[it->first]->rows = recordsref;
    class Schema *schemaPtr = domainidsToSchemata[domainid];

    for (size_t n=0; n < recordsref.size(); n++)
    {
      class Table &tableRef = *schemaPtr->tables[recordsref[n].tableid];

      switch (recordsref[n].primitive)
      {
        case INSERT:
        {
          vector<fieldValue_s> fields;
          tableRef.unmakerow(&recordsref[n].row, &fields);

          for (size_t f=0; f < tableRef.fields.size(); f++)
          {
            if (tableRef.fields[f].indextype==NONE)
            {
              continue;
            }

            // hence, create new index entry
            MessageApply::applyindex_s indexinfo;
            indexinfo.fieldVal = fields[f];
            indexinfo.fieldid = f;
            indexinfo.flags = 0;
            MessageApply::setisaddflag(&indexinfo.flags);
            indexinfo.tableid = recordsref[n].tableid;
            indexinfo.entry = {recordsref[n].rowid,
                               getPartitionid(fields[f], tableRef.fields[f].type,
                                              myTopology.numpartitions)
                              };

            msgs[indexinfo.entry.engineid]->indices.push_back(indexinfo);
          }
        }
        break;

        case UPDATE:
        {
          vector<fieldValue_s> newfields;
          tableRef.unmakerow(&recordsref[n].row, &newfields);
          vector<fieldValue_s> oldfields;
          tableRef.unmakerow(&recordsref[n].oldrow, &oldfields);

          for (size_t f=0; f < tableRef.fields.size(); f++)
          {
            if (tableRef.fields[f].indextype==NONE)
            {
              continue;
            }

            // only add entries if new & old are different
            bool aredifferent;

            switch (tableRef.fields[f].type)
            {
              case INT:
                aredifferent = newfields[f].value.integer !=
                               oldfields[f].value.integer ? true : false;
                break;

              case UINT:
                aredifferent = newfields[f].value.uinteger !=
                               oldfields[f].value.uinteger ? true : false;
                break;

              case BOOL:
                aredifferent = newfields[f].value.boolean !=
                               oldfields[f].value.boolean ? true : false;
                break;

              case FLOAT:
                aredifferent = newfields[f].value.floating !=
                               oldfields[f].value.floating ? true : false;
                break;

              case CHAR:
                aredifferent = newfields[f].value.character !=
                               oldfields[f].value.character ? true : false;
                break;

              case CHARX:
                aredifferent = newfields[f].str.compare(oldfields[f].str)
                               ? true : false;
                break;

              case VARCHAR:
                aredifferent = newfields[f].str.compare(oldfields[f].str)
                               ? true : false;
                break;

              default:
                printf("%s %i anomaly fieldtype %i\n", __FILE__, __LINE__,
                       tableRef.fields[f].type);
                aredifferent=false;
            }

            if (aredifferent==true)
            {
              // delete the old, add the new
              MessageApply::applyindex_s indexinfo;
              indexinfo.fieldVal = oldfields[f];
              indexinfo.fieldid = f;
              indexinfo.flags = 0;
              indexinfo.tableid = recordsref[n].tableid;
              indexinfo.entry = {recordsref[n].rowid,
                                 getPartitionid(oldfields[f], tableRef.fields[f].type,
                                                myTopology.numpartitions)
                                };

              msgs[indexinfo.entry.engineid]->indices.push_back(indexinfo);

              indexinfo.fieldVal = newfields[f];
              indexinfo.fieldid = f;
              indexinfo.flags = 0;
              MessageApply::setisaddflag(&indexinfo.flags);
              indexinfo.tableid = recordsref[n].tableid;
              indexinfo.entry = {recordsref[n].rowid,
                                 getPartitionid(newfields[f], tableRef.fields[f].type,
                                                myTopology.numpartitions)
                                };

              msgs[indexinfo.entry.engineid]->indices.push_back(indexinfo);
            }
          }
        }
        break;

        case DELETE:
        {
          vector<fieldValue_s> fields;
          tableRef.unmakerow(&recordsref[n].oldrow, &fields);

          for (size_t f=0; f < tableRef.fields.size(); f++)
          {
            if (tableRef.fields[f].indextype==NONE)
            {
              continue;
            }

            // hence, create new index entry
            MessageApply::applyindex_s indexinfo;
            indexinfo.fieldVal = fields[f];
            indexinfo.flags = 0;
            indexinfo.fieldid = f;
            indexinfo.tableid = recordsref[n].tableid;
            indexinfo.entry = {recordsref[n].rowid,
                               getPartitionid(fields[f], tableRef.fields[f].type,
                                              myTopology.numpartitions)
                              };

            msgs[indexinfo.entry.engineid]->indices.push_back(indexinfo);
          }
        }
        break;

        default:
          printf("%s %i anomaly primitive %i\n", __FILE__, __LINE__,
                 recordsref[n].primitive);
      }
    }
  }

  boost::unordered_map<int64_t, class MessageApply *>::iterator it2;

  for (it2 = msgs.begin(); it2 != msgs.end(); it2++)
  {
    mboxes.toPartition(myIdentity.address, it2->first, *it2->second);
  }
}

void TransactionAgent::newstatement()
{
  MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;		// JLH: removed 'class' keyword

  Larxer lx((char *)msgrcvref.argstring.c_str(), this,				// JLH: removed 'class' keyword
                      domainidsToSchemata[msgrcvref.domainid]);

  if (lx.statementPtr==NULL)
  {
    printf("%s %i anomaly\n", __FILE__, __LINE__);
  }

  lx.statementPtr->resolveTableFields();
  statements[msgrcvref.domainid][msgrcvref.procname] = *lx.statementPtr;
  delete lx.statementPtr;
  //  printf("%s %i stmt '%s' %p %p\n", __FILE__, __LINE__, msgrcvref.procname.c_str(), lx.statementPtr, statements[msgrcvref.domainid][msgrcvref.procname]);
  /*
  Statement stmt;								// JLH: removed 'class' keyword
  stmt = *lx.statementPtr;
  Statement *stmtPtr = statements[msgrcvref.domainid][msgrcvref.procname];
  stmt = *stmtPtr;								// JLH: removed 'class' keyword
   */

}

void TransactionAgent::tryMapBuiltinCmdsToMethods()	// JLH: add try ... catch block
{
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Entering tryMapBuiltinCmdsToMethods(): trying to map builtin commands to methods.");
#endif
  try {
  builtins["ping"] = &TransactionAgent::ping;
  builtins["login"] = &TransactionAgent::login;
  builtins["logout"] = &TransactionAgent::logout;
  builtins["changepassword"] = &TransactionAgent::changepassword;
  builtins["createdomain"] = &TransactionAgent::createdomain;
  builtins["createuser"] = &TransactionAgent::createuser;
  builtins["deleteuser"] = &TransactionAgent::deleteuser;
  builtins["deletedomain"] = &TransactionAgent::deletedomain;
  builtins["createschema"] = &TransactionAgent::createschema;
  builtins["createtable"] = &TransactionAgent::createtable;
  builtins["addcolumn"] = &TransactionAgent::addcolumn;
  builtins["deleteindex"] = &TransactionAgent::deleteindex;
  builtins["deletetable"] = &TransactionAgent::deletetable;
  builtins["deleteschema"] = &TransactionAgent::deleteschema;
  builtins["loadprocedure"] = &TransactionAgent::loadprocedure;
  builtins["compile"] = &TransactionAgent::compile;
  } catch (const std::exception& e) {
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "TransactionAgent::tryMapBuiltinCmdsToMethods() caught an exception.", e.what());
#endif
    std::cout << "Caught an exception when trying to map builtin commands to methods." << std::endl;
    exit(E_CAUGHT_EXCEPTION);
  }
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Leaving tryMapBuiltinCmdsToMethods(): reached end of method.");
#endif
}

TransactionAgent::builtinsMap::iterator TransactionAgent::tryFindOperations(string operation)	// JLH: add std (non-DEBUG) logging framework
{
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Entering TransactionAgent::tryFindOperations().");
#endif
  try {
     return builtins.find(operation);
  } catch (const std::exception& e) {
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "TransactionAgent::tryFindOperations() caught an exception.", e.what());
#endif
     std::cout << "Caught an exception when trying to find operation." << std::endl;
     exit(E_CAUGHT_EXCEPTION);
  }
#ifndef NDEBUG
  logDebugMessage(__FILE__, __LINE__, "Leaving TransactionAgent::tryFindOperations().");
#endif
}
