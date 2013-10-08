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

#include "infinisql_UserSchemaMgr.h"
#line 28 "UserSchemaMgr.cc"

//UserSchemaMgr::UserSchemaMgr(mboxid_t idarg) : id (idarg) {
UserSchemaMgr::UserSchemaMgr(Topology::partitionAddress *myIdentityArg) :
  myIdentity(*myIdentityArg)
{
  delete myIdentityArg;
  mboxes.nodeid = myIdentity.address.nodeid;
  mboxes.update(myTopology);

  // these are incremented before being given out. domainid 1 is _global
  nextdomainid = 1;

  msgsnd = NULL;

  domainNameToDomainId["_global"] = 1;
  domainIdToUserNames[ domainNameToDomainId["_global"] ] =
    new userNameToUserIdMap;
  domainIdToNextUserId[ domainNameToDomainId["_global"] ] = 1;
  domainIdToUserNames[domainNameToDomainId["_global"]]->operator[]("admin") = 1;
  domainIdToUserIds[ domainNameToDomainId["_global"] ] =
    new userIdToPasswordMap;
  SHA512().CalculateDigest(pwdStruct.digest,
                           (const byte *)myIdentity.argstring.c_str(),
                           myIdentity.argstring.size());
  pwdStruct.clearpassword.assign(myIdentity.argstring.c_str(),
                                 myIdentity.argstring.size());
  domainIdToUserIds[ domainNameToDomainId["_global"] ]->operator[](1) =
    pwdStruct;

  fieldTypeMap["int"] = INT;
  fieldTypeMap["uint"] = UINT;
  fieldTypeMap["bool"] = BOOL;
  fieldTypeMap["float"] = FLOAT;
  fieldTypeMap["char"] = CHAR;
  fieldTypeMap["charx"] = CHARX;
  fieldTypeMap["varchar"] = VARCHAR;

  indexTypeMap["none"] = NONE;
  indexTypeMap["unique"] = UNIQUE;
  indexTypeMap["nonunique"] = NONUNIQUE;
  indexTypeMap["unordered"] = UNORDERED;
  indexTypeMap["uniquenotnull"] = UNIQUENOTNULL;
  indexTypeMap["nonuniquenotnull"] = NONUNIQUENOTNULL;
  indexTypeMap["unorderednotnull"] = UNORDEREDNOTNULL;

  while (1)
  {
    do
    {
//      msgrcv = mymbox.receive(1000);
      GETMSG(msgrcv, myIdentity.mbox, 1000)
    }
    while (msgrcv==NULL);

    class MessageUserSchema &msgrcvref =
          *(class MessageUserSchema *)msgrcv;

    if (msgrcvref.messageStruct.topic != TOPIC_TOPOLOGY && msgrcvref.messageStruct.topic != TOPIC_OPERATION
        && msgrcvref.messageStruct.topic != TOPIC_NONE)
  {
      argsize = msgrcvref.userschemaStruct.argsize;
      tainstance = msgrcvref.userschemaStruct.instance;
      operationid = msgrcvref.userschemaStruct.operationid;
      domainid = msgrcvref.userschemaStruct.domainid;
      userid = msgrcvref.userschemaStruct.userid;
      resultVector = new vector<string>;
      msgpack2Vector(resultVector, (char *)msgrcvref.argstring.c_str(), argsize);
    }

    switch (msgrcv->messageStruct.topic)
    {
      case TOPIC_LOGIN:
        login();
        break;

      case TOPIC_CHANGEPASSWORD:
        changepassword();
        break;

      case TOPIC_CREATEDOMAIN:
        createdomain();
        break;

      case TOPIC_CREATEUSER:
        createuser();
        break;

      case TOPIC_DELETEUSER:
        deleteuser();
        break;

      case TOPIC_DELETEDOMAIN:
        deletedomain();
        break;

      case TOPIC_SCHEMAREQUEST:
        switch (msgrcvref.userschemaStruct.builtincmd)
        {
          case BUILTINCREATESCHEMA:
          {
            createschema(STARTCMD);
          }
          break;

          case BUILTINCREATETABLE:
            createtable(STARTCMD);
            break;

          case BUILTINADDCOLUMN:
            addcolumn(STARTCMD);
            break;

          case BUILTINDELETEINDEX:
            deleteindex(STARTCMD);
            break;

          case BUILTINDELETETABLE:
            deletetable(STARTCMD);
            break;

          case BUILTINDELETESCHEMA:
            deleteschema(STARTCMD);
            break;

          default:
            fprintf(logfile, "UserSchemaMgr bad schema builtin %li\n",
                    msgrcvref.userschemaStruct.builtincmd);
        }

        break;

      case TOPIC_TOPOLOGY:
        mboxes.update(myTopology);
        break;

      case TOPIC_OPERATION:
        operationHandler(*((class MessageUserSchema *)msgrcv));
        break;

      default:
        fprintf(logfile, "UserSchemaMgr bad topic %i\n", msgrcv->messageStruct.topic);
    }

    if (msgrcvref.messageStruct.topic != TOPIC_TOPOLOGY && msgrcvref.messageStruct.topic != TOPIC_OPERATION
        && msgrcvref.messageStruct.topic != TOPIC_NONE)
    {
      delete resultVector;
    }
  }
}

