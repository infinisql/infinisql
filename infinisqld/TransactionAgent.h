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
 * @file   TransactionAgent.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 14:03:09 2013
 * 
 * @brief  Actor which communicates with clients and executes transactions.
 * Coordinates of activities between several other actors.
 */

#ifndef INFINISQLTRANSACTIONAGENT_H
#define INFINISQLTRANSACTIONAGENT_H

#include "gch.h"
#include "Operation.h"
#include "Table.h"
#include "Schema.h"
#include "Larxer.h"

enum transactionpayload_e
{
    NOPAYLOAD = 0,
    SUBTRANSACTIONCMDPAYLOAD,
    COMMITROLLBACKPAYLOAD
};

enum idlcmd_e
{
    IDLNOCMD = 0,
    IDLROWLOCK,
    IDLINDEXLOCK,
    IDLINSERTROW,
    IDLINDEX,
    IDLDELETEUNIQUEINDEX,
    IDLINSERTNONUNIQUEINDEX,
    IDLINSERTNULLINDEX,
    IDLDELETENONUNIQUEINDEX,
    IDLDELETENULLINDEX,
    IDLREPLACEINDEX,
    IDLROLLBACKROW,
    IDLROLLBACKINDEX,
};

// deletethis this
typedef struct
{
    idlcmd_e cmd;
    bool isrow;

    int64_t rowid;
    int64_t tableid;
    std::string row;
    locktype_e locktype;

    // for index
    int64_t fieldid;
    fieldValue_s fieldVal;
    // for pending locks
    int64_t pendingcmdid;
} idl;
// delete this
typedef vector<idl> cmds;

#include "Transaction.h"
#include "Applier.h"

typedef boost::unordered_map<std::string, procedures_s> domainProceduresMap;

typedef struct
{
    int64_t resultCode;
    msgpack::sbuffer *sbuf;
    std::vector<std::string> *responseVector;
} responseData;

typedef struct
{
    int64_t domainid;
    int64_t userid;
    std::string domainName;
} authInfo;

typedef boost::unordered_map<int, responseData> sendLaterMap;
// there is apparently a bug in boost::unordered_map that causes
// this thing to dump core when trying to count or erase a key under
// some circumstances. no time presently to figure it out
//typedef boost::unordered_map<int, authInfo> socketAuthInfo;
typedef std::map<int, authInfo> socketAuthInfo;
typedef boost::unordered_map<int64_t, class Operation *> operationMap;

msgpack::sbuffer *makeSbuf(msgpack::sbuffer *);
msgpack::sbuffer *makeSbuf(vector<string> *);
msgpack::sbuffer *makeSbuf(std::map<string, string> *);

/** 
 * @brief execute Transaction Agent actor
 *
 * @param myIdentityArg how to identify this
 */
class TransactionAgent
{
public:
    TransactionAgent(Topology::partitionAddress *myIdentityArg);
    virtual ~TransactionAgent();

    /** 
     * @brief update Topology of replicas
     *
     */
    void updateReplicas();

    // pubic for replyTa:
    //  Mbox::msgstruct msgsnd;
    int64_t operationid;
    int64_t domainid;
    int64_t userid;
    int64_t status;
    int64_t tainstance;
    //public for createSchema:
    class Message *msgrcv;
    REUSEMESSAGES
        domainidToSchemaMap domainidsToSchemata;

    // needs to be all/mostly public for stored procedures

