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
#line 29 "TransactionAgent.cc"

TransactionAgent::TransactionAgent(Topology::partitionAddress *myIdentityArg) :
  myIdentity(*myIdentityArg), nexttransactionid(0), nextapplierid(0),
  myreplica(-1), mymember(-1)
{
  delete myIdentityArg;
  epollfd = myIdentity.epollfd;
  instance = myIdentity.instance;
  mboxes.nodeid = myIdentity.address.nodeid;

#ifdef PROFILE
  rid = 0; // profiling
  profilecount = 0; //profiling
  profiles = new PROFILERCOMPREHENSIVE[PROFILEENTRIES];
#endif
  builtincmds_e cmd = NOCMD;
  spclasscreate spC;
  spclassdestroy spD;
  uint32_t events = 0;

  typedef boost::unordered_map<std::string,
          void (TransactionAgent::*)(builtincmds_e)> builtinsMap;
  builtinsMap builtins;
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

  operationid=0;
  int waitfor = 100;

  while (1)
  {
    // clear data from msgrcv
    domainid=-1;
    userid=-1;
    argsize=-1;
    sockfd=-1;

    for (size_t inmsg=0; inmsg < 50; inmsg++)
    {
//      msgrcv = mymbox.receive(waitfor);
      GETMSG(msgrcv, myIdentity.mbox, waitfor)

      if (msgrcv==NULL)
      {
        waitfor = 100;
        break;
      }

      waitfor = 0;

      if (msgrcv->messageStruct.payloadtype==PAYLOADUSERSCHEMA)
      {
        class MessageUserSchema &msgref =
              *((class MessageUserSchema *)msgrcv);

        if (msgrcv->messageStruct.topic != TOPIC_SCHEMAREQUEST)
      {
          // don't want to validate somebody else's operationid or override
          pendingOperationsIterator =
            pendingOperations.find(msgref.userschemaStruct.operationid);

          if (pendingOperationsIterator == pendingOperations.end())
          {
            ;
            //                    continue;
          }
          else
          {
            sockfd = pendingOperations[msgref.userschemaStruct.operationid]->sockfd;
            userid = pendingOperations[msgref.userschemaStruct.operationid]->userid;
            domainid = pendingOperations[msgref.userschemaStruct.operationid]->domainid;
          }
        }

        operationid = msgref.userschemaStruct.operationid;
      }

      switch (msgrcv->messageStruct.topic)
      {
        case TOPIC_SOCKET:
        {
#ifdef PROFILE
          gettimeofday(&inboundProfile[0].tv, NULL);
          inboundProfile[0].tag = 0;
#endif

          switch (((class MessageSocket *)msgrcv)->socketStruct.listenertype)
          {
            case LISTENER_RAW:
            {
              sockfd = ((class MessageSocket *)msgrcv)->socketStruct.socket;
              events = ((class MessageSocket *)msgrcv)->socketStruct.events;

              if ((events & EPOLLERR) || (events & EPOLLHUP))
              {
                endConnection();
                break;
              }

              if (events & EPOLLIN)
              {
                operation = (string *)new string;
                argsize = readSocket();

                if (argsize < 0)
                {
                  delete operation;
                  endConnection();
                  break;
                }

                // ok, if no user logged in, then can only login, else
                // break connection exit
                // if user logged in, then cannot login but everything else
                // login shouldn't be in binFunctions map therefore
                socketAuthInfo::iterator loggedInUsersIterator;
                loggedInUsersIterator = loggedInUsers.find(sockfd);

                if (loggedInUsersIterator == loggedInUsers.end())
                {
                  // this means not logged in
                  if (operation->compare("login")==0)
                  {
                    // so, login
                    login(STARTCMD);
                  }
                  else if (operation->compare("ping")==0 &&
                           __sync_add_and_fetch(&cfgs.anonymousping, 0))
                  {
                    ping(STARTCMD);
                  }
                  else     // gtfo
                  {
                    endConnection();
                    break;
                  }
                }
                else
                {
                  // get my domainid & userid
                  domainid = loggedInUsersIterator->second.domainid;
                  userid = loggedInUsersIterator->second.userid;
                  domainName = loggedInUsersIterator->second.domainName;

                  // first, check domain operations, when those are built
                  if (domainidsToProcedures.count(domainid))
                  {
                    if (domainidsToProcedures[domainid].count(*operation))
                    {
                      spC = (spclasscreate)domainidsToProcedures[domainid]
                            [*operation].procedurecreator;
                      spD = (spclassdestroy)domainidsToProcedures
                            [domainid][*operation].proceduredestroyer;
#ifdef PROFILE
                      gettimeofday(&inboundProfile[1].tv, NULL);
                      inboundProfile[1].tag = 1;
#endif
                      spC(this, NULL, (void *)spD);
#ifdef PROFILE
                      rid++; // bump rid here for profiling
#endif
                      continue;
                      break;
                    }
                  }

                  builtinsMap::iterator builtinsIterator;
                  builtinsIterator = builtins.find(*operation);

                  if (builtinsIterator != builtins.end())
                  {
                    (this->*(builtinsIterator->second))(STARTCMD);
                  }
                  else
                  {
                    // terminate with extreme prejudice
                    endConnection();
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
                  endConnection();
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

            case LISTENER_PG:
            {
              class MessageSocket &msgrcvref =
                    *(class MessageSocket *)msgrcv;

              if (!Pgs.count(msgrcvref.socketStruct.socket))
              {
                if ((msgrcvref.socketStruct.events & EPOLLERR) ||
                    (msgrcvref.socketStruct.events & EPOLLHUP))
                {
                  fprintf(logfile, "\t%s %i hanging it up\n", __FILE__, __LINE__);
                  Pg::pgclosesocket(*this, msgrcvref.socketStruct.socket);
                  break;
                }

                new class Pg(this, msgrcvref.socketStruct.socket);
              }
              else
              {
                Pgs[msgrcvref.socketStruct.socket]->cont();
                //                it->second->cont();
              }
            }
            break;

            default:
              printf("%s %i anomaly listenertype %i\n", __FILE__, __LINE__,
                     ((class MessageSocket *)msgrcv)->socketStruct.listenertype);
          }
        }
        break;

        case TOPIC_LOGINOK:
          // set data members based on msgrcv
          login(OKCMD);
          break;

        case TOPIC_LOGINFAIL:
          // set data members base on msgrcv
          login(NOTOKCMD);
          break;

        case TOPIC_CHANGEPASSWORDOK:
          // set data members based on msgrcv
          changepassword(OKCMD);
          break;

        case TOPIC_CHANGEPASSWORDFAIL:
          // set data members base on msgrcv
          changepassword(NOTOKCMD);
          break;

        case TOPIC_CREATEDOMAINOK:
          // set data members based on msgrcv
          createdomain(OKCMD);
          break;

        case TOPIC_CREATEDOMAINFAIL:
          // set data members base on msgrcv
          createdomain(NOTOKCMD);
          break;

        case TOPIC_CREATEUSEROK:
          // set data members based on msgrcv
          createuser(OKCMD);
          break;

        case TOPIC_CREATEUSERFAIL:
          // set data members base on msgrcv
          createuser(NOTOKCMD);
          break;

        case TOPIC_DELETEUSEROK:
          // set data members based on msgrcv
          deleteuser(OKCMD);
          break;

        case TOPIC_DELETEUSERFAIL:
          // set data members base on msgrcv
          deleteuser(NOTOKCMD);
          break;

        case TOPIC_DELETEDOMAINOK:
          // set data members based on msgrcv
          deletedomain(OKCMD);
          break;

        case TOPIC_DELETEDOMAINFAIL:
          // set data members base on msgrcv
          deletedomain(NOTOKCMD);
          break;

          /* schema */
        case TOPIC_SCHEMAREPLY:
          pendingOperationsIterator = pendingOperations.find(operationid);

          if (pendingOperationsIterator == pendingOperations.end())
          {
            fprintf(logfile, "bad operationid %li %s %i\n", operationid,
                    __FILE__, __LINE__);
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
              fprintf(logfile, "builtincmd %i %s %i\n",
                      operationPtr->schemaData.builtincmd, __FILE__, __LINE__);
              /* this would be weird for flow */
          }

          break;

        case TOPIC_SCHEMAREQUEST:
        {
          class MessageUserSchema &msgref =
                *(class MessageUserSchema *)msgrcv;
          tainstance = msgref.userschemaStruct.instance;

          switch (msgref.userschemaStruct.builtincmd)
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
                      msgref.userschemaStruct.builtincmd, __FILE__, __LINE__);
          }
        }
        break;

        case TOPIC_TRANSACTION:
        {
          class MessageTransaction &msgref =
                *(class MessageTransaction *)msgrcv;

          // need pendingTransactions
          if (Transactions.count(msgref.transactionStruct.transactionid))
          {
            Transactions[msgref.transactionStruct.transactionid]->processTransactionMessage(msgrcv);
          }
          else
          {
            // have to check for a LOCKED message cmd, to bounce back a
            // message to roll it back
            fprintf(logfile, "%s %i transactionid %li\n", __FILE__, __LINE__,
                    msgref.transactionStruct.transactionid);
            fprintf(logfile, "%s %i thismsg %p next ptr, count %p %lu, messageStruct.payloadtype %i pendingcmdid %i entrypoint %i locktype %i\n", __FILE__, __LINE__, msgrcv, Mbox::getPtr(msgref.nextmsg), Mbox::getCount(msgref.nextmsg), msgref.messageStruct.payloadtype, msgref.transactionStruct.transaction_pendingcmdid, msgref.transactionStruct.transaction_tacmdentrypoint, ((class MessageSubtransactionCmd *)msgrcv)->subtransactionStruct.locktype);
            badMessageHandler();
          }

          break;
        }

        case TOPIC_DEADLOCKABORT:
        {
          class MessageDeadlock &msgref = *(class MessageDeadlock *)msgrcv;

          if (Transactions.count(msgref.deadlockStruct.transactionid))
          {
            Transactions[msgref.deadlockStruct.transactionid]->deadlockAbort(msgref);
          }
        }
        break;
#ifdef PROFILE

        case TOPIC_PROFILE:
        {
          int start = profilecount < PROFILEENTRIES ? 0 :
                      (profilecount+1) % PROFILEENTRIES;
          int stop = profilecount < PROFILEENTRIES ? profilecount :
                     profilecount % PROFILEENTRIES;
          std::stringstream fname;
          fname << "profile/transactionagent_" << instance << ".txt";
          FILE *fp = fopen((char *)fname.str().c_str(), "w");

          for (int n=start; n % PROFILEENTRIES != stop; n++)
          {
            int pos = n % PROFILEENTRIES;
            std::stringstream outstream;
            outstream << instance << "\t" << profiles[pos].accountid << "\t" <<
                      profiles[pos].rid << "\t";

            for (int m=0; m < 4; m++)
            {
              outstream << profiles[pos].points[m].tag << ",";
            }

            outstream << profiles[pos].points[4].tag << "\t";

            for (int m=0; m<4; m++)
            {
              outstream << profiles[pos].points[m].tv.tv_sec * 1000000 +
                        profiles[pos].points[m].tv.tv_usec << ",";
            }

            outstream << profiles[pos].points[4].tv.tv_sec * 1000000 +
                      profiles[pos].points[4].tv.tv_usec << "\t";

            for (int m=0; m < profiles[pos].transactionpointcount-1; m++)
            {
              outstream << profiles[pos].transactionpoints[m].tag << ",";
            }

            if (profiles[pos].transactionpointcount)
            {
              outstream << profiles[pos].transactionpoints[profiles[pos].
                        transactionpointcount-1].tag;
            }

            outstream << "\t";

            for (int m=0; m < profiles[pos].transactionpointcount-1; m++)
            {
              outstream << profiles[pos].transactionpoints[m].tv.tv_sec *
                        1000000 + profiles[pos].transactionpoints[m].tv.tv_usec <<
                        ",";
            }

            if (profiles[pos].transactionpointcount)
            {
              outstream << profiles[pos].transactionpoints[profiles[pos].
                        transactionpointcount-1].tv.tv_sec * 1000000 +
                        profiles[pos].transactionpoints[profiles[pos].
                            transactionpointcount-1].tv.tv_usec;
            }

            outstream << "\t" << profiles[pos].transactionid << "\t";
            boost::unordered_map<int64_t, int64_t>::iterator pIt;

            for (pIt = profiles[pos].engineToSubTransactionids->begin();
                 pIt != profiles[pos].engineToSubTransactionids->end();
                 pIt++)
            {
              if (pIt != profiles[pos].engineToSubTransactionids->begin())
              {
                outstream << ",";
              }

              outstream << pIt->first << " " << pIt->second;
            }

            outstream << std::endl;
            fputs(outstream.str().c_str(), fp);
          }

          fclose(fp);
          mboxes.topologyMgr->sendMsg(msgsnd, true);

          while (1)
          {
            sleep(10);
          }
        }
        break;
#endif

        case TOPIC_TOPOLOGY:
          mboxes.update(myTopology, instance);
          updateReplicas();
          break;

        case TOPIC_ACKDISPATCH:
        {
          class MessageAckDispatch &msgref =
                *(class MessageAckDispatch *)msgrcv;

          // need pendingTransactions
          if (Transactions.count(msgref.ackdispatchStruct.transactionid))
        {
            // for now 4/5/13 don't think about msgref.status
            Transactions[msgref.ackdispatchStruct.transactionid]->continueCommitTransaction(1);
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
          class MessageAckApply &msgref = *(class MessageAckApply *)msgrcv;

          if (Appliers.count(msgref.ackapplyStruct.applierid))
          {
            Appliers[msgref.ackapplyStruct.applierid]->ackedApply(msgref);
          }
          else
          {
            printf("%s %i no Applier to ack status %i %li,%li,%li\n", __FILE__,
                   __LINE__, msgref.ackapplyStruct.status, msgref.ackapplyStruct.subtransactionid,
                   msgref.ackapplyStruct.applierid, msgref.ackapplyStruct.partitionid);
          }
        }
        break;

        case TOPIC_OPERATION:
        {
          operationMap::iterator it;
          it = pendingOperations.find(((class
                                        MessageUserSchema *)msgrcv)->userschemaStruct.operationid);

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
          class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;
          domainidsToSchemata[msgrcvref.userschemaStruct.domainid]->tableNameToId[msgrcvref.argstring] =
            msgrcvref.userschemaStruct.tableid;
        }
        break;

        case TOPIC_FIELDNAME:
        {
          class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;
          domainidsToSchemata[msgrcvref.userschemaStruct.domainid]->fieldNameToId[msgrcvref.userschemaStruct.tableid][msgrcvref.argstring] = msgrcvref.userschemaStruct.fieldid;
        }
        break;

        default:
          fprintf(logfile, "anomaly %i %s %i\n",
                  msgrcv->messageStruct.topic, __FILE__, __LINE__);
      }
    }
  }
}

TransactionAgent::~TransactionAgent()
{
}

void TransactionAgent::endConnection(void)
{
  epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, NULL);
  close(sockfd);
  loggedInUsers.erase(sockfd);
}

int64_t TransactionAgent::readSocket(void)
{
  char inbuf[PAYLOADSIZE];
  ssize_t bytesread = read(sockfd, inbuf, PAYLOADSIZE);

  if (bytesread < 8)
  {
    return -1;
  }

  uint64_t a;
  memcpy(&a, inbuf, sizeof(a));
  int64_t msgsize = be64toh(a);

  if (bytesread-8 != msgsize)
  {
    printf("%s %i bytesread %li msgsize %li sockfd %i\n", __FILE__, __LINE__, bytesread, msgsize, sockfd);
    return -2;
  }

  char operationlength = inbuf[8];

  if ((int)operationlength >= msgsize-1)
  {
    return -3;
  }

  int64_t localargsize = msgsize - 1 - (int)operationlength;
  operation->assign(inbuf+9, (int)operationlength);
  memset(&args, 0, PAYLOADSIZE);
  memcpy(args, inbuf+9+(int)operationlength, localargsize);

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

void TransactionAgent::login(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      operationPtr = new class Operation(OP_AUTH, this, -1, -1);
      operationid = operationPtr->getid();

      vector<string> v;
      msgpack2Vector(&v, args, argsize);
      operationPtr->setDomainName(v[0]);
      class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_LOGIN);
      class MessageUserSchema &msgref = *msg;
      msgref.messageStruct.topic = TOPIC_LOGIN;
      msgref.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msgref.userschemaStruct.argsize = argsize;
      msgref.userschemaStruct.instance = instance;
      msgref.userschemaStruct.operationid = operationid;
      msgref.argstring.assign(args, 0, argsize);
      mboxes.toUserSchemaMgr(this->myIdentity.address, msgref);
    }
    break;

    case OKCMD:
    {
      // let's hope I remember to populate the object's domainid & userid before
      // calling this
      class MessageUserSchema &msgrcvref =
            *(class MessageUserSchema *)msgrcv;
      operationPtr = pendingOperations[operationid];
      authInfo aInfo;
      aInfo.domainid = msgrcvref.userschemaStruct.domainid;
      aInfo.userid = msgrcvref.userschemaStruct.userid;
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
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_CHANGEPASSWORD);
      class MessageUserSchema &msgref = *msg;
      msgref.messageStruct.topic = TOPIC_CHANGEPASSWORD;
      msgref.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msgref.userschemaStruct.argsize = argsize;
      msgref.userschemaStruct.instance = instance;
      msgref.userschemaStruct.operationid = operationid;
      msgref.userschemaStruct.domainid = domainid;
      msgref.userschemaStruct.userid = userid;
      msgref.argstring.assign(args, 0, argsize);
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
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_CREATEDOMAIN);
      class MessageUserSchema &msgref = *msg;
      msgref.messageStruct.topic = TOPIC_CREATEDOMAIN;
      msgref.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msgref.userschemaStruct.argsize = argsize;
      msgref.userschemaStruct.instance = instance;
      msgref.userschemaStruct.operationid = operationid;
      msgref.userschemaStruct.domainid = domainid;
      msgref.userschemaStruct.userid = userid;
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
      rv.push_back(boost::lexical_cast<string>(msgref.userschemaStruct.domainid));
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
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_CREATEUSER);
      class MessageUserSchema &msgref = *msg;
      msgref.messageStruct.topic = TOPIC_CREATEUSER;
      msgref.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msgref.userschemaStruct.argsize = argsize;
      msgref.userschemaStruct.instance = instance;
      msgref.userschemaStruct.operationid = operationid;
      msgref.userschemaStruct.domainid = domainid;
      msgref.userschemaStruct.userid = userid;
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
      rv.push_back(boost::lexical_cast<string>(msgref.userschemaStruct.userid));
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
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_DELETEUSER);
      class MessageUserSchema &msgref = *msg;
      msgref.messageStruct.topic = TOPIC_DELETEUSER;
      msgref.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msgref.userschemaStruct.argsize = argsize;
      msgref.userschemaStruct.instance = instance;
      msgref.userschemaStruct.operationid = operationid;
      msgref.userschemaStruct.domainid = domainid;
      msgref.userschemaStruct.userid = userid;
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
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_DELETEDOMAIN);
      class MessageUserSchema &msgref = *msg;
      msgref.messageStruct.topic = TOPIC_DELETEDOMAIN;
      msgref.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msgref.userschemaStruct.argsize = argsize;
      msgref.userschemaStruct.instance = instance;
      msgref.userschemaStruct.operationid = operationid;
      msgref.userschemaStruct.domainid = domainid;
      msgref.userschemaStruct.userid = userid;
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
      msg.messageStruct.topic = TOPIC_TABLENAME;
      msg.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msg.userschemaStruct.domainid = msgrcvref.userschemaStruct.domainid;
      msg.userschemaStruct.tableid = msgrcvref.userschemaStruct.tableid;
      msg.argstring = msgrcvref.argstring;

      mboxes.toAllOfType(ACTOR_TRANSACTIONAGENT, myIdentity.address, msg);
    }
    break;

    case TASENGINESRESPONSECMD:
      responseVector.clear();
      responseVector.push_back(boost::lexical_cast<string>
                               (((class MessageUserSchema *)msgrcv)->userschemaStruct.tableid));
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
      msg.messageStruct.topic = TOPIC_FIELDNAME;
      msg.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msg.userschemaStruct.domainid = msgrcvref.userschemaStruct.domainid;
      msg.userschemaStruct.tableid = msgrcvref.userschemaStruct.tableid;
      msg.userschemaStruct.fieldid = msgrcvref.userschemaStruct.fieldid;
      msg.argstring = msgrcvref.argstring;

      mboxes.toAllOfType(ACTOR_TRANSACTIONAGENT, myIdentity.address, msg);
    }
    break;

    case TASENGINESRESPONSECMD:
      responseVector.clear();
      responseVector.push_back(boost::lexical_cast<string>
                               (((class MessageUserSchema *)msgrcv)->userschemaStruct.fieldid));
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
  TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
  domainProceduresMap domainProcedures;
  int64_t did = ((class MessageUserSchema *)msgrcv)->userschemaStruct.domainid;
  domainidsToProcedures[did] = domainProcedures;
}