UserSchemaMgr::~UserSchemaMgr()
{
}

// launcher
void *userSchemaMgr(void *identity)
{
  UserSchemaMgr((Topology::partitionAddress *)identity);
  return NULL;
}

void UserSchemaMgr::login(void)
{
  // vector: 0,1,2: domainname,username,password
  domainNameToDomainIdIterator = domainNameToDomainId.find(resultVector->at(0));

  if (domainNameToDomainIdIterator == domainNameToDomainId.end())
  {
    class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_LOGINFAIL);
    //    replyTa(this, TOPIC_LOGINFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  int64_t did = domainNameToDomainIdIterator->second;
  // check username against
  domainIdToUserNamesIterator = domainIdToUserNames.find(did);

  if (domainIdToUserNamesIterator == domainIdToUserNames.end())
  {
    class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_LOGINFAIL);
    //    replyTa(this, TOPIC_LOGINFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  userNameToUserId = domainIdToUserNamesIterator->second;
  userNameToUserIdIterator = userNameToUserId->find(resultVector->at(1));

  if (userNameToUserIdIterator == userNameToUserId->end())
  {
    class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_LOGINFAIL);
    //    replyTa(this, TOPIC_LOGINFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  int64_t uid = userNameToUserIdIterator->second;
  userIdToPassword = domainIdToUserIds[did];
  userIdToPasswordIterator = userIdToPassword->find(uid);

  if (userIdToPasswordIterator == userIdToPassword->end())
  {
    class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_LOGINFAIL);
    //    replyTa(this, TOPIC_LOGINFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  pwdStruct = userIdToPasswordIterator->second;
  byte passworddigest[64];
  SHA512().CalculateDigest(passworddigest,
                           (const byte *)resultVector->at(2).c_str(), resultVector->at(2).length());

  if (memcmp(pwdStruct.digest, passworddigest, 64))
  {
    class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_LOGINFAIL);
    //    replyTa(this, TOPIC_LOGINFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  userid=uid;
  domainid=did;
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_LOGINOK);
  msg->domainname=resultVector->at(0);
  TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
}

void UserSchemaMgr::changepassword(void)
{
  domainIdToUserIdsIterator = domainIdToUserIds.find(domainid);

  if (domainIdToUserIdsIterator == domainIdToUserIds.end())
  {
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_CHANGEPASSWORDFAIL);
    //    replyTa(this, TOPIC_CHANGEPASSWORDFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  userIdToPasswordIterator = userIdToPassword->find(userid);

  if (userIdToPasswordIterator == userIdToPassword->end())
{
    class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_CHANGEPASSWORDFAIL);
    //    replyTa(this, TOPIC_CHANGEPASSWORDFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  SHA512().CalculateDigest(userIdToPasswordIterator->second.digest,
                           (const byte *)resultVector->at(0).c_str(), resultVector->at(0).length());
  class MessageUserSchema *msg =
        new class MessageUserSchema(TOPIC_CHANGEPASSWORDOK);
  //  replyTa(this, TOPIC_CHANGEPASSWORDOK, msg);
  TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
}

