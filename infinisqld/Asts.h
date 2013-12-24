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
    /** 
     * @brief create abstract syntax tree object
     *
     * @param parentarg parent Ast (NULL if root)
     * @param operatortypearg operator type
     */
    Ast(class Ast *parentarg , operatortypes_e operatortypearg); // for operators
    /** 
     * @brief create abstract syntax tree object
     *
     * @param parentarg parent Ast (NULL if root)
     * @param operandarg operand
     */
    Ast(class Ast *parentarg, std::string &operandarg); // for operands
    Ast(const Ast &orig);
    Ast &operator= (const Ast &orig);
    /** 
     * @brief deep copy of Ast
     *
     * @param orig source Ast
     */
    void cp(const Ast &orig);
    virtual ~Ast();

    /** 
     *
     * evaluate Ast as part of continuation.
     * returns the next Ast node to evaluate in 1st param, or NULL if
     * finished.
     * if both children (left only for unary operator) are not operators
     * then return false so caller resolves it. Evaluate each child, convert
     * self to resulting operand, return true
     *
     * @param nextAstNode next Ast node to evaluate by subsequent call
     * @param statementPtr current Ast node to evaluate
     *
     * @return 
     */
    bool evaluate(class Ast **nextAstNode,
                  class Statement *statementPtr statementPtr);
    /* evaluates for setassignment per row. does not get backgrounded */
    /** 
     *  evaluate assignments in UPDATE queries
     *
     * @param fieldValues list of values to set each row to
     * @param statementPtr associated statement
     */
    void evaluateAssignment(std::vector<fieldValue_s> &fieldValues,
                            class Statement *statementPtr);
    /** 
     *  converts INTEGER to FLOAT (or leaves float alone).
     * Supports arithmetic between numbers. Parser doesn't determine type
     * based on context, but on content. So, "35+15.7" is parsed as
     * INTEGER + FLOAT. While "35.0+15.7" is parsed as FLOAT + FLOAT.
     * This function casts INTEGER to FLOAT so math can be performed.
     *
     * @param inoperand operand to convert
     * @param outField converted to fieldValue_s type
     */
    static void toFloat(const string &inoperand, fieldValue_s &outField);
    /** 
     * converts INTEGER to FLOAT (or leaves float alone).
     * Supports arithmetic between numbers. Parser doesn't determine type
     * based on context, but on content. So, "35+15.7" is parsed as
     * INTEGER + FLOAT. While "35.0+15.7" is parsed as FLOAT + FLOAT.
     * This function casts INTEGER to FLOAT so math can be performed.
     * 
     * @param inoperand operand to convert
     * @param outoperand output operand
     */
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

/** 
 * @brief contains all necessary information to execute a SQL statement
 *
 */
class Statement
{
public:
    /** 
     * @brief ORDER BY fields and order
     *
     */
    struct orderbyitem_s
    {
        bool isasc;             /**< true ASCENDING, false DESCENDING */
        std::string operandstr; /**< field operand */
    };

    /** 
     * @brief IN (or NOT IN) row values
     *
     */
    struct inobject_s
    {
        bool issubquery; /**< true subquery, false expression */
        int64_t subquery; /**< if subquery, index value to Statement::queries */
        std::vector<class Ast *> expressionlist; /**< predicate value */
    };

    /** 
     * @brief FROM clause column
     *
     */
    struct column_s
    {
        char aggregatetype;     /**< if AGGREGATE, type */
        int64_t fieldid;        /**< fieldid */
        std::string name;       /**< field name */
    };

    /** 
     * @brief results of evaulations during query execution, including final
     * results
     *
     */
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

    /** 
     * @brief all elements for a main or sub-query
     *
     */
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

    /** 
     * @brief Ast walking direction
     *
     */
    enum direction_e
    {
        FROM_ABOVE = 1,
        FROM_LEFT = 2,
        FROM_RIGHT = 3
    };

    /** 
     * @brief continuation destination
     *
     */
    struct reentry_s
    {
        class ApiInterface *reentryObject;
        apifPtr reentryfptr;
        int64_t reentrypoint;
        void *reentrydata;
    };

    Statement();
    /** 
     * @brief create Statement object
     *
     * @param taPtrarg associated TransactionAgent
     * @param schemaPtrarg associated Schema
     */
    Statement(class TransactionAgent *taPtrarg, class Schema *schemaPtrarg);
    Statement(const Statement &orig);
    Statement &operator= (const Statement &orig);
    /** 
     * @brief deep copy of Statement
     *
     * @param orig source Statement
     */
    void cp(const Statement &orig);
    virtual ~Statement();

