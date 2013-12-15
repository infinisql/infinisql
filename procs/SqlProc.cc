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

//#include "infinisql_gch.h"
//#include "infinisql_defs.h"
#include "infinisql_api.h"
//#include "infinisql_Asts.h"
#line 25 "SqlProc.cc"

extern "C" void InfiniSQL_texas_SqlProc_destroy(ApiInterface *p) {
    delete p;
}

typedef void(ApiInterface::*fptr)(int, void *);

// pgPtr->statementPtr->queries[0].storedProcedureArgs

class SqlProcClass : public ApiInterface
{
public:
    SqlProcClass(class TransactionAgent *taPtrarg, class ApiInterface  *pgPtrarg,
                 void *destructorPtrarg)
    {
        // class boilerplate
        pgPtr = pgPtrarg;
        taPtr = pgPtr->taPtr;
        domainid = pgPtr->domainid;
        transactionPtr = pgPtr->transactionPtr;
        continueFunc1Ptr = &ApiInterface::continueFunc1;
        continueFunc2Ptr = &ApiInterface::continueFunc2;
        continuePgFuncPtr = &ApiInterface::continuePgFunc;
        // end boilerplate
        beginTransaction();
        vector<string> args;
        args.push_back(pgPtr->statementPtr->queries[0].storedProcedureArgs[2]);
        args.push_back(pgPtr->statementPtr->queries[0].storedProcedureArgs[0]);
        if (execStatement("debitbuyer", args, &ApiInterface::continueFunc1, 1,
                          NULL)==false)
        {
            int64_t ret = 1;
            rollback(&ApiInterface::continueFunc1, 10, &ret);
            return;
        }
    }

    void doit(void) {;}

    void continueFunc1(int64_t entrypoint, void *statePtr)
    {
        switch (entrypoint)
        {
        case 1:
        {
            if (results.statementStatus != STATUS_OK ||
                results.statementResults.size() != 1)
            {
                rollback(&ApiInterface::continueFunc1, 10, &results.statementStatus);
                return;
            }

            // next update
            vector<string> args;
            args.push_back(pgPtr->statementPtr->queries[0].storedProcedureArgs[2]);
            args.push_back(pgPtr->statementPtr->queries[0].storedProcedureArgs[1]);
            results = results_s();
            if (execStatement("creditseller", args, &ApiInterface::continueFunc1, 2,
                              NULL)==false)
            {
                int64_t ret = 1;
                rollback(&ApiInterface::continueFunc1, 10, &ret);
                return;
            }
        }
        break;

        case 2:
        {
            if (results.statementStatus != STATUS_OK ||
                results.statementResults.size() != 1)
            {
                rollback(&ApiInterface::continueFunc1, 10, &results.statementStatus);
                return;
            }

            commit(&ApiInterface::continueFunc1, 3, NULL);
        }
        break;

        case 3:
        {
            if (results.statementStatus != STATUS_OK ||
                results.statementResults.size() != 1)
            {
                rollback(&ApiInterface::continueFunc1, 10, &results.statementStatus);
                return;
            }

            pgPtr->results.selectFields.push_back({INT, string("SqlProcResult")});
            fieldValue_s fieldValue = {};
            fieldValue.value.integer=0;
            vector<fieldValue_s> fieldValues;
            fieldValues.push_back(fieldValue);
            pgPtr->results.selectResults[{-1, -1, -1}]=fieldValues;
            exitProc(STATUS_OK);
        }
        break;

        // after rollback
        case 10:
        {
            delete transactionPtr;
            if (*(int64_t *)statePtr==STATUS_OK)
            {
                pgPtr->results.selectFields.push_back({INT, string("SqlProcResult")});
                fieldValue_s fieldValue = {};
                fieldValue.value.integer=1;
                vector<fieldValue_s> fieldValues;
                fieldValues.push_back(fieldValue);
                pgPtr->results.selectResults[{-1, -1, -1}]=fieldValues;
            }
            exitProc(*(int64_t *)statePtr);
        }
        break;

        default:
            printf("%s %i anomaly %li\n", __FILE__, __LINE__, entrypoint);
            exitProc(STATUS_NOTOK);
        }
    }

    void continueFunc2(int64_t entrypoint, void *statePtr) {;}
    void continuePgFunc(int64_t entrypoint, void *statePtr) {;}
    void continuePgCommitimplicit(int64_t entrypoint, void *statePtr) {;}
    void continuePgCommitexplicit(int64_t entrypoint, void *statePtr) {;}
    void continuePgRollbackimplicit(int64_t entrypoint, void *statePtr) {;}
    void continuePgRollbackexplicit(int64_t entrypoint, void *statePtr) {;}

    void exitProc(int64_t status)
    {
        pgPtr->results.statementStatus=status;
        pgPtr->transactionPtr=transactionPtr;
        class ApiInterface *retobject=pgPtr;
        InfiniSQL_texas_SqlProc_destroy(this);
        (*retobject.*(&ApiInterface::continuePgFunc))(0, NULL);
    }
};

extern "C" ApiInterface* InfiniSQL_texas_SqlProc_create \
(class TransactionAgent *taPtr, class ApiInterface *pgPtr,
 void *destructorPtr)
{
    return new SqlProcClass(taPtr, pgPtr, destructorPtr);
}