void UserSchemaMgr::createdomain(void)
{
  // only _global admin user can do this
  if (domainid != 1 || userid != 1)
  {
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_CREATEDOMAINFAIL);
    //    replyTa(this, TOPIC_CREATEDOMAINFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  domainNameToDomainIdIterator = domainNameToDomainId.find(resultVector->at(0));

  if (domainNameToDomainIdIterator != domainNameToDomainId.end())
{
    // domainname exists already
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_CREATEDOMAINFAIL);
    //    replyTa(this, TOPIC_CREATEDOMAINFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  int64_t newdomainid = getnextdomainid();
  // make dname->did & did->usernamemap & did->useridmap & did->nextuserid
  domainNameToDomainId[resultVector->at(0)] = newdomainid;
  domainIdToUserNames[newdomainid] = new userNameToUserIdMap;
  domainIdToUserIds[newdomainid] = new userIdToPasswordMap;
  domainIdToNextUserId[newdomainid] = 0;

  domainid = newdomainid;
class MessageUserSchema *msg =
        new class MessageUserSchema(TOPIC_CREATEDOMAINOK);
  //  replyTa(this, TOPIC_CREATEDOMAINOK, msg);
  TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
}

// must be _global admin
void UserSchemaMgr::createuser(void)
{
  if (domainid != 1 || userid != 1)
  {
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_CREATEUSERFAIL);
    //    replyTa(this, TOPIC_CREATEUSERFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  domainNameToDomainIdIterator = domainNameToDomainId.find(resultVector->at(0));

  if (domainNameToDomainIdIterator == domainNameToDomainId.end())
{
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_CREATEUSERFAIL);
    //    replyTa(this, TOPIC_CREATEUSERFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  int64_t did = domainNameToDomainIdIterator->second;
  domainIdToUserNamesIterator = domainIdToUserNames.find(did);

  if (domainIdToUserNamesIterator == domainIdToUserNames.end())
{
    // no domainid to usernames entry, so it must have been deleted during this
    // session
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_CREATEUSERFAIL);
    //    replyTa(this, TOPIC_CREATEUSERFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  // check for username already
  userNameToUserId = domainIdToUserNames[did];
  userNameToUserIdIterator = userNameToUserId->find(resultVector->at(1));

  if (userNameToUserIdIterator != userNameToUserId->end())
{
    // username already exists
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_CREATEUSERFAIL);
    //    replyTa(this, TOPIC_CREATEUSERFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  /* get next userid, then insert into things
   * domainIdToUserNames, domainIdToUserIds */
  int64_t newuserid = getnextuserid(did);
  userNameToUserId->operator[](resultVector->at(1)) = newuserid;
  userIdToPassword = domainIdToUserIds[did];
  SHA512().CalculateDigest(pwdStruct.digest,
                           (const byte *)resultVector->at(2).c_str(), resultVector->at(2).length());
  userIdToPassword->operator[](newuserid) = pwdStruct;

  domainid = did;
  userid = newuserid;
  class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_CREATEUSEROK);
  //  replyTa(this, TOPIC_CREATEUSEROK, msg);
  TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
}

