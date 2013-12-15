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

#include "infinisql_gch.h"
#include "infinisql_defs.h"
#include "infinisql_api.h"
#include "infinisql_Asts.h"
#line 25 "SetkeyProc.cc"

extern "C" void InfiniSQL_benchmark_Setkey_destroy(ApiInterface *p)
{
    delete p;
}

typedef void(ApiInterface::*fptr)(int, void *);

class SetkeyProcClass : public ApiInterface
{
public:
    enum statementstates_e
    {
        BEGINNING=0,
        UPDATED,
        INSERTED,
        UPDATELOCKED,
        UPDATENOTFOUND,
        INSERTNOTUNIQUE,
        COMMITTED,
        UNRECOVERABLE
        /*
          DO_INSERT=0,
          DO_CHECKINSERT,
          DO_UPDATING,
          DO_ABORT
        */
    };
    enum statementstates_e statementstate;
    vector<string> storedProcedureArgs;
//  int64_t badstatus;
  
    SetkeyProcClass(class TransactionAgent *taPtrarg, class ApiInterface  *pgPtrarg,
                    void *destructorPtrarg) : statementstate (BEGINNING)
    {
        pgPtr = pgPtrarg;
        taPtr = pgPtr->taPtr;
        domainid = pgPtr->domainid;
        if (pgPtr->transactionPtr != NULL)
        {
            exitProc(STATUS_NOTOK);
            return;
        }
        storedProcedureArgs=pgPtr->statementPtr->queries[0].storedProcedureArgs;
        results.statementStatus=STATUS_OK;

        continueFunc1(1, NULL);
    }

    void doit(void) {;}
  
    void continueFunc1(int64_t entrypoint, void *statePtr)
    {
        string &keyRef=storedProcedureArgs[0];
        string &valRef=storedProcedureArgs[1];
    
        switch (results.statementStatus)
        {
        case STATUS_OK:
        {
            switch (statementstate)
            {
            case BEGINNING:
            {
                beginTransaction();
                vector<string> statementArgs;
                statementArgs.push_back(keyRef);
                statementArgs.push_back(valRef);
                statementstate=UPDATED;
                if (execStatement("keyval_update", statementArgs,
                                  &ApiInterface::continueFunc1, 1, NULL)==false)
                {
                    statementstate=UNRECOVERABLE;
                    rollback(&ApiInterface::continueFunc1, 1, NULL);
                }
            }
            break;
          
            case UPDATED:
            {
                if (!results.statementResults.size())
                { /* no results, try insert. it's ok to keep existing transaction
                   * since its not failed */
                    vector<string> statementArgs;
                    statementArgs.push_back(keyRef);
                    statementArgs.push_back(valRef);
                    statementstate=INSERTED;
                    if (execStatement("keyval_insert", statementArgs,
                                      &ApiInterface::continueFunc1, 1, NULL)==false)
                    {
                        statementstate=UNRECOVERABLE;
                        rollback(&ApiInterface::continueFunc1, 1, NULL);
                    }
                    return;
                }

                // succesful update
                pgPtr->results.selectFields.push_back({VARCHAR, string("val")});
                vector<fieldValue_s> fieldValues(1, fieldValue_s());
                fieldValues[0].str=valRef.substr(1, string::npos);
                pgPtr->results.selectResults[{-1, -1, -1}]=fieldValues;
                statementstate=COMMITTED;
                commit(&ApiInterface::continueFunc1, 1, NULL);
            }
            break;
          
            case INSERTED:
            {            
                pgPtr->results.selectFields.push_back({VARCHAR, string("val")});
                vector<fieldValue_s> fieldValues(1, fieldValue_s());
                fieldValues[0].str=valRef.substr(1, string::npos);
                pgPtr->results.selectResults[{-1, -1, -1}]=fieldValues;
                statementstate=COMMITTED;
                commit(&ApiInterface::continueFunc1, 1, NULL);
            }
            break;
          
            case UPDATELOCKED:
            { // came from rollback, so retry update
                int64_t s=transactionPtr->resultCode;
                delete transactionPtr;
                if (s != STATUS_OK)
                {
                    exitProc(s);
                    return;
                }
                statementstate=BEGINNING;
                results.statementStatus=STATUS_OK;
                continueFunc1(1, NULL);
            }
            break;
                    
            case INSERTNOTUNIQUE:
            { // came back from rollback, retry update
                int64_t s=transactionPtr->resultCode;
                delete transactionPtr;
                if (s != STATUS_OK)
                {
                    exitProc(s);
                    return;
                }
                statementstate=BEGINNING;
                results.statementStatus=STATUS_OK;
                continueFunc1(1, NULL);
            }
            break;
          
            case COMMITTED:
            {
                int64_t s=transactionPtr->resultCode;
                delete transactionPtr;
                exitProc(s);
            }
            break;
          
            case UNRECOVERABLE:
            { // came back from rollback
                delete transactionPtr;
                exitProc(STATUS_NOTOK);
            }
            break;
          
            default:
                printf("%s %i anomalous status %li\n", __FILE__, __LINE__,
                       results.statementStatus);
                exitProc(STATUS_NOTOK);
            }
        }
        break;
        
        case APISTATUS_LOCK:
        {
            statementstate=UPDATELOCKED;
            results.statementStatus=STATUS_OK;
            rollback(&ApiInterface::continueFunc1, 1, NULL);
        }
        break;
        
        case APISTATUS_UNIQUECONSTRAINT:
        {
            statementstate=INSERTNOTUNIQUE;
            results.statementStatus=STATUS_OK;
            rollback(&ApiInterface::continueFunc1, 1, NULL);
        }
        break;
      
        default:
            // bail out, this may cause a leak of Transaction, maybe
            printf("%s %i anomalous state %i\n", __FILE__, __LINE__,
                   statementstate);
            exitProc(APISTATUS_NOTOK);
            return;
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
        InfiniSQL_benchmark_Setkey_destroy(this);
        (*retobject.*(&ApiInterface::continuePgFunc))(0, NULL);
    }
};

extern "C" ApiInterface* InfiniSQL_benchmark_Setkey_create \
(class TransactionAgent *taPtr, class ApiInterface *pgPtr,
 void *destructorPtr)
{
    return new SetkeyProcClass(taPtr, pgPtr, destructorPtr);
}
