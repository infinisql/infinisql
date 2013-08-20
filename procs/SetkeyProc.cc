#include "infinisql_gch.h"
#include "infinisql_defs.h"
#include "infinisql_api.h"
#include "infinisql_Asts.h"
#line 6 "SetkeyProc.cc"

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
    DO_INSERT=0,
    DO_CHECKINSERT,
    DO_UPDATING,
    DO_ABORT
  };
  enum statementstates_e statementstate;
  vector<string> storedProcedureArgs;
  int64_t badstatus;
  
  SetkeyProcClass(class TransactionAgent *taPtrarg, class ApiInterface  *pgPtrarg,
      void *destructorPtrarg) : statementstate (DO_INSERT)
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
//    vector<string> &storedProcedureArgsRef=
//            pgPtr->statementPtr->queries[0].storedProcedureArgs;
//    statementArgs.push_back(storedProcedureArgsRef[0]);
//    statementArgs.push_back(storedProcedureArgsRef[1]);
//    printf("%s %i args 1,2 '%s', '%s'\n", __FILE__, __LINE__, statementArgs[0].c_str(), statementArgs[1].c_str());
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
        string &keyRef=storedProcedureArgs[0];
        string &valRef=storedProcedureArgs[1];

        switch (statementstate)
        {
          case DO_INSERT:
          {
            vector<string> statementArgs;
            statementArgs.push_back(keyRef);
            statementArgs.push_back(valRef);
            statementstate=DO_CHECKINSERT;
            if (execStatement("keyval_insert", statementArgs,
                    &ApiInterface::continueFunc1, 1, NULL)==false)
            {
              statementstate=DO_ABORT;
              badstatus=STATUS_NOTOK;
              rollback(&ApiInterface::continueFunc1, 3, NULL);
              return;
            }
          }
          break;
          
          case DO_CHECKINSERT:
          {
            if (results.statementStatus != STATUS_OK)
            {
              rollback(&ApiInterface::continueFunc1, 3, NULL);
              return;
            }
            commit(&ApiInterface::continueFunc1, 2, NULL);
          }
          break;
          
          case DO_UPDATING:
          {
            if (results.statementStatus != STATUS_OK)
            {
              statementstate=DO_ABORT;
              badstatus=results.statementStatus;
              rollback(&ApiInterface::continueFunc1, 3, NULL);
              return;
            }
            commit(&ApiInterface::continueFunc1, 2, NULL);
          }
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
          statementstate=DO_ABORT;
          badstatus=results.statementStatus;
          rollback(&ApiInterface::continueFunc1, 3, &results.statementStatus);
          return;
        }
        delete transactionPtr;
        exitProc(STATUS_OK, 0);
        break;
     
      // return from rollback
      case 3:
      {
        string &keyRef=storedProcedureArgs[0];
        string &valRef=storedProcedureArgs[1];

        switch (statementstate)
        {
          case DO_ABORT:
            delete transactionPtr;
            exitProc(badstatus, 0);
            break;
            
          case DO_CHECKINSERT: // try update
          {
            delete transactionPtr;
            beginTransaction();
            vector<string> statementArgs;
            statementArgs.push_back(keyRef);
            statementArgs.push_back(valRef);
            statementstate=DO_UPDATING;
            if (execStatement("keyval_update", statementArgs,
                    &ApiInterface::continueFunc1, 1, NULL)==false)
            {
              statementstate=DO_ABORT;
              badstatus=STATUS_NOTOK;
              rollback(&ApiInterface::continueFunc1, 3, NULL);
              return;
            }
          }
          break;

          default:
            delete transactionPtr;
            exitProc(badstatus, 0);
        }
      }
      break;

      default:
        printf("%s %i anomaly %li\n", __FILE__, __LINE__, entrypoint);
        exitProc(2000+entrypoint, 0);
    }
  }

  void continueFunc2(int64_t entrypoint, void *statePtr) {;}
  void continuePgFunc(int64_t entrypoint, void *statePtr) {;}
  void continuePgCommitimplicit(int64_t entrypoint, void *statePtr) {;}
  void continuePgCommitexplicit(int64_t entrypoint, void *statePtr) {;}
  void continuePgRollbackimplicit(int64_t entrypoint, void *statePtr) {;}
  void continuePgRollbackexplicit(int64_t entrypoint, void *statePtr) {;}

  void exitProc(int64_t status, int64_t procresult)
  {
    if (status==0)
    {
      pgPtr->results.selectFields.push_back({INT, string("pgbenchResult")});
      vector<fieldValue_s> fieldValues(1, fieldValue_s());
      fieldValues[0].value.integer=procresult;
      pgPtr->results.selectResults[{-1, -1, -1}]=fieldValues;
    }

    pgPtr->results.statementStatus=status;
    class ApiInterface *retobject=pgPtr;
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