// must be _global admin, domainname,username
void UserSchemaMgr::deleteuser(void)
{
  if (domainid != 1 || userid != 1)
  {
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_DELETEUSERFAIL);
    //    replyTa(this, TOPIC_DELETEUSERFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  domainNameToDomainIdIterator = domainNameToDomainId.find(resultVector->at(0));

  if (domainNameToDomainIdIterator == domainNameToDomainId.end())
{
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_DELETEUSERFAIL);
    //    replyTa(this, TOPIC_DELETEUSERFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  domainid = domainNameToDomainIdIterator->second;
  domainIdToUserNamesIterator = domainIdToUserNames.find(domainid);

  if (domainIdToUserNamesIterator == domainIdToUserNames.end())
{
    // no domainid to usernames entry, so it must have been deleted during this
    //  session
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_DELETEUSERFAIL);
    //    replyTa(this, TOPIC_DELETEUSERFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  domainIdToUserNamesIterator = domainIdToUserNames.find(domainid);

  if (domainIdToUserNamesIterator == domainIdToUserNames.end())
{
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_DELETEUSERFAIL);
    //    replyTa(this, TOPIC_DELETEUSERFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  userNameToUserId = domainIdToUserNamesIterator->second;
  // check for username already
  userNameToUserId = domainIdToUserNames[domainid];
  userNameToUserIdIterator = userNameToUserId->find(resultVector->at(1));

  if (userNameToUserIdIterator == userNameToUserId->end())
{
    // username doesn't exist
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_DELETEUSERFAIL);
    //    replyTa(this, TOPIC_DELETEUSERFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  userid = userNameToUserId->at(resultVector->at(1));
  userNameToUserId->erase(resultVector->at(1));

  domainIdToUserIdsIterator = domainIdToUserIds.find(domainid);

  if (domainIdToUserIdsIterator == domainIdToUserIds.end())
{
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_DELETEUSERFAIL);
    //    replyTa(this, TOPIC_DELETEUSERFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  userIdToPassword = domainIdToUserIdsIterator->second;
  userIdToPassword->erase(userid);

class MessageUserSchema *msg =
        new class MessageUserSchema(TOPIC_DELETEUSEROK);
  //  replyTa(this, TOPIC_DELETEUSEROK, msg);
  TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
}

void UserSchemaMgr::deletedomain(void)
{
  // only _global admin can do this
  if (domainid != 1 || userid != 1)
  {
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_DELETEDOMAINFAIL);
    //    replyTa(this, TOPIC_DELETEDOMAINFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  /* delete maps, always return ok if name exists. 3 user maps & domain entry
   * userIdToPassword,userNameToUserId
   * erase from: domainIdToUserIds, domainIdToUserNames, domainNameToDomainId
   * domainIdToNextUserId
   * just need the domainid (by way of the domain name
   */
  domainNameToDomainIdIterator = domainNameToDomainId.find(resultVector->at(0));

  if (domainNameToDomainIdIterator == domainNameToDomainId.end())
{
    class MessageUserSchema *msg =
          new class MessageUserSchema(TOPIC_DELETEDOMAINFAIL);
    //    replyTa(this, TOPIC_DELETEDOMAINFAIL, msg);
    TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    return;
  }

  domainid = domainNameToDomainIdIterator->second;
  // assuming that deleting map that isn't there is ok, so no need to check
  delete domainIdToUserNames[domainid];
  delete domainIdToUserIds[domainid];
  domainIdToUserNames.erase(domainid);
  domainIdToUserIds.erase(domainid);
  domainIdToNextUserId.erase(domainid);
  domainNameToDomainId.erase(resultVector->at(0));

class MessageUserSchema *msg =
        new class MessageUserSchema(TOPIC_DELETEDOMAINOK);
  //  replyTa(this, TOPIC_DELETEDOMAINOK, msg);
  TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
}

// builtins for schema
void UserSchemaMgr::createschema(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      createSchema(this);
      class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
      TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
      break;
    }

    case ABORTCMD:
      break;

    default:
      fprintf(logfile, "no such cmd %i %s %i\n", cmd, __FILE__, __LINE__);
  }
}

void UserSchemaMgr::createtable(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      class Schema *schemaPtr = NULL;
      int64_t tid = 0;

      if (domainidsToSchemata.count(domainid))
      {
        schemaPtr = domainidsToSchemata[domainid];

        // something to check table name exists:
        if (schemaPtr->tableNameToId.count(resultVector->at(0)))
        {
          //table name already exists
          status = BUILTIN_STATUS_NOTOK;
        }
        else
        {
          tid = schemaPtr->getnexttableid();
          status = schemaPtr->createTable(tid);
        }
      }
      else
      {
        status = BUILTIN_STATUS_NOTOK;
      }

      /* either succeeds or fails :-) */
      // table id & name mapping
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_SCHEMAREPLY);
      class MessageUserSchema &msgref = *msg;

      if (status==BUILTIN_STATUS_OK)
    {
        schemaPtr->tableNameToId[resultVector->at(0)] = tid;
        msgref.userschemaStruct.tableid = tid;

        msgref.userschemaStruct.domainid = domainid;
        msgref.argstring = resultVector->at(0);
      }

      //      replyTa(this, TOPIC_SCHEMAREPLY, msg);
      TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    }
    break;

    case ABORTCMD:
      break;

    default:
      fprintf(logfile, "no such cmd %i %s %i\n", cmd, __FILE__, __LINE__);
  }
}

