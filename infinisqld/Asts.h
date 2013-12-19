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

/**
 * @file   Asts.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:05:40 2013
 * 
 * @brief  Abstract Syntax Tree class and Statement class. A SQL statement
 * turns into these in order to be executed.
 */

#ifndef INFINISQLASTS_H
#define INFINISQLASTS_H

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include "defs.h"
#include "larx.h"

class ApiInterface;
typedef void(ApiInterface::*apifPtr)(int64_t, void *);

typedef ApiInterface *(*spclasscreate)(class TransactionAgent *,
                                       class ApiInterface *, void *);
typedef void(*spclassdestroy)(ApiInterface *);

using std::string;
using std::vector;

/** 
 * @brief Abstract Syntax Tree
 */
class Ast
{
public:
    Ast();
    Ast(class Ast *parentarg , operatortypes_e operatortypearg); // for operators
    Ast(class Ast *parentarg, std::string &operandarg); // for operands
    Ast(const Ast &orig);
    Ast &operator= (const Ast &orig);
    void cp(const Ast &orig);
    virtual ~Ast();

    /* returns complete true, incomplete false. 1st *Ast is node to evaluate,
     * 2nd is next node to evaluate (or NULL)
     */
    bool evaluate(class Ast **nextAstNode,
                  class Statement *statementPtr statementPtr);
    /* evaluates for setassignment per row. does not get backgrounded */
    void evaluateAssignment(std::vector<fieldValue_s> &,
                            class Statement *statementPtr);
    void normalizeSetAssignmentOperand(vector<fieldValue_s> &fieldValues,
                                       class Statement *statementPtr);
    static void toFloat(const string &inoperand, fieldValue_s &outField);
    static void toFloat(const string &inoperand, string &outoperand);

    class Ast *parent;
    class Ast *rightchild;
    class Ast *leftchild;

    bool isoperator; // false is operand
    operatortypes_e operatortype;
    std::string operand; // type and value is embedded
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
        std::string operandstr;
    };

    struct inobject_s
    {
        bool issubquery; // false is expressionlist
        int64_t subquery; // index value of subquery in subqueries
        std::vector<class Ast *> expressionlist;
    };

    struct column_s
    {
        char aggregatetype;
        int64_t fieldid;
        std::string name;
    };

    /* results of evaulations during query execution, including final results */
    struct results_s
    {
        boost::unordered_map<uuRecord_s, returnRow_s> searchResults;
        /* each string is a field, with type embedded in 1st char */
        boost::unordered_map< uuRecord_s,
                              std::vector<fieldValue_s> > selectResults;
        size_t initerator;
        std::vector<fieldValue_s> inValues;
        std::vector<fieldValue_s> insertValues;
        std::string newrow; // insert
        uuRecord_s originalrowuur;
        int64_t newrowengineid;
        uuRecord_s newrowuur;
        boost::unordered_map<uuRecord_s,
                             returnRow_s>::const_iterator updateIterator;
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

        std::string table;
        int64_t tableid;
        locktype_e locktype;
        std::vector<std::string> groupByList; // identifiers
        // fromColumns are operands
        std::vector<std::string> fromColumns;
        std::vector<column_s> fromColumnids;
        std::vector<orderbyitem_s> orderbylist;
        inobject_s inobject;

        class Ast *searchCondition;
        boost::unordered_map<string, class Ast *> assignments;
        boost::unordered_map<int64_t, class Ast *> fieldidAssignments;
        std::vector<class Ast *> insertColumns;

        std::string storedProcedure;
        std::vector<std::string> storedProcedureArgs;

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
    Statement(class TransactionAgent *taPtrarg, class Schema *schemaPtrarg);
    Statement(const Statement &orig);
    Statement &operator= (const Statement &orig);
    void cp(const Statement &orig);
    virtual ~Statement();

    query_s cpquery(const query_s &orig);
    bool resolveTableFields();
    bool resolveTableFields2();
    bool resolveFieldNames(class Ast *myPosition);
    int64_t getfieldid(int64_t tableid, const string & fieldName);
    bool stagedPredicate(operatortypes_e op, int64_t tableid,
                         string &leftoperand, string &rightoperand,
                         vector<fieldValue_s> &inValues,
                         const boost::unordered_map<uuRecord_s,
                         stagedRow_s> &stagedRows,
                         boost::unordered_map<uuRecord_s, returnRow_s> &results);
    void andPredicate(operatortypes_e op, int64_t tableid,
                      string &leftoperand, string &rightoperand,
                      vector<fieldValue_s> &inValues,
                      const boost::unordered_map<uuRecord_s,
                      returnRow_s> &andResults,
                      boost::unordered_map<uuRecord_s, returnRow_s> &results);
    void execute(class ApiInterface *reentryObject, apifPtr reentryfptr,
                 int64_t reentrypoint, void *reentrydata,
                 class Transaction *transactionPtrarg,
                 const vector<string> &parametersarg);
    /* return if evaluation is complete (true), or incomplete (false)
     * if incomplete, caller should return as it means a background transaction
     * call is taking place if the nextastnode is NULL, or it should execute
     * the nextastnode, which is one of the children
     */
    void searchExpression(int64_t entrypoint, class Ast *astNode);
    void branchtotype();
    void reenter(int64_t status);
    void continueSelect(int64_t entrypoint, class Ast *ignorethis);
    void continueDelete(int64_t entrypoint, class Ast *ignorethis);
    void continueInsert(int64_t entrypoint, class Ast *ignorethis);
    void continueUpdate(int64_t entrypoint, class Ast *ignorethis);
    void startQuery();
    void subqueryScalar(class Ast *astnode);
    void subqueryUnique(class Ast *astnode);
    void subqueryExists(class Ast *astnode);
    void subqueryIn(class Ast *astnode);
    void abortQuery(int64_t status);

    class TransactionAgent *taPtr;
    class Schema *schemaPtr;
    reentry_s reentry;
    class Transaction *transactionPtr;
    query_s *currentQuery;

    std::vector<query_s> queries;
    std::vector<std::string> parameters;

    ssize_t queryindex;
};

#endif  /* INFINISQLASTS_H */
