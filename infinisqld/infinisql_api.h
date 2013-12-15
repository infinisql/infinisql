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

#ifndef INFINISQLAPI_H
#define INFINISQLAPI_H

#include "infinisql_gch.h"
#include "infinisql_Transaction.h"

class Field;

class ApiInterface;
typedef void(ApiInterface::*apifPtr)(int64_t, void *);

class ApiInterface
{
public:
    struct results_s
    {
        cmd_e cmdtype;
        class Transaction *transactionPtr;
        int64_t statementStatus;
        boost::unordered_map< uuRecord_s, returnRow_s > statementResults;
        std::vector<fieldtypename_s> selectFields;
        boost::unordered_map< uuRecord_s, std::vector<fieldValue_s> > selectResults;
    };

    ApiInterface()
    {
        ;
    }
    virtual ~ApiInterface()
    {
        ;
    }

    virtual void doit(void) = 0;
    virtual void continueFunc1(int64_t, void *) = 0;
    virtual void continueFunc2(int64_t, void *) = 0;
    virtual void continuePgFunc(int64_t, void *) = 0;
    virtual void continuePgCommitimplicit(int64_t, void *) = 0;
    virtual void continuePgCommitexplicit(int64_t, void *) = 0;
    virtual void continuePgRollbackimplicit(int64_t, void *) = 0;
    virtual void continuePgRollbackexplicit(int64_t, void *) = 0;
    void deserialize2Vector(void);
    void beginTransaction(void);
    void destruct(void);
    void bouncebackproxy(void);
    void insertRow(apifPtr, int64_t, void *, int64_t);
    void deleteRow(apifPtr, int64_t, void *, uuRecord_s &);
    void replaceRow(apifPtr, int64_t, void *);
    // isnull,isnotnull
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e);
    // eq,neq,gt,lt,gte,lte,regex
    void selectRows(apifPtr, int64_t, void *, int64_t,
                    int64_t, locktype_e, operatortypes_e, int64_t);
    void selectRows(apifPtr, int64_t, void *, int64_t,
                    int64_t, locktype_e, operatortypes_e, uint64_t);
    void selectRows(apifPtr, int64_t, void *, int64_t,
                    int64_t, locktype_e, operatortypes_e, bool);
    void selectRows(apifPtr, int64_t, void *, int64_t,
                    int64_t, locktype_e, operatortypes_e, long double);
    void selectRows(apifPtr, int64_t, void *, int64_t,
                    int64_t, locktype_e, operatortypes_e, char);
    void selectRows(apifPtr, int64_t, void *, int64_t,
                    int64_t, locktype_e, operatortypes_e, string *);
    // in
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, vector<int64_t> *);
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, vector<uint64_t> *);
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, vector<bool> *);
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, vector<long double> *);
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, vector<char> *);
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, vector<string> *);
    // between
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, int64_t, int64_t);
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, uint64_t, uint64_t);
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, bool, bool);
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, long double, long double);
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, char, char);
    void selectRows(apifPtr, int64_t, void *, int64_t, int64_t,
                    locktype_e, operatortypes_e, string *, string *);
    void fetchRows(apifPtr, int64_t, void *);
    void unlock(apifPtr, int64_t, void *, int64_t, int64_t, int64_t);
    void rollback(apifPtr, int64_t, void *, uuRecord_s &);
    void commit(apifPtr, int64_t, void *);
    void rollback(apifPtr, int64_t, void *);
    void revert(apifPtr, int64_t, void *, uuRecord_s &);
    void updateRow(apifPtr, int64_t, void *, uuRecord_s &); // calls a map of fields
    // sets the field to null:
    void updateRowNullField(apifPtr, int64_t, void *, uuRecord_s &, int64_t);
    void updateRow(apifPtr, int64_t, void *, uuRecord_s &, int64_t, int64_t);
    void updateRow(apifPtr, int64_t, void *, uuRecord_s &, int64_t, uint64_t);
    void updateRow(apifPtr, int64_t, void *, uuRecord_s &, int64_t, bool);
    void updateRow(apifPtr, int64_t, void *, uuRecord_s &, int64_t, long double);
    void updateRow(apifPtr, int64_t, void *, uuRecord_s &, int64_t, char);
    void updateRow(apifPtr, int64_t, void *, uuRecord_s &, int64_t, string);
    bool unmakerow(int64_t, string *, vector<fieldValue_s> *);
    void prepareResponseVector(int64_t);

    void addFieldToRow(void);
    void addFieldToRow(int64_t);
    void addFieldToRow(uint64_t);
    void addFieldToRow(bool);
    void addFieldToRow(long double);
    void addFieldToRow(char);
    void addFieldToRow(string &);

    void setReEntry(apifPtr, int64_t, void *);
    void sendResponse(int64_t, vector<std::string> *);

    class Statement *newStatement(char *);
    bool execStatement(const char *, vector<std::string> &, apifPtr, int64_t, void *);

    class TransactionAgent *taPtr;
    class ApiInterface *pgPtr;
    class Statement *statementPtr;
    std::vector<std::string> inputVector;
    class Transaction *transactionPtr;
    procedureResponse_s response;
    std::vector<std::string> responseVector;
    void *destroyerPtr;
    apifPtr continueFunc1Ptr;
    apifPtr continueFunc2Ptr;
    apifPtr continuePgFuncPtr;
    int sockfd;
    int64_t domainid;

    results_s results;
};

typedef ApiInterface *(*spclasscreate)(class TransactionAgent *,
                                       class ApiInterface *, void *);
typedef void(*spclassdestroy)(ApiInterface *);

// the types of the class factories
typedef ApiInterface *create_t();
typedef void destroy_t(ApiInterface *);

#endif  /* INFINISQLAPI_H */