void UserSchemaMgr::addcolumn(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      /* either succeeds or fails :-) */
      // uses domainid, input tableid
      int64_t tid = boost::lexical_cast<int64_t>(resultVector->at(0));
      string stringtype = resultVector->at(1);
      int64_t len = boost::lexical_cast<int64_t>(resultVector->at(2));
      string name = resultVector->at(3);
      string stringidxtype = resultVector->at(4);
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_SCHEMAREPLY);
      class MessageUserSchema &msgref = *msg;

      if (!fieldTypeMap.count(stringtype))
    {
        // map string not found, doh!
        status = BUILTIN_STATUS_NOTOK;
        //        replyTa(this, TOPIC_SCHEMAREPLY, msg);
        TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
        return;
      }

      fieldtype_e type = fieldTypeMap[stringtype];

      if (type != CHARX)
      {
        len = 0; // zero out the length unless it is charx
      }

      if (!indexTypeMap.count(stringidxtype))
      {
        status = BUILTIN_STATUS_NOTOK;
        TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
        return;
      }

      indextype_e idxtype = indexTypeMap[stringidxtype];

      if (!domainidsToSchemata.count(domainid))
      {
        status = BUILTIN_STATUS_NOTOK;
        TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
        return;
      }

      class Schema *schemaPtr = domainidsToSchemata[domainid];

      if (!schemaPtr->tables.count(tid))
      {
        status = BUILTIN_STATUS_NOTOK;
        //        replyTa(this, TOPIC_SCHEMAREPLY, msg);
        TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
        return;
      }

      class Table *tablePtr = schemaPtr->tables[tid];

      if (tablePtr->columnaNameToFieldMap.count(name))
      {
        // column already exists
        status = BUILTIN_STATUS_NOTOK;
        //        replyTa(this, TOPIC_SCHEMAREPLY, msg);
        TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
        return;
      }

      msgref.userschemaStruct.fieldid = tablePtr->addfield(type, len, name, idxtype);
      schemaPtr->fieldNameToId[tid][name] = msgref.userschemaStruct.fieldid;
      status = BUILTIN_STATUS_OK;
      msgref.userschemaStruct.tableid = tid;
      msgref.userschemaStruct.fieldlen = len;
      msgref.userschemaStruct.fieldtype = type;
      msgref.userschemaStruct.indextype = idxtype;

      msgref.userschemaStruct.domainid = domainid;
      msgref.argstring = name;
      msgref.userschemaStruct.argsize = 0;

      TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    }
    break;

    case ABORTCMD:
      break;

    default:
      fprintf(logfile, "no such cmd %i %s %i\n", cmd, __FILE__, __LINE__);
  }
}

void UserSchemaMgr::deleteindex(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      /* either succeeds or fails :-) */
      status = BUILTIN_STATUS_OK;
      class MessageUserSchema *msg = new class MessageUserSchema(TOPIC_SCHEMAREPLY);
      //      replyTa(this, TOPIC_SCHEMAREPLY, msg);
      TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    }
    break;

    case ABORTCMD:
      break;

    default:
      fprintf(logfile, "no such cmd %i %s %i\n", cmd, __FILE__, __LINE__);
  }
}

void UserSchemaMgr::deletetable(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      /* either succeeds or fails :-) */
      status = BUILTIN_STATUS_OK;
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_SCHEMAREPLY);
      //      replyTa(this, TOPIC_SCHEMAREPLY, msg);
      TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    }
    break;

    case ABORTCMD:
      break;

    default:
      fprintf(logfile, "no such cmd %i %s %i\n", cmd, __FILE__, __LINE__);
  }
}

