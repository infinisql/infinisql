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

#ifndef ASTS_H
#define ASTS_H

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include "infinisql_defs.h"
#include "larx.h"

class ApiInterface;
typedef void(ApiInterface::*apifPtr)(int64_t, void *);

typedef ApiInterface *(*spclasscreate)(class TransactionAgent *,
                                       class ApiInterface *, void *);
typedef void(*spclassdestroy)(ApiInterface *);

using std::string;
using std::vector;

class Ast
{
public:
  Ast();
  Ast(class Ast *, operatortypes_e); // for operators
  Ast(class Ast *, string &); // for operands
  Ast(const Ast &);
  Ast &operator= (const Ast &);
  void cp(const Ast &);
  virtual ~Ast();

  /* returns complete true, incomplete false. 1st *Ast is node to evaluate,
   * 2nd is next node to evaluate (or NULL)
   */
  bool evaluate(class Ast **, class Statement *statementPtr);
  /* evaluates for setassignment per row. does not get backgrounded */
  void evaluateAssignment(vector<fieldValue_s> &,
                          class Statement *statementPtr);
  void normalizeSetAssignmentOperand(vector<fieldValue_s> &,
                                     class Statement *statementPtr);
  static void toFloat(const string &, fieldValue_s &);
  static void toFloat(const string &, string &);

  //  class Statement *statementPtr;

  class Ast *parent;
  class Ast *rightchild;
  class Ast *leftchild;

  bool isoperator; // false is operand
  operatortypes_e operatortype;
  string operand; // type and value is embedded
  boost::unordered_map<uuRecord_s, returnRow_s> predicateResults;
  //private:
};

class Statement;
typedef void(Statement::*statementfPtr)(int64_t, void *);

class Statement
{
public:
  struct orderbyitem_s
  {
    bool isasc; // false is descending
    string operandstr;
  };

  struct inobject_s
  {
    bool issubquery; // false is expressionlist
    int64_t subquery; // index value of subquery in subqueries
    vector<class Ast *> expressionlist;
  };

  struct column_s
  {
    char aggregatetype;
    int64_t fieldid;
    string name;
  };

  /* results of evaulations during query execution, including final results */
  struct results_s
  {
    boost::unordered_map<uuRecord_s, returnRow_s> searchResults;
    /* each string is a field, with type embedded in 1st char */
    boost::unordered_map< uuRecord_s, vector<fieldValue_s> > selectResults;
    size_t initerator;
    vector<fieldValue_s> inValues;
    vector<fieldValue_s> insertValues;
    string newrow; // insert
    uuRecord_s originalrowuur;
    int64_t newrowengineid;
    uuRecord_s newrowuur;
    boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator updateIterator;
    // setFields[fieldid] = fieldValue
    boost::unordered_map<int64_t, fieldValue_s> setFields;
  };

  struct query_s
  {
    ssize_t instance;

    cmd_e type;
    bool isforupdate;
    bool hasnolock;
    bool haswhere;
    bool hasgroupby;
    bool hashaving;
    bool hasorderby;

    string table;
    int64_t tableid;
    locktype_e locktype;
    vector<string> groupByList; // identifiers
    // fromColumns are operands
    vector<string> fromColumns;
    vector<column_s> fromColumnids;
    vector<orderbyitem_s> orderbylist;
    inobject_s inobject;

    class Ast *searchCondition;
    boost::unordered_map<string, class Ast *> assignments;
    boost::unordered_map<int64_t, class Ast *> fieldidAssignments;
    vector<class Ast *> insertColumns;

    string storedProcedure;
    vector<string> storedProcedureArgs;

    results_s results;
  };

  enum direction_e
  {
    FROM_ABOVE = 1,
    FROM_LEFT = 2,
    FROM_RIGHT = 3
  };

  struct reentry_s
  {
    class ApiInterface *reentryObject;
    apifPtr reentryfptr;
    int64_t reentrypoint;
    void *reentrydata;
  };

  Statement();
  Statement(class TransactionAgent *, class Schema *);
  Statement(const Statement &);
  Statement &operator= (const Statement &);
  void cp(const Statement &);
  virtual ~Statement();

  query_s cpquery(const query_s &);
  bool resolveTableFields();
  bool resolveTableFields2();
  bool resolveFieldNames(class Ast *);
  int64_t getfieldid(int64_t, const string &);
  bool stagedPredicate(operatortypes_e, int64_t, string &, string &,
                       vector<fieldValue_s> &,
                       const boost::unordered_map<uuRecord_s, stagedRow_s> &,
                       boost::unordered_map<uuRecord_s, returnRow_s> &);
  void andPredicate(operatortypes_e, int64_t, string &, string &,
                    vector<fieldValue_s> &,
                    const boost::unordered_map<uuRecord_s, returnRow_s> &,
                    boost::unordered_map<uuRecord_s, returnRow_s> &);
  void execute(class ApiInterface *, apifPtr, int64_t, void *,
               class Transaction *, const vector<string> &);
  /* return if evaluation is complete (true), or incomplete (false)
   * if incomplete, caller should return as it means a background transaction
   * call is taking place if the nextastnode is NULL, or it should execute
   * the nextastnode, which is one of the children
   */
  void searchExpression(int64_t, class Ast *);
  void branchtotype();
  void reenter(int64_t);
  void continueSelect(int64_t, class Ast *);
  void continueDelete(int64_t, class Ast *);
  void continueInsert(int64_t, class Ast *);
  void continueUpdate(int64_t, class Ast *);
  void startQuery();
  void subqueryScalar(class Ast *);
  void subqueryUnique(class Ast *);
  void subqueryExists(class Ast *);
  void subqueryIn(class Ast *);
  void abortQuery(int64_t);

  class TransactionAgent *taPtr;
  class Schema *schemaPtr;
  reentry_s reentry;
  class Transaction *transactionPtr;
  query_s *currentQuery;

  vector<query_s> queries;
  vector<string> parameters;

  ssize_t queryindex;
};

#endif  /* ASTS_H */