void TransactionAgent::TAcreatetable(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;
  status =
    domainidsToSchemata[msgrcvref.userschemaStruct.domainid]->createTable(msgrcvref.userschemaStruct.tableid);
  class MessageUserSchema *msg =
        new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  class MessageUserSchema &msgref = *msg;
  msgref.userschemaStruct.tableid = msgrcvref.userschemaStruct.tableid;
  TransactionAgent::usmReply(this,
                             ((class Message *)msgrcv)->messageStruct.sourceAddr, *msg);
}

void TransactionAgent::TAaddcolumn(void)
{
  class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;
  class Schema *schemaPtr = domainidsToSchemata[msgrcvref.userschemaStruct.domainid];
  class Table *tablePtr = schemaPtr->tables[msgrcvref.userschemaStruct.tableid];
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  class MessageUserSchema &msgref = *msg;
  msgref.userschemaStruct.fieldid = tablePtr->addfield((fieldtype_e) msgrcvref.userschemaStruct.fieldtype,
                                      msgrcvref.userschemaStruct.fieldlen, msgrcvref.argstring, (indextype_e) msgrcvref.userschemaStruct.indextype);
  status = BUILTIN_STATUS_OK;
  TransactionAgent::usmReply(this,
                             ((class Message *)msgrcv)->messageStruct.sourceAddr, *msg);
}

