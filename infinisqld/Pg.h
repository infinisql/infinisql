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
    void continuePgFunc(int64_t entrypoint, void *statePtr);
    void continuePgCommitimplicit(int64_t entrypoint, void *statePtr);
    void continuePgCommitexplicit(int64_t entrypoint, void *statePtr);
    void continuePgRollbackimplicit(int64_t entrypoint, void *statePtr);
    void continuePgRollbackexplicit(int64_t entrypoint, void *statePtr);
    /* end ApiInterface base class functions */

    bool readsocket(string &buf);
    void closesocket(class TransactionAgent &taRef);
    static void pgclosesocket(class TransactionAgent &taRef, int socketfd);
    void cont();
    short initcmd(string &newdata);
    void replymsg();
    short writesocket();
    short rewritesocket();

    bool get(int16_t *val);
    bool get(int32_t *val);
    bool get(int64_t *val);
    bool get(vector<int16_t> &val, size_t nelem);
    bool get(vector<int32_t> &val, size_t nelem);
    bool get(vector<int64_t> &val, size_t nelem);
    bool get(char *val);
    bool get(string &val, size_t nelem);
    bool get(string &val);
    void put(int16_t val);
    void put(int32_t val);
    void put(int64_t val);
    void put(vector<int16_t> &val);
    void put(vector<int32_t> &val);
    void put(vector<int64_t> &val);
    void put(char val);
    void put(char *val, size_t nelem);
    void put(string &val);
    void put(char *val); // string literal
    void putCommandComplete(char *tag);
    void putErrorResponse(char *severity, char *code, char *message);
    void putNoticeResponse(char *severity, char *code, char *message);
    void putRowDescription();
    void putDataRows();
    void putAuthenticationOk();
    void putParameterStatus(char *name, char *value);

    void sqlcommitimplicit();
    void sqlcommitexplicit();
    void sqlrollbackimplicit();
    void sqlrollbackexplicit();
    bool sqlbegin();

    void continueLogin(int cmdstate, class MessageUserSchema &msgrcvref);
    void executeStatement(string &stmtstr);
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
