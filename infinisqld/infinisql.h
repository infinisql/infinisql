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

//#include "infinisql_gch.h"
//#include "infinisql_Transaction.h"
#include <stdint.h>
#include <boost/unordered_map.hpp>
#include <vector>
#include <string>
#include <msgpack.hpp>

using std::string;
using std::vector;

// statuses to client
#define STATUS_OK 0
#define STATUS_NOTOK 1

#define APISTATUS_OK 0
#define APISTATUS_NOTOK 1
#define APISTATUS_NULLCONSTRAINT 2
#define APISTATUS_PENDING 3
#define APISTATUS_FIELD 4
#define APISTATUS_STATE 5
#define APISTATUS_UNIQUECONSTRAINT 6
#define APISTATUS_FOUND 7
#define APISTATUS_DEADLOCK 8
#define APISTATUS_LOCK 9

enum cmd_e
{
    CMD_NONE = 0,
    CMD_SELECT,
    CMD_INSERT,
    CMD_UPDATE,
    CMD_DELETE,
    CMD_BEGIN,
    CMD_COMMIT,
    CMD_ROLLBACK,
    CMD_SET,
    CMD_STOREDPROCEDURE
};

typedef struct
{
    int64_t rowid;
    int64_t tableid;
    int64_t engineid;
} uuRecord_s;

enum __attribute__ ((__packed__)) locktype_e
{
    NOTFOUNDLOCK = -1,
        NOLOCK = 0,
        READLOCK,
        WRITELOCK,
        PENDINGLOCK,
        INDEXLOCK,
        INDEXPENDINGLOCK,
        PENDINGTOWRITELOCK,
        PENDINGTOREADLOCK,
        PENDINGTONOLOCK,
        PENDINGTOINDEXLOCK,
        PENDINGTOINDEXNOLOCK
        };

enum __attribute__ ((__packed__)) operatortypes_e
{
    OPERATOR_NONE = 0,
        OPERATOR_CONCATENATION = 1,
        OPERATOR_ADDITION = 2,
        OPERATOR_SUBTRACTION = 3,
        OPERATOR_MULTIPLICATION = 4,
        OPERATOR_DIVISION = 5,
        OPERATOR_NEGATION = 7,
        OPERATOR_AND = 8,
        OPERATOR_OR = 9,
        OPERATOR_NOT = 10,
        OPERATOR_TRUE = 11,
        OPERATOR_FALSE = 12,
        OPERATOR_UNKNOWN = 13,
        OPERATOR_EQ = 14,
        OPERATOR_NE = 15,
        OPERATOR_LT = 16,
        OPERATOR_GT = 17,
        OPERATOR_LTE = 18,
        OPERATOR_GTE = 19,
        OPERATOR_BETWEEN = 20,
        OPERATOR_ISNULL = 21,
        OPERATOR_IN = 22,
        OPERATOR_LIKE = 23,
        OPERATOR_EXISTS = 24,
        OPERATOR_UNIQUE = 25,
        OPERATOR_BETWEENAND = 26,
        OPERATOR_NOTBETWEEN = 27,
        OPERATOR_ISNOTNULL = 28,
        OPERATOR_NOTIN = 29,
        OPERATOR_NOTLIKE = 30,
        OPERATOR_REGEX = 31,
        OPERATOR_SELECTALL = 32,
        OPERATOR_NULL = 33
        };

typedef struct
{
    int64_t rowid;
    int64_t previoussubtransactionid;
    locktype_e locktype;
    std::string row;
} returnRow_s;

enum __attribute__ ((__packed__)) fieldtype_e
{
    NOFIELDTYPE = -1,
    INT = 0,
    UINT = 1,
    BOOL = 2,
    FLOAT = 3,
    CHAR = 4,
    CHARX = 5,
    VARCHAR = 6
};

typedef struct
{
    fieldtype_e type;
    std::string name;
} fieldtypename_s;

typedef union __attribute__ ((__packed__)) fieldInput_u
{
    int64_t integer;
    uint64_t uinteger;
    bool boolean;
    long double floating;
    char character;
} fieldInput_s;

typedef struct
{
    fieldInput_s value;
    std::string str;
    bool isnull;
} fieldValue_s;

typedef struct
{
    int64_t resultCode;
    msgpack::sbuffer *sbuf;
} procedureResponse_s;

size_t hash_value(uuRecord_s const &);
bool operator==(uuRecord_s const &, uuRecord_s const &);

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
    bool execStatement(const char *, Statement *, apifPtr,
                       int64_t, void *);
    bool execStatement(const char *, vector<std::string> &, apifPtr, int64_t, void *);
    int64_t getResultCode();
    void deleteTransaction();
    void deleteStatement();
    void getStoredProcedureArgs(Statement *, std::vector<std::string> &);

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
