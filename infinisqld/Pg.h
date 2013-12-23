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
 * @file   Pg.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:43:16 2013
 * 
 * @brief  Objects created for every SQL login and associated with a specific
 * TransactionAgent. Takes incoming requests, has them processed and
 * responds to client.
 */

#ifndef INFINISQLPG_H
#define INFINISQLPG_H

#include "gch.h"
#include "infinisql.h"

/** 
 * @brief object that handles SQL activities
 *
 * implements PostgreSQL frontend/backend protocol, version 3:
 * http://www.postgresql.org/docs/7.4/static/protocol.html
 *
 * @param entrypoint 
 * @param statePtr 
 *
 * @return 
 */
class Pg : public ApiInterface
{
public:
    enum states_e
    {
        STATE_NONE = 0,
        STATE_BEGIN,
        STATE_AUTH,
        STATE_ESTABLISHED,
        STATE_ABORTED,
        STATE_EXITING
    };

    Pg(class TransactionAgent *entrypoint, int statePtr);
    virtual ~Pg();

    /* for the ApiInterface base class */
    void doit()
    {
        ;
    }
    void continueFunc1(int64_t entrypoint, void *statePtr)
    {
        ;
    }
    void continueFunc2(int64_t entrypoint, void *statePtr)
    {
        ;
    }
    /** 
     * @brief after SQL activity is performed
     * 
     * based on the statement type and transaction state, a variety of things
     * if session_isautocommit==true, and is SELECT, INSERT, UPDATE, DELETE,
     * then output
     * If autocommit==false, and SELECT, INSERT, UPDATE, DELETE, then prepare
     * output but don't output
     * if COMMIT (END), then commit open transaction and output results already
     * prepared
     * if ROLLBACK, then rollback open transaction and output results
     * CommandComplete at the end of everything returned
     * If set, then set whatever
     *
     * @param entrypoint entry point to contiue
     * @param statePtr state data to continue with
     */
    void continuePgFunc(int64_t entrypoint, void *statePtr);
    /** 
     * @brief continuation function after implicit commit
     *
     * implicit commit done after a single statement is entered, not
     * in a transaction already, and session_isautocommit is true
     *
     * @param entrypoint entry point to continue
     * @param statePtr state data to continue with
     */
    void continuePgCommitimplicit(int64_t entrypoint, void *statePtr);
    /** 
     * @brief continuation function after explicit commit
     *
     * explicit commit is sent as COMMIT or END at end of
     * transaction
     *
     * @param entrypoint entry point to continue
     * @param statePtr state data to continue with
     */
    void continuePgCommitexplicit(int64_t entrypoint, void *statePtr);
    /** 
     * @brief continuation function after implicit rollback
     *
     * implicit rollback done generally after a failure of some kind
     * which forces a rollback
     *
     * @param entrypoint entry point to continue
     * @param statePtr state data to continue with
     */
    void continuePgRollbackimplicit(int64_t entrypoint, void *statePtr);
    /** 
     * @brief continuation function after explicit rollback
     *
     * explicit commit is sent as ROLLBACK statement within transaction
     *
     * @param entrypoint entry point to continue
     * @param statePtr state data to continue with
     */
    void continuePgRollbackexplicit(int64_t entrypoint, void *statePtr);
    /* end ApiInterface base class functions */

    /** 
     * @brief read from socket
     *
     * @param buf buffer for data read
     *
     * @return true: something read (or spurious epoll event), false
     * error or EOF
     */
    bool readsocket(string &buf);
    /** 
     * @brief close socket
     *
     * @param taRef TransactionAgent
     */
    void closesocket(class TransactionAgent &taRef);
    /** 
     * @brief more socket closing-related activities
     *
     * @param taRef TransactionAgent
     * @param socketfd socket descriptor
     */
    static void pgclosesocket(class TransactionAgent &taRef, int socketfd);
    /** 
     * @brief wire protocol handling
     * 
     * called by TransactionAgent when incoming socket reads ready
     *
     */
    void cont();
    /** 
     * @brief read new command from socket
     *
     * @param newdata buffer for command string
     *
     * @return -1: bad input, 0: not completely received, -1: complete
     */
    short initcmd(string &newdata);
    /** 
     * @brief put message for reply into outbound buffer
     *
     */
    void replymsg();
    /** 
     * @brief write response messages to socket
     *
     * if not all data can be written, store data until EPOLLOUT
     * event is received
     *
     * @return -1: socket error, 0: send complete, 1: data remaining to send
     */
    short writesocket();
    /** 
     * @brief write stored buffer to socket
     *
     * called after socket becomes writable
     *
     *
     * @return see writesocket
     */
    short rewritesocket();