void TransactionAgent::TAdeleteindex(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  status = BUILTIN_STATUS_OK;
  TransactionAgent::usmReply(this,
                             msgrcv->messageStruct.sourceAddr, *msg);
}

void TransactionAgent::TAdeletetable(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  status = BUILTIN_STATUS_OK;
  TransactionAgent::usmReply(this,
                             msgrcv->messageStruct.sourceAddr, *msg);
}

void TransactionAgent::TAdeleteschema(void)
{
  // either succeeds or fails :-)
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
  status = BUILTIN_STATUS_OK;
  TransactionAgent::usmReply(this,
                             msgrcv->messageStruct.sourceAddr, *msg);
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
      msgref.messageStruct.topic = TOPIC_SCHEMAREQUEST;
      msgref.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msgref.userschemaStruct.builtincmd = builtin;
      msgref.userschemaStruct.argsize = argsize;
      msgref.userschemaStruct.instance = instance;
      msgref.userschemaStruct.operationid = operationid;
      msgref.userschemaStruct.userid = userid;
      msgref.userschemaStruct.domainid = domainid;
      msgref.argstring.assign(args, 0, argsize);
      mboxes.toUserSchemaMgr(this->myIdentity.address, msgref);
    }
    break;

    case USMRESPONSECMD:
  {
      class MessageUserSchema &msgrcvref =
            *(class MessageUserSchema *)msgrcv;

      if (msgrcvref.userschemaStruct.status != BUILTIN_STATUS_OK)   // abort
      {
        responseVector.clear();
        sendResponse(false, STATUS_NOTOK, &responseVector);
        endOperation();
        return;
      }

      class MessageUserSchema msg(TOPIC_SCHEMAREQUEST);
      msg.messageStruct.topic = TOPIC_SCHEMAREQUEST;
      msg.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msg.userschemaStruct.tableid = msgrcvref.userschemaStruct.tableid;
      msg.userschemaStruct.builtincmd = builtin;
      msg.userschemaStruct.instance = instance;
      msg.userschemaStruct.operationid = operationid;
      msg.userschemaStruct.domainid = domainid;
      msg.userschemaStruct.fieldtype = msgrcvref.userschemaStruct.fieldtype;
      msg.userschemaStruct.fieldlen = msgrcvref.userschemaStruct.fieldlen;
      if (msgrcvref.userschemaStruct.argsize)
      {
        msg.userschemaStruct.argsize = msgrcvref.userschemaStruct.argsize;
        msg.argstring.assign(args, 0, argsize);
      }
      else
      {
        msg.argstring=msgrcvref.argstring;
      }

      msg.userschemaStruct.indextype = msgrcvref.userschemaStruct.indextype;
      msg.userschemaStruct.indexid = msgrcvref.userschemaStruct.indexid;
      msg.userschemaStruct.tableindexid = msgrcvref.userschemaStruct.tableindexid;
      msg.userschemaStruct.simple = msgrcvref.userschemaStruct.simple;
      msg.userschemaStruct.fieldid = msgrcvref.userschemaStruct.fieldid;
      msg.userschemaStruct.numfields = msgrcvref.userschemaStruct.numfields;

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

      if (msgrcvref.userschemaStruct.status != BUILTIN_STATUS_OK)
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
  msg.messageStruct.topic = TOPIC_COMPILE;
  msg.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
  msg.userschemaStruct.domainid = domainid;
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
      msg.messageStruct.topic = TOPIC_PROCEDURE1;
      msg.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msg.userschemaStruct.domainid = domainid;
      msg.pathname = resultVector[0];
      msg.procname = storedprocprefix;
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
            mboxes.toActor(myIdentity.address, {(int16_t)n, (int16_t)m}, *nmsg);

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
        printf("%s %i anomaly nodeid %i instance %li error %s\n", __FILE__, __LINE__, myTopology.nodeid, myIdentity.instance, dlsym_error);
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
      msg.messageStruct.topic = TOPIC_PROCEDURE2;
      msg.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
      msg.userschemaStruct.domainid = inmsg.userschemaStruct.domainid;
      msg.procs.procedurecreator = (void *)call_func1create;
      msg.procs.proceduredestroyer = (void *)call_func1destroy;
      msg.userschemaStruct.intdata = inmsg.procname.length();
      msg.argstring.assign(inmsg.procname, 0, msg.userschemaStruct.intdata);

      for (size_t n=0; n < myTopology.allActors[myTopology.nodeid].size(); n++)
      {
        if (myTopology.allActors[myTopology.nodeid][n]==ACTOR_TRANSACTIONAGENT)
        {
          class MessageUserSchema *nmsg = new class MessageUserSchema;
          *nmsg = msg;
          mboxes.toActor(myIdentity.address, {myTopology.nodeid, (int16_t)n},
                         *nmsg);
        }
      }
    }
    break;

    case 3:
    {
      class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;
      domainidsToProcedures[msgrcvref.userschemaStruct.domainid][msgrcvref.argstring] =
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
  domainid = msgrcvref.dispatchStruct.domainid;
  class MessageAckDispatch *msg =
        new class MessageAckDispatch(msgrcvref.dispatchStruct.transactionid, STATUS_OK);
  mboxes.toActor(myIdentity.address, msgrcvref.messageStruct.sourceAddr, *msg);

  int64_t partitioncount=0;
  boost::unordered_map<int64_t, class MessageApply *> msgs;
class Applier *applierPtr = new class Applier(this, domainid,
            msgrcvref.messageStruct.sourceAddr, partitioncount);

  boost::unordered_map< int64_t, vector<MessageDispatch::record_s> >::iterator it;

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

          for (uint16_t f=0; f < tableRef.fields.size(); f++)
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
                                              (int16_t)myTopology.numpartitions)
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
  class MessageUserSchema &msgrcvref = *(class MessageUserSchema *)msgrcv;

  class Larxer lx((char *)msgrcvref.argstring.c_str(), this,
                      domainidsToSchemata[msgrcvref.userschemaStruct.domainid]);

  if (lx.statementPtr==NULL)
{
    printf("%s %i anomaly\n", __FILE__, __LINE__);
  }

  lx.statementPtr->resolveTableFields();
  statements[msgrcvref.userschemaStruct.domainid][msgrcvref.procname] = *lx.statementPtr;
  delete lx.statementPtr;
}
