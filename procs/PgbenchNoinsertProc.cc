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
#line 25 "PgbenchNoinsertProc.cc"

extern "C" void InfiniSQL_benchmark_PgbenchNoinsert_destroy(ApiInterface *p)
{
    delete p;
}

typedef void(ApiInterface::*fptr)(int, void *);

class PgbenchNoinsertProcClass : public ApiInterface
{
public:
    enum statementstates_e
    {
        DO_UPDATE1=0,
        DO_SELECT,
        DO_UPDATE2,
        DO_UPDATE3,
        DO_INSERT,
        DO_COMMIT
    };
    enum statementstates_e statementstate;
    vector<string> storedProcedureArgs;
    int64_t badstatus;
  
    PgbenchNoinsertProcClass(class TransactionAgent *taPtrarg, class ApiInterface  *pgPtrarg,
                             void *destructorPtrarg) : statementstate (DO_UPDATE1)
    {
        pgPtr = pgPtrarg;
        taPtr = pgPtr->taPtr;
        domainid = pgPtr->domainid;
        if (pgPtr->transactionPtr != NULL)
        {
            exitProc(STATUS_NOTOK, 0);
            return;
        }
        storedProcedureArgs=pgPtr->statementPtr->queries[0].storedProcedureArgs;
        results.statementStatus=STATUS_OK;

        beginTransaction();
    
        continueFunc1(1, NULL);
    }

    void doit(void) {;}

    void continueFunc1(int64_t entrypoint, void *statePtr)
    {
        switch (entrypoint)
        {
        case 1:
        {
            string &deltaRef=storedProcedureArgs[0];
            string &aidRef=storedProcedureArgs[1];
            string &tidRef=storedProcedureArgs[2];
            string &bidRef=storedProcedureArgs[3];
        
            if (results.statementStatus != STATUS_OK)
            {
                badstatus=results.statementStatus;
                rollback(&ApiInterface::continueFunc1, 3, NULL);
                return;
            }
        
            switch (statementstate)
            {
            case DO_UPDATE1:
            {
                vector<string> statementArgs;
                statementArgs.push_back(deltaRef);
                statementArgs.push_back(aidRef);
                statementstate=DO_SELECT;
                if (execStatement("pgbench_updateaccounts", statementArgs,
                                  &ApiInterface::continueFunc1, 1, NULL)==false)
                {
                    rollback(&ApiInterface::continueFunc1, 3, NULL);
                    return;
                }
            }
            break;
            
            case DO_SELECT:
            {
                vector<string> statementArgs;
                statementArgs.push_back(aidRef);
                statementstate=DO_UPDATE2;
                if (execStatement("pgbench_selectaccounts", statementArgs,
                                  &ApiInterface::continueFunc1, 1, NULL)==false)
                {
                    rollback(&ApiInterface::continueFunc1, 3, NULL);
                    return;
                }
            }
            break;
            
            case DO_UPDATE2:
            {
                pgPtr->results.selectFields=results.selectFields;
                pgPtr->results.selectResults=results.selectResults;
                vector<string> statementArgs;
                statementArgs.push_back(deltaRef);
                statementArgs.push_back(tidRef);
                statementstate=DO_UPDATE3;
                if (execStatement("pgbench_updatetellers", statementArgs,
                                  &ApiInterface::continueFunc1, 1, NULL)==false)
                {
                    rollback(&ApiInterface::continueFunc1, 3, NULL);
                    return;
                }
            }
            break;
            
            case DO_UPDATE3:
            {
                vector<string> statementArgs;
                statementArgs.push_back(deltaRef);
                statementArgs.push_back(bidRef);
                statementstate=DO_COMMIT;
                if (execStatement("pgbench_updatebranches", statementArgs,
                                  &ApiInterface::continueFunc1, 1, NULL)==false)
                {
                    rollback(&ApiInterface::continueFunc1, 3, NULL);
                    return;
                }
            }
            break;
            
            case DO_INSERT:
            {
                vector<string> statementArgs;
                statementArgs.push_back(tidRef);
                statementArgs.push_back(bidRef);
                statementArgs.push_back(aidRef);
                statementArgs.push_back(deltaRef);
                statementstate=DO_COMMIT;
                if (execStatement("pgbench_inserthistory", statementArgs,
                                  &ApiInterface::continueFunc1, 1, NULL)==false)
                {
                    rollback(&ApiInterface::continueFunc1, 3, NULL);
                    return;
                }
            }
            break;
            
            case DO_COMMIT:
                commit(&ApiInterface::continueFunc1, 2, NULL);
                break;
          
            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, statementstate);
                exitProc(statementstate, 0);
            }
        
        }
        break;

        // return from commit
        case 2:
            if (results.statementStatus != STATUS_OK)
            {
                badstatus=results.statementStatus;
                rollback(&ApiInterface::continueFunc1, 3, &results.statementStatus);
                return;
            }

            delete transactionPtr;
            exitProc(STATUS_OK, 0);
            break;
     
            // return from rollback
        case 3:
            delete transactionPtr;
            exitProc(badstatus, 0);
            break;

        default:
            printf("%s %i anomaly %li\n", __FILE__, __LINE__, entrypoint);
            exitProc(2000+entrypoint, 0);
        }
    }
  
    ~PgbenchNoinsertProcClass()
    {
    }

    void continueFunc2(int64_t entrypoint, void *statePtr) {;}
    void continuePgFunc(int64_t entrypoint, void *statePtr) {;}
    void continuePgCommitimplicit(int64_t entrypoint, void *statePtr) {;}
    void continuePgCommitexplicit(int64_t entrypoint, void *statePtr) {;}
    void continuePgRollbackimplicit(int64_t entrypoint, void *statePtr) {;}
    void continuePgRollbackexplicit(int64_t entrypoint, void *statePtr) {;}

    void exitProc(int64_t status, int64_t procresult)
    {
        pgPtr->results.statementStatus=status;
        class ApiInterface *retobject=pgPtr;
        delete pgPtr->statementPtr;
        InfiniSQL_benchmark_PgbenchNoinsert_destroy(this);
        (*retobject.*(&ApiInterface::continuePgFunc))(0, NULL);
    }
};

extern "C" ApiInterface* InfiniSQL_benchmark_PgbenchNoinsert_create \
(class TransactionAgent *taPtr, class ApiInterface *pgPtr,
 void *destructorPtr)
{
    return new PgbenchNoinsertProcClass(taPtr, pgPtr, destructorPtr);
}
