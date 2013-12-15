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

#ifndef INFINISQLPG_H
#define INFINISQLPG_H

#include "infinisql_gch.h"
#include "infinisql_api.h"

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

    Pg(class TransactionAgent *, int);
    virtual ~Pg();

    /* for the ApiInterface base class */
    void doit(void)
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
    void continuePgFunc(int64_t, void *);
    void continuePgCommitimplicit(int64_t, void *);
    void continuePgCommitexplicit(int64_t, void *);
    void continuePgRollbackimplicit(int64_t, void *);
    void continuePgRollbackexplicit(int64_t, void *);
    /* end ApiInterface base class functions */

    bool readsocket(string &);
    void closesocket(class TransactionAgent &);
    static void pgclosesocket(class TransactionAgent &, int);
    void cont();
    short initcmd(string &);
    void replymsg();
    short writesocket();
    short rewritesocket();

    bool get(int16_t *);
    bool get(int32_t *);
    bool get(int64_t *);
    bool get(vector<int16_t> &, size_t);
    bool get(vector<int32_t> &, size_t);
    bool get(vector<int64_t> &, size_t);
    bool get(char *);
    bool get(string &, size_t);
    bool get(string &);
    void put(int16_t);
    void put(int32_t);
    void put(int64_t);
    void put(vector<int16_t> &);
    void put(vector<int32_t> &);
    void put(vector<int64_t> &);
    void put(char);
    void put(char *, size_t);
    void put(string &);
    void put(char *); // string literal
    void putCommandComplete(char *);
    void putErrorResponse(char *, char *, char *);
    void putNoticeResponse(char *, char *, char *);
    void putRowDescription();
    void putDataRows();
    void putAuthenticationOk();
    void putParameterStatus(char *, char *);

    void sqlcommitimplicit();
    void sqlcommitexplicit();
    void sqlrollbackimplicit();
    void sqlrollbackexplicit();
    bool sqlbegin();

    void continueLogin(int, class MessageUserSchema &);
    void executeStatement(string &);
    void errorStatus(int64_t);

    //private:

    states_e state;
    int sockfd;

    char pgcmdtype;
    uint32_t size;
    string inbuf;
    size_t pos;
    char outcmd;
    string outmsg;
    string outbuf;

    int64_t userid;
    //  int64_t domainid;
    class Schema *schemaPtr;
    //  class Statement *statementPtr;

    // startupArgs["user"] and "database" are username & dbname
    boost::unordered_map<string, string> startupArgs;

    string procedureprefix;

    // session settings
    bool session_isautocommit;
    // autocommit a command that started without being in a transaction block
    bool command_autocommit;
    bool isintransactionblock;
};

#endif  /* INFINISQLPG_H */
