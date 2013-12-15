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

#ifndef USERSCHEMAMGR_H
#define USERSCHEMAMGR_H

#include "infinisql_gch.h"
#include "infinisql_Table.h"
#include "infinisql_TransactionAgent.h"

typedef struct
{
    string clearpassword;
    byte digest[64];
} passwordStruct;

typedef boost::unordered_map<std::string, int64_t> domainNameToDomainIdMap;
typedef boost::unordered_map<string, int64_t> userNameToUserIdMap;
typedef boost::unordered_map<int64_t, passwordStruct> userIdToPasswordMap;
typedef boost::unordered_map<int64_t, userNameToUserIdMap *>
    domainIdToUserNamesMap;
typedef boost::unordered_map<int64_t, userIdToPasswordMap *>
    domainIdToUserIdsMap;

// map of domainid->nextuserid
typedef boost::unordered_map<int64_t, int64_t> domainIdToNextUserIdMap;

class UserSchemaMgr
{
public:
    UserSchemaMgr(Topology::partitionAddress *);
    virtual ~UserSchemaMgr();

    // pubic for replyTa:
    class Message *msgsnd;
    int64_t operationid;
    int64_t domainid;
    int64_t userid;
    int64_t status;
    int64_t tainstance;
    //public for createSchema:
    class Message *msgrcv;
    REUSEMESSAGES
        domainidToSchemaMap domainidsToSchemata;
    class Mboxes mboxes;
    Topology::partitionAddress myIdentity;

private:
    class Topology myTopology;
    int64_t getnextdomainid(void);
    int64_t getnextuserid(int64_t);
    void operationHandler(class MessageUserSchema &);
    int64_t operationLogin(string &, string &, string &, int64_t *, int64_t *);

    void login(void);
    void changepassword(void);
    void createdomain(void);
    void createuser(void);
    void deleteuser(void);
    void deletedomain(void);
    void createschema(builtincmds_e);
    void createtable(builtincmds_e);
    void addcolumn(builtincmds_e);
    void deleteindex(builtincmds_e);
    void deletetable(builtincmds_e);
    void deleteschema(builtincmds_e);

    //private:
    int64_t argsize;
    int64_t instance;
    class Mbox *mymboxPtr;
    domainNameToDomainIdMap domainNameToDomainId;
    domainNameToDomainIdMap::iterator domainNameToDomainIdIterator;
    domainIdToUserNamesMap domainIdToUserNames;
    domainIdToUserNamesMap::iterator domainIdToUserNamesIterator;
    domainIdToUserIdsMap domainIdToUserIds;
    domainIdToUserIdsMap::iterator domainIdToUserIdsIterator;
    domainIdToNextUserIdMap domainIdToNextUserId;
    domainIdToNextUserIdMap::iterator domainIdToNextUserIdIterator;
    userNameToUserIdMap *userNameToUserId;
    userNameToUserIdMap::iterator userNameToUserIdIterator;
    userIdToPasswordMap *userIdToPassword;
    userIdToPasswordMap::iterator userIdToPasswordIterator;
    passwordStruct pwdStruct;
    passwordStruct *pwdStructPtr;
    vector<string> *resultVector;
    int64_t nextdomainid;
    domainidToSchemaMap::iterator domainidsToSchemataIterator;
    boost::unordered_map <string, fieldtype_e> fieldTypeMap;
    boost::unordered_map <string, indextype_e> indexTypeMap;
};

void *userSchemaMgr(void *);

#endif  /* USERSCHEMAMGR_H */
