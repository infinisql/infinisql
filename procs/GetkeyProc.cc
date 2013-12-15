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
#line 25 "GetkeyProc.cc"

extern "C" void InfiniSQL_benchmark_Getkey_destroy(ApiInterface *p)
{
    delete p;
}

typedef void(ApiInterface::*fptr)(int, void *);

class GetkeyProcClass : public ApiInterface
{
public:
    vector<string> storedProcedureArgs;
  
    GetkeyProcClass(class TransactionAgent *taPtrarg, class ApiInterface  *pgPtrarg,
                    void *destructorPtrarg)
    {
        pgPtr = pgPtrarg;
        taPtr = pgPtr->taPtr;
        domainid = pgPtr->domainid;
        if (pgPtr->transactionPtr != NULL)
        {
            exitProc(STATUS_NOTOK);
            return;
        }

        beginTransaction();
        if (execStatement("keyval_select", 
                          pgPtr->statementPtr->queries[0].storedProcedureArgs,
                          &ApiInterface::continueFunc1, 1, NULL)==false)
        {
            rollback(&ApiInterface::continueFunc1, 2, NULL);
        }
    }

    void doit(void) {;}
  
    void continueFunc1(int64_t entrypoint, void *statePtr)
    {
        switch (entrypoint)
        {
        case 1:
        {
            if (transactionPtr->resultCode != STATUS_OK)
            {
                rollback(&ApiInterface::continueFunc1, 2, NULL);
                return;
            }

            pgPtr->results.selectFields=results.selectFields;
            pgPtr->results.selectResults=results.selectResults;
            commit(&ApiInterface::continueFunc1, 3, NULL);
        }
        break;
      
        case 2:
        { // return from rollback
            delete transactionPtr;
            exitProc(STATUS_NOTOK);
        }
        break;
      
        case 3:
        { // return from commit
            int64_t s=transactionPtr->resultCode;
            delete transactionPtr;
            exitProc(s);
        }
        break;
      
        default:
            printf("%s %i anomalous status %li\n", __FILE__, __LINE__,
                   results.statementStatus);
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
        class ApiInterface *retobject=pgPtr;
        delete pgPtr->statementPtr;
        InfiniSQL_benchmark_Getkey_destroy(this);
        (*retobject.*(&ApiInterface::continuePgFunc))(0, NULL);
    }
};

extern "C" ApiInterface* InfiniSQL_benchmark_Getkey_create \
(class TransactionAgent *taPtr, class ApiInterface *pgPtr,
 void *destructorPtr)
{
    return new GetkeyProcClass(taPtr, pgPtr, destructorPtr);
}