    // builtins
    /** 
     * @brief ping operation
     *
     * sends an empty response--used for testing functionality
     *
     * @param cmd not used
     */
    void ping(builtincmds_e cmd);
    /** 
     * @brief login
     *
     * @param cmd continuation entry point
     */
    void login(builtincmds_e cmd);
    /** 
     * @brief logout
     *
     * @param cmd continuation entry point
     */
    void logout(builtincmds_e cmd);
    /** 
     * @brief change password
     *
     * @param cmd continuation entry point
     */
    void changepassword(builtincmds_e cmd);
    /** 
     * @brief create domain
     *
     * @param cmd continuation entry point
     */
    void createdomain(builtincmds_e cmd);
    /** 
     * @brief create user
     *
     * @param cmd continuation entry point
     */
    void createuser(builtincmds_e cmd);
    /** 
     * @brief delete user
     *
     * @param cmd continuation entry point
     */
    void deleteuser(builtincmds_e cmd);
    /** 
     * @brief delete domain
     *
     * @param cmd continuation entry point
     */
    void deletedomain(builtincmds_e cmd);
    /** 
     * @brief create schema
     *
     * @param cmd continuation entry point
     */
    void createschema(builtincmds_e cmd);
    /** 
     * @brief create table
     *
     * @param cmd continuation entry point
     */
    void createtable(builtincmds_e cmd);
    /** 
     * @brief add column
     *
     * @param cmd continuation entry point
     */
    void addcolumn(builtincmds_e cmd);
    /** 
     * @brief delete index
     *
     * @param cmd continuation entry point
     */
    void deleteindex(builtincmds_e cmd);
    /** 
     * @brief delete table
     *
     * @param cmd continuation entry point
     */
    void deletetable(builtincmds_e cmd);
    /** 
     * @brief delete schema
     *
     * @param cmd continuation entry point
     */
    void deleteschema(builtincmds_e cmd);
    /** 
     * @brief load stored procedure
     *
     * @param cmd continuation entry point
     */
    void loadprocedure(builtincmds_e cmd);
    /** 
     * @brief compile SQL statement
     *
     * @param cmd continuation entry point
     */
    void compile(builtincmds_e cmd);
    /** 
     * @brief common code for many operations, such as login, create table, etc
     *
     * @param cmd continuation entry point
     * @param builtin command to work on
     */
    void schemaBoilerplate(builtincmds_e cmd, int builtin);
    // loop-back schema commands
    /** 
     * @brief continuation function for createschema
     *
     */
    void TAcreateschema();
    /** 
     * @brief continuation for createtable
     *
     */
    void TAcreatetable();
    /** 
     * @brief continuation for addcolumn
     *
     */
    void TAaddcolumn();
    /** 
     * @brief continuation for deleteindex
     *
     */
    void TAdeleteindex();
    /** 
     * @brief continuation for deletetable
     *
     */
    void TAdeletetable();
    /** 
     * @brief continuation for deleteschema
     *
     */
    void TAdeleteschema();
    /** 
     * @brief continuation for loadprocedure
     *
     */
    void TAloadprocedure();
    /** 
     * @brief delete Operation, remove from pendingOperations
     *
     */
    void endOperation();
    //private:
    /** 
     * @brief finish TCP connection on raw interface (not SQL interface)
     *
     */
    void endConnection();
    /** 
     * @brief read socket on raw interface
     *
     *
     * @return 
     */
    int64_t readSocket();
    /** 
     * @brief generate unique, constantly increasing Transaction identifier
     *
     *
     * @return next transactionid
     */
    int64_t getnexttransactionid();
    /** 
     * @brief generate unique, constantly increasing Applier identifier
     *
     *
     * @return next applierid
     */
    int64_t getnextapplierid();
    /** 
     * @brief stub for Message variants that can't be handled otherwise
     *
     */
    void badMessageHandler();
    /** 
     * @brief called by loadprocedure
     *
     * communicates back and forth with UserSchemaMgr and all TransactionAgents
     * to load stored procedure
     *
     * @param entrypoint entry point
     */
    void newprocedure(int64_t entrypoint);
    /** 
     * @brief called by compile to compile SQL statement
     *
     */
    void newstatement();

    /** 
     * @brief for replica to apply transaction changes synchronously
     *
     */
    void handledispatch();

    /** 
     * @brief send raw protocol TCP responses to builtins, ping, login, logout,
     * etc.
     *
     * @param resending 
     * @param resultCode 
     * @param response 
     */
    template <typename T>
        void sendResponse(bool resending, int64_t resultCode, T response)
    {
        msgpack::sbuffer *sbuf = makeSbuf(response);
        int64_t totalsize = 2*sizeof(uint64_t) + sbuf->size();
        char payload[PAYLOADSIZE];
        uint64_t x = htobe64((uint64_t)totalsize);
        memcpy(payload, &x, sizeof(x));
        x = htobe64((uint64_t)resultCode);
        memcpy(payload + sizeof(x), &x, sizeof(x));

        memcpy(payload+(2*sizeof(x)), sbuf->data(), sbuf->size());
        ssize_t totalwritten = write(sockfd, payload, totalsize);

        if (totalwritten == totalsize)   // send was successful
        {
            if (resending)
            {
                waitingToSend.erase(sockfd);
            }

            delete sbuf;
            return;
        }

        if (totalwritten == -1)
        {
            if (errno==EAGAIN || errno==EWOULDBLOCK)
            {
                // we wait for a time to send the data
                sendLaterMap::iterator sendLaterIterator;
                sendLaterIterator = waitingToSend.find(sockfd);

                if (sendLaterIterator != waitingToSend.end()) //gratuitous
                {
                    printf("%s %i endConnection\n", __FILE__, __LINE__);
                    endConnection();
                    return;
                }

                if (!resending)
                {
                    responseData resp;
                    resp.resultCode = resultCode;
                    resp.sbuf = sbuf;
                    resp.responseVector = NULL;
                    waitingToSend[sockfd] = resp;
                }

                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLET;
                ev.data.fd = sockfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev))
                {
                    printf("%s %i endConnection\n", __FILE__, __LINE__);
                    endConnection();
                }
            }
            else
            {
                perror("oops");
                endConnection();
            }