    /** 
     * @brief read 16bit integer from inbound message string
     *
     * @param val retrieve buffer
     *
     * @return success or failure
     */
    bool get(int16_t *val);
    /** 
     * @brief read 32bit integer from inbound message string
     *
     * @param val retrieve buffer
     *
     * @return success or failure
     */
    bool get(int32_t *val);
    /** 
     * @brief read 64bit integer from inbound message string
     *
     * @param val retrieve buffer
     *
     * @return success or failure
     */
    bool get(int64_t *val);
    /** 
     * @brief read list of 16bit integers from inbound message string
     *
     * @param val retrieve buffer
     * @param nelem number of elements
     *
     * @return success or failure
     */
    bool get(vector<int16_t> &val, size_t nelem);
    /** 
     * @brief read array of 32bit integers from inbound message string
     *
     * @param val retrieve buffer
     * @param nelem number of elements
     *
     * @return success or failure
     */
    bool get(vector<int32_t> &val, size_t nelem);
    /** 
     * @brief read array of 64bit integers inbound message string
     *
     * @param val retrieve buffer
     * @param nelem number of elements
     *
     * @return success or failure
     */
    bool get(vector<int64_t> &val, size_t nelem);
    /** 
     * @brief read Byte1 from inbound message string
     *
     * @param val retrieve buffer
     *
     * @return success or failure
     */
    bool get(char *val);
    /** 
     * @brief read Byten from inbound message string
     *
     * @param val retrieve buffer
     * @param nelem number of bytes (n)
     *
     * @return success or failure
     */
    bool get(string &val, size_t nelem);
    /** 
     * @brief read String (c-style) from inbound message string
     *
     * @param val retrieve buffer
     *
     * @return successor failure
     */
    bool get(string &val);
    /** 
     * @brief write data to outbound message buffer
     *
     * @param val 16bit integer
     */
    void put(int16_t val);
    /** 
     * @brief write data to outbound message buffer
     *
     * @param val 32bit integer
     */
    void put(int32_t val);
    /** 
     * @brief write data to outbound message buffer
     *
     * @param val 64bit integer
     */
    void put(int64_t val);
    /** 
     * @brief write data to outbound message buffer
     *
     * @param val array of 16bit integers
     */
    void put(vector<int16_t> &val);
    /** 
     * @brief write data to outbound message buffer
     *
     * @param val array of 32bit integers
     */
    void put(vector<int32_t> &val);
    /** 
     * @brief write data to outbound message buffer
     *
     * @param val array of 64bit integers
     */
    void put(vector<int64_t> &val);
    /** 
     * @brief write data to outbound message buffer
     *
     * @param val Byte1
     */
    void put(char val);
    /** 
     * @brief write Byten to outbound message buffer
     *
     * @param val bytes
     * @param nelem number of bytes (n)
     */
    void put(char *val, size_t nelem);
    /** 
     * @brief write data to outbound message buffer
     *
     * @param val String (c-style)
     */
    void put(string &val);
    /** 
     * @brief write data to outbound message buffer
     *
     * @param val string literal
     */
    void put(char *val);
    /** 
     * @brief write completed command to outbound message buffer
     *
     * @param tag command name
     */
    void putCommandComplete(char *tag);
    /** 
     * @brief write error to outbound message buffer
     *
     * @param severity error severity
     * @param code error code
     * @param message error message
     */
    void putErrorResponse(char *severity, char *code, char *message);
    /** 
     * @brief write notice to outbound message buffer
     *
     * @param severity notice severity
     * @param code notice code
     * @param message notice message
     */
    void putNoticeResponse(char *severity, char *code, char *message);
    /** 
     * @brief write field names to outbound message buffer
     *
     */
    void putRowDescription();
    /** 
     * @brief write rows to outbound message buffer
     *
     */
    void putDataRows();
    /** 
     * @brief write authenticationok status to outbound message buffer
     *
     */
    void putAuthenticationOk();
    /** 
     * @brief write server parameters in status message to outbound message
     * buffer
     *
     * @param name parameter name
     * @param value parameter value
     */
    void putParameterStatus(char *name, char *value);

    /** 
     * @brief implicit commit
     *
     * implicit commit done after a single statement is entered, not
     * in a transaction already, and session_isautocommit is true
     *
     */
    void sqlcommitimplicit();
    /** 
     * @brief explicit commit
     *
     * explicit commit is sent as COMMIT or END at end of
     * transaction
     *
     */
    void sqlcommitexplicit();
    /** 
     * @brief implicit rollback
     *
     * implicit rollback done generally after a failure of some kind
     * which forces a rollback
     *
     */
    void sqlrollbackimplicit();
    /** 
     * @brief explicit rollback
     *
     * explicit commit is sent as ROLLBACK statement within transaction
     *
     */
    void sqlrollbackexplicit();
    /** 
     * @brief appears to be orphan
     *
     *
     * @return 
     */
    bool sqlbegin();

    /** 
     * @brief continuation after login operation
     *
     * @param cmdstate function state to continue
     * @param msgrcvref reply MessageUserSchema
     */
    void continueLogin(int cmdstate, class MessageUserSchema &msgrcvref);
    /** 
     * @brief tokenize, parse, compile and execute SQL query
     *
     * @param stmtstr query string
     */
    void executeStatement(string &stmtstr);
    /** 
     * @brief output error based on Transaction::resultCode
     *
     * @param status value from Transaction::resultCode
     */
    void errorStatus(int64_t status);

    //private:

    states_e state;
    int sockfd;

    char pgcmdtype;
    uint32_t size;
    std::string inbuf;
    size_t pos;
    char outcmd;
    std::string outmsg;
    std::string outbuf;

    int64_t userid;
    class Schema *schemaPtr;

    // startupArgs["user"] and "database" are username & dbname
    boost::unordered_map<std::string, std::string> startupArgs;

    std::string procedureprefix;

    // session settings
    bool session_isautocommit;
    // autocommit a command that started without being in a transaction block
    bool command_autocommit;
    bool isintransactionblock;
};

#endif  /* INFINISQLPG_H */
