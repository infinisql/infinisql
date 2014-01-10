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
 * @file   UserSchemaMgr.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 14:07:10 2013
 * 
 * @brief  Actor which maintains user and domain authentication information
 * and schema definitions. There is only 1 active UserSchemaMgr per cluster.
 */

#ifndef INFINISQLUSERSCHEMAMGR_H
#define INFINISQLUSERSCHEMAMGR_H

#include "gch.h"
#include "Table.h"
#include "TransactionAgent.h"
#include "Actor.h"

/** 
 * @brief user password
 *
 */
typedef struct
{
    std::string clearpassword;
    byte digest[64];
} passwordStruct;

typedef boost::unordered_map<std::string, int64_t> domainNameToDomainIdMap;
typedef boost::unordered_map<std::string, int64_t> userNameToUserIdMap;
typedef boost::unordered_map<int64_t, passwordStruct> userIdToPasswordMap;
typedef boost::unordered_map<int64_t, userNameToUserIdMap *>
    domainIdToUserNamesMap;
typedef boost::unordered_map<int64_t, userIdToPasswordMap *>
    domainIdToUserIdsMap;

// map of domainid->nextuserid
typedef boost::unordered_map<int64_t, int64_t> domainIdToNextUserIdMap;

/** 
 * @brief execute User Schema Manager actor
 *
 * @param myIdentityArg how to identify this 
 */
class UserSchemaMgr : public Actor
{
public:
    UserSchemaMgr(Topology::actorIdentity *myIdentityArg);
    virtual ~UserSchemaMgr();

    // pubic for replyTa:
    class Message *msgsnd;
    int64_t operationid;
    int64_t domainid;
    int64_t userid;
    int64_t status;
    int64_t tainstance;
    //public for createSchema:
//    class Message *msgrcv;
//    REUSEMESSAGES
    domainidToSchemaMap domainidsToSchemata;
//    class Mboxes mboxes;
//    Topology::actorIdentity myIdentity;

private:
//    class Topology myTopology;
    /** 
     * @brief generate unique, ever-increasing domain identifier
     *
     *
     * @return next domainid
     */
    int64_t getnextdomainid();
    /** 
     * @brief generate unique, ever-increasing user identifier
     *
     *
     * @return next userid
     */
    int64_t getnextuserid(int64_t argdomainid);
    /** 
     * @brief perform operation (currently only login for SQL)
     *
     * @param msgrcvref received MessageUserSchema
     */
    void operationHandler(class MessageUserSchema &msgrcvref);
    /** 
     * @brief login for SQL session
     *
     * @param username user name
     * @param domainname domain name (PostgreSQL dbname)
     * @param password password (clear)
     * @param uid userid
     * @param did domainid
     *
     * @return status
     */
    int64_t operationLogin(string &username, string &domainname,
                           string &password, int64_t *uid, int64_t *did);

    /** 
     * @brief login (raw interface)
     *
     */
    void login();
    /** 
     * @brief change password
     *
     */
    void changepassword();
    /** 
     * @brief create domain
     *
     */
    void createdomain();
    /** 
     * @brief create user
     *
     */
    void createuser();
    /** 
     * @brief delete user
     *
     */
    void deleteuser();
    /** 
     * @brief delete domain
     *
     */
    void deletedomain();
    /** 
     * @brief create schema
     *
     * @param cmd entry point for continuation
     */
    void createschema(builtincmds_e cmd);
    /** 
     * @brief create table
     *
     * @param cmd entry point for continuation
     */
    void createtable(builtincmds_e cmd);
    /** 
     * @brief add column
     *
     * @param cmd entry point for continuation
     */
    void addcolumn(builtincmds_e cmd);
    /** 
     * @brief delete index
     *
     * @param cmd entry point for continuation
     */
    void deleteindex(builtincmds_e cmd);
    /** 
     * @brief delete table
     *
     * @param cmd entry point for continuation
     */
    void deletetable(builtincmds_e cmd);
    /** 
     * @brief delete schema
     *
     * @param cmd entry point for continuation
     */
    void deleteschema(builtincmds_e cmd);

    //private:
    int64_t argsize;
    int64_t instance;
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
    std::vector<std::string> *resultVector;
    int64_t nextdomainid;
    domainidToSchemaMap::iterator domainidsToSchemataIterator;
    boost::unordered_map <std::string, fieldtype_e> fieldTypeMap;
    boost::unordered_map <std::string, indextype_e> indexTypeMap;
};

/** 
 * @brief launch User Schema Manager actor
 *
 * @param identity how to identify actor instance
 *
 * @return 
 */
void *userSchemaMgr(void *identity);

#endif  /* INFINISQLUSERSCHEMAMGR_H */