            return;
        }
    }

    /** 
     * @brief reply MessageUserSchema to UserSchemaMgr
     *
     * for builtin functions, ping, login, createindex, and so on
     *
     * @param actor sending actor
     * @param dest UserSchemaMgr address
     * @param msg MessageUserSchema
     */
    template < typename T >
        static void usmReply(T actor, Topology::addressStruct &dest,
                             class MessageUserSchema &msg)
    {
        msg.messageStruct.payloadtype = PAYLOADUSERSCHEMA;
        msg.userschemaStruct.operationid = actor->operationid;
        msg.userschemaStruct.domainid = actor->domainid;
        msg.userschemaStruct.userid = actor->userid;
        msg.userschemaStruct.status = actor->status;

        actor->mboxes.toActor(actor->myIdentity.address, dest, msg);
    }

    Topology::partitionAddress myIdentity;
    class Mboxes mboxes;
    class Topology myTopology;
    int64_t instance;
    class Mbox *mymboxPtr;
    int epollfd;
    int sockfd;
    char payload[PAYLOADSIZE];
    std::string *operation;
    socketAuthInfo loggedInUsers;
    int64_t argsize;
    char args[PAYLOADSIZE]; // get rid of this when possible
    std::string argstring;
    sendLaterMap waitingToSend;
    operationMap pendingOperations;
    operationMap::iterator pendingOperationsIterator;
    int64_t operationidcounter; // never touch this!
    std::string domainName;
    std::vector<std::string> responseVector;
    class Operation *operationPtr;
    domainidToSchemaMap::iterator domainidsToSchemataIterator;
    boost::unordered_map<int64_t, domainProceduresMap> domainidsToProcedures;
    boost::unordered_map<int64_t, class Transaction *> Transactions;
    boost::unordered_map<int64_t, class Applier *> Appliers;
    // Pgs[socket] = *Pg
    boost::unordered_map<int, class Pg *> Pgs;
    int64_t nexttransactionid;
    int64_t nextapplierid;
    int batchSendCount;

    size_t myreplica;
    size_t mymember;
    Topology::addressStruct replicaAddress;
    std::vector<Topology::addressStruct> replicaAddresses;

    // statements[domainid][statementname] = compiledstatement
    // user submits statementname
    boost::unordered_map< int64_t,
        boost::unordered_map<std::string, class Statement> > statements;
};

class ApiInterface;
typedef ApiInterface *(*spclasscreate)(class TransactionAgent *,
                                       class ApiInterface *, void *);
typedef void(*spclassdestroy)(ApiInterface *);

/** 
 * @brief orphan
 *
 * @param servent 
 * @param result 
 * @param msg 
 */
template < typename T >
void replyTa(T servent, topic_e result, void *msg)
{
    class MessageUserSchema &msgref = *(class MessageUserSchema *)msg;
    servent->msgsnd.data = (void *)msg;
    ((class Message *)servent->msgsnd.data)->messageStruct.topic = result;
    ((class Message *)servent->msgsnd.data)->messageStruct.payloadtype =
        PAYLOADUSERSCHEMA;
    msgref.userschemaStruct.operationid = servent->operationid;
    msgref.userschemaStruct.domainid = servent->domainid;
    msgref.userschemaStruct.userid = servent->userid;
    msgref.userschemaStruct.status = servent->status;
}

/** 
 * @brief launch Transaction Agent actor
 *
 * @param identity how to identify actor instance
 *
 * @return 
 */
void *transactionAgent(void *identity);

#endif  /* INFINISQLTRANSACTIONAGENT_H */