    /** 
     * @brief deep copy of query_s instance
     *
     * @param orig source query_s
     *
     * @return new query_s 
     */
    query_s cpquery(const query_s &orig);
    /** 
     * @brief resolves table and field names to integers
     *
     *
     * @return success (true) or failure (false)
     */
    bool resolveTableFields();
    /** 
     * called by resolveTableFields recursively on each
     * query then subquery to resolve table and field names to integers
     *
     *
     * @return success (true) or failure (false)
     */
    bool resolveTableFields2();
    /** 
     * @brife field name resolution for resolveTableFields
     *
     * @param myPosition Ast with field name as operand
     *
     * @return success (true) or failure (false)
     */
    bool resolveFieldNames(class Ast *myPosition);
    /** 
     * @brief returns fieldid
     *
     * @param tableid tableid
     * @param fieldName name of field
     *
     * @return fieldid
     */
    int64_t getfieldid(int64_t tableid, const string &fieldName);
    /** 
     * perform search predicate query on results already gathered by this
     * transaction. saves from having to do unnecessary message traffic
     * with engines
     *
     * @param op operation
     * @param tableid tbaleid
     * @param leftoperand left operand of Ast
     * @param rightoperand right operand of Ast
     * @param inValues for IN (or NOT IN) predicate, these are the values
     * @param stagedRows rows already gathered
     * @param results rows matched by predicate
     *
     * @return true if rows matched, false if none
     */
    bool stagedPredicate(operatortypes_e op, int64_t tableid,
                         string &leftoperand, string &rightoperand,
                         vector<fieldValue_s> &inValues,
                         const boost::unordered_map<uuRecord_s,
                         stagedRow_s> &stagedRows,
                         boost::unordered_map<uuRecord_s, returnRow_s> &results);
    /**
     * not yet functional, possibly gratuitous, redundant to stagedPredicate
     */
    void andPredicate(operatortypes_e op, int64_t tableid,
                      string &leftoperand, string &rightoperand,
                      vector<fieldValue_s> &inValues,
                      const boost::unordered_map<uuRecord_s,
                      returnRow_s> &andResults,
                      boost::unordered_map<uuRecord_s, returnRow_s> &results);
    /** 
     * @brief execute SQL statement
     *
     * @param reentryObject object to continue to after execution
     * @param reentryfptr function pointer to continue to
     * @param reentrypoint entry point to continue to
     * @param reentrydata arbitrary data to pass to continuation
     * @param transactionPtrarg associated Transaction (NULL if none)
     * @param parametersarg parameters to SQL statement
     */
    void execute(class ApiInterface *reentryObject, apifPtr reentryfptr,
                 int64_t reentrypoint, void *reentrydata,
                 class Transaction *transactionPtrarg,
                 const vector<string> &parametersarg);
     */
    /** 
     * @brief perform search expression
     *
     * return if evaluation is complete (true), or incomplete (false)
     * if incomplete, caller should return as it means a background transaction
     * call is taking place if the nextastnode is NULL, or it should execute
     * the nextastnode, which is one of the children
     * 
     * @param entrypoint entrypoint in calling function to return to
     * @param astNode Ast to evaluate
     */
    void searchExpression(int64_t entrypoint, class Ast *astNode);
    /** 
     * @brief SELECT, INSERT, UPDATE, DELETE, stored proc branch to
     *
     */
    void branchtotype();
    /** 
     * @brief return to calling function (Statement execution complete)
     *
     * @param status 
     */
    void reenter(int64_t status);
    /** 
     * @brief continuation for SELECT statement
     *
     * @param entrypoint 
     * @param ignorethis 
     */
    void continueSelect(int64_t entrypoint, class Ast *ignorethis);
    /** 
     * @brief continuation for DELETE statement
     *
     * @param entrypoint 
     * @param ignorethis 
     */
    void continueDelete(int64_t entrypoint, class Ast *ignorethis);
    /** 
     * @brief continuation for INSERT statement
     *
     * @param entrypoint 
     * @param ignorethis 
     */
    void continueInsert(int64_t entrypoint, class Ast *ignorethis);
    /** 
     * @brief continuation for UPDATE statement
     *
     * @param entrypoint 
     * @param ignorethis 
     */
    void continueUpdate(int64_t entrypoint, class Ast *ignorethis);
    /** 
     * @brief begin query execution
     *
     */
    void startQuery();
    /** 
     * @brief subquery that returns a scalar
     *
     * @param astnode 
     */
    void subqueryScalar(class Ast *astnode);
    /** 
     * @brief UNIQUE type of subquery
     *
     * @param astnode 
     */
    void subqueryUnique(class Ast *astnode);
    /** 
     * @brief EXISTS subquery
     *
     * @param astnode 
     */
    void subqueryExists(class Ast *astnode);
    /** 
     * @brief subquery returns list of values for IN predicate
     *
     * @param astnode 
     */
    void subqueryIn(class Ast *astnode);
    /** 
     * @brief g-bye
     *
     * @param status status code returned to caller
     */
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
