#include "infinisql_gch.h"
#include "infinisql_defs.h"
#include "infinisql_api.h"
#include "infinisql_Asts.h"
#line 6 "GetkeyProc.cc"

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