void UserSchemaMgr::deleteschema(builtincmds_e cmd)
{
  switch (cmd)
  {
    case STARTCMD:
    {
      /* either succeeds or fails :-) */
      status = BUILTIN_STATUS_OK;
      class MessageUserSchema *msg =
            new class MessageUserSchema(TOPIC_SCHEMAREPLY);
      //      replyTa(this, TOPIC_SCHEMAREPLY, msg);
      TransactionAgent::usmReply(this, msgrcv->messageStruct.sourceAddr, *msg);
    }
    break;

    case ABORTCMD:
      break;

    default:
      fprintf(logfile, "no such cmd %i %s %i\n", cmd, __FILE__, __LINE__);
  }
}

int64_t UserSchemaMgr::getnextdomainid(void)
{
  return ++nextdomainid;
}

int64_t UserSchemaMgr::getnextuserid(int64_t argdomainid)
{
  domainIdToNextUserIdIterator = domainIdToNextUserId.find(argdomainid);

  if (domainIdToNextUserIdIterator == domainIdToNextUserId.end())
  {
    return -1;
  }

  domainIdToNextUserIdIterator->second =
    domainIdToNextUserIdIterator->second + 1;
  return domainIdToNextUserIdIterator->second;
}

void UserSchemaMgr::operationHandler(class MessageUserSchema &msgrcvref)
{
  class MessageUserSchema *replyMsg =
        new class MessageUserSchema(TOPIC_OPERATION);
  class MessageUserSchema &replyMsgRef = *replyMsg;

  switch ((operationtype_e)msgrcvref.userschemaStruct.operationtype)
{
    case OPERATION_LOGIN:
      replyMsgRef.userschemaStruct.status = operationLogin(msgrcvref.username,
                                          msgrcvref.domainname, msgrcvref.password, &replyMsgRef.userschemaStruct.userid,
                                          &replyMsgRef.userschemaStruct.domainid);
      replyMsgRef.domainname=msgrcvref.domainname;
      break;

    default:
      printf("%s %i anomaly operationtype %i\n", __FILE__, __LINE__,
             msgrcvref.userschemaStruct.operationtype);
  }

  replyMsgRef.userschemaStruct.operationid = msgrcvref.userschemaStruct.operationid;
  replyMsgRef.userschemaStruct.caller = msgrcvref.userschemaStruct.caller;
  replyMsgRef.userschemaStruct.callerstate = msgrcvref.userschemaStruct.callerstate;
  mboxes.toActor(myIdentity.address, msgrcvref.messageStruct.sourceAddr, replyMsgRef);
}

int64_t UserSchemaMgr::operationLogin(string &username, string &domainname,
                                      string &password, int64_t *uid, int64_t *did)
{
  boost::unordered_map<std::string, int64_t>::iterator itDname2Did;
  boost::unordered_map<int64_t, userNameToUserIdMap *>::iterator itDid2Unames;
  boost::unordered_map<string, int64_t>::iterator itUname2Uid;
  boost::unordered_map<int64_t, passwordStruct>::iterator itUid2Password;

  itDname2Did = domainNameToDomainId.find(domainname);

  if (itDname2Did == domainNameToDomainId.end())
  {
    return STATUS_NOTOK;
  }

  *did = itDname2Did->second;

  itDid2Unames = domainIdToUserNames.find(*did);

  if (itDid2Unames == domainIdToUserNames.end())
  {
    return STATUS_NOTOK;
  }

  userNameToUserIdMap &uname2Uid = *itDid2Unames->second;

  itUname2Uid = uname2Uid.find(username);

  if (itUname2Uid == uname2Uid.end())
  {
    return STATUS_NOTOK;
  }

  *uid = itUname2Uid->second;

  userIdToPasswordMap &uid2Password = *domainIdToUserIds[*did];
  itUid2Password = uid2Password.find(*uid);

  if (itUid2Password == uid2Password.end())
  {
    return STATUS_NOTOK;
  }

  passwordStruct pwd = itUid2Password->second;

  byte passworddigest[64];
  SHA512().CalculateDigest(passworddigest, (const byte *)password.c_str(),
                           password.length());

  if (memcmp(pwdStruct.digest, passworddigest, 64))
  {
    return STATUS_NOTOK;
  }

  // return vals did & uid have already been set
  return STATUS_OK;
}
