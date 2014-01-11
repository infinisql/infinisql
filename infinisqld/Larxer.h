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
 * @file   Larxer.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:25:37 2013
 * 
 * @brief  Portmanteau of "lexer" and "parser". This class tokenizes, parses
 * and converts SQL into executable Statement with Abstract Syntax Trees.
 */

#ifndef INFINISQLLARXER_H
#define INFINISQLLARXER_H

#include "larx.h"
#include "Asts.h"
#include "gch.h"
#include "defs.h"

/** 
 * @brief create object to tokenize, parse, and compile SQL statement
 *
 * many of these functions are called by the lexer.ll and parser.yy
 * generated code
 *
 * @param instr SQL query
 * @param taPtr TransactionAgent
 * @param schemaPtr Schema
 *
 * @return 
 */
class Larxer
{
public:
    /** 
     * @brief SQL tokens for pre- Ast stack
     *
     */
    enum stacktypes_e
    {
        TYPE_NONE = 0,
        TYPE_IDENTIFIER = 1,
        TYPE_SUBQUERY = 2,
        TYPE_NOT = 3,
        TYPE_ISNULL = 4,
        TYPE_eq = 5,
        TYPE_ne = 6,
        TYPE_lt = 7,
        TYPE_gt = 8,
        TYPE_lte = 9,
        TYPE_gte = 10,
        TYPE_COMPARISON = 11,
        TYPE_ISTRUE = 12,
        TYPE_ISFALSE = 13,
        TYPE_ISUNKNOWN = 14,
        TYPE_INTVAL = 15,
        TYPE_FLOATVAL = 16,
        TYPE_PARAMETER = 17,
        TYPE_STRINGVAL = 18,
        TYPE_concat = 19,
        TYPE_addition = 20,
        TYPE_subtraction = 21,
        TYPE_multiplication = 22,
        TYPE_division = 23,
        TYPE_exponent = 24,
        TYPE_uminus = 25,
        TYPE_ASC = 26,
        TYPE_DESC = 27,
        TYPE_ORDERBY = 28,
        TYPE_FORUPDATE = 29,
        TYPE_HAVING = 30,
        TYPE_GROUPBY = 31,
        TYPE_WHERE = 32,
        TYPE_AVG = 33,
        TYPE_COUNT = 34,
        TYPE_MAX = 35,
        TYPE_MIN = 36,
        TYPE_SUM = 37,
        TYPE_ASTERISK = 38,
        TYPE_DISTINCT = 39,
        TYPE_ALL = 40,
        TYPE_BETWEEN = 41,
        TYPE_BETWEENEXPRESSIONS = 42,
        TYPE_IN = 43,
        TYPE_LIKE = 44,
        TYPE_EXISTS = 45,
        TYPE_UNIQUE = 46,
        TYPE_assignment = 47,
        TYPE_AND = 48,
        TYPE_OR = 49,
        TYPE_SELECT = 50,
        TYPE_INSERT = 51,
        TYPE_UPDATE = 52,
        TYPE_DELETE = 53,
        TYPE_COLUMNS = 55,
        TYPE_EXPRESSION = 56,
        TYPE_FROM = 57,
        TYPE_search_condition = 58,
        TYPE_SORTSPECIFICATION = 59,
        TYPE_AGGREGATE = 60,
        TYPE_operator = 61,
        TYPE_operand = 62,
        TYPE_inobject = 63,
        TYPE_BEGIN = 64,
        TYPE_COMMIT = 65,
        TYPE_ROLLBACK = 66,
        TYPE_SET = 67,
        TYPE_storedprocedure = 68,
        TYPE_inbegin = 69,
        TYPE_NOLOCK = 70,
	TYPE_CREATE = 71,
	TYPE_DROP = 72,
	TYPE_ALTER = 73
    };

    /** 
     * @brief data populatng pre- Ast stack. SQL token and associated value.
     *
     */
    struct stackmember_s
    {
        stacktypes_e type;
        std::string val;
    };


    /** 
     * @brief tokenize, parse, and compile SQL statement
     *
     * many of these functions are called by the lexer.ll and parser.yy
     * generated code
     * 
     * @param instr SQL statement
     * @param taPtr TransactionAgent
     * @param schemaPtr Schema
     */
    Larxer(char *instr, class TransactionAgent *taPtr, class Schema *schemaPtr);
    virtual ~Larxer();

    /** 
     * @brief put item on stack
     *
     * stack is intermediary between parsing and Statement object
     *
     * @param type stack entry
     */
    void pushstack(stacktypes_e type);
    /** 
     * @brief put item on stack
     *
     * stack is intermediary between parsing and Statement object
     *
     * @param type stack entry
     * @param val value associated with entry
     */
    void pushstack(stacktypes_e type, int64_t val);
    /** 
     * @brief put item on stack
     *
     * stack is intermediary between parsing and Statement object
     *
     * @param type stack entry
     * @param val value associated with entry
     */
    void pushstack(stacktypes_e type, long double val);
    /** 
     * @brief put item on stack
     *
     * stack is intermediary between parsing and Statement object
     *
     * @param type stack entry
     * @param val value associated with entry
     */
    void pushstack(stacktypes_e type, const char *val);
    /** 
     * @brief put item on stack
     *
     * stack is intermediary between parsing and Statement object
     *
     * @param type stack entry
     * @param val value associated with entry
     */
    void pushstack(stacktypes_e type, string &val);
    /** 
     * @brief push operand onto stack
     *
     * @param operandtype type of operand
     */
    void pushoperand(char operandtype);
    /** 
     * @brief push operand onto stack
     *
     * @param operandtype type of operand
     * @param val associated value
     */
    void pushoperand(char operandtype, int64_t val);
    /** 
     * @brief push operand onto stack
     *
     * @param operandtype type of operand
     * @param val associated value
     */
    void pushoperand(char operandtype, long double val);
    /** 
     * @brief push operand onto stack
     *
     * @param operandtype type of operand
     * @param val associated value
     */
    void pushoperand(char operandtype, const char *val);
    /** 
     * @brief push aggregate and associated field onto stack
     *
     * @param aggregatetype type of aggregate
     * @param val associated field
     */
    void pushaggregate(char aggregatetype, const char *val);
    /** 
     * @brief take item off stack
     *
     * @return item
     */
    stackmember_s popstack();
    /** 
     * @brief convert string to integer
     *
     * @param val string input
     *
     * @return integer value
     */
    int64_t getintval(string &val);
    /** 
     * @brief convert string to float
     *
     * @param val string input
     *
     * @return float value
     */
    long double getfloatval(string &val);
    /** 
     * @brief consume entire stack
     *
     * @param taPtr TransactionAgent
     * @param schemaPtr Schema
     */
    void eatstack(class TransactionAgent *taPtr, class Schema *schemaPtr);
    /** 
     * @brief show stack entries, for debugging
     *
     */
    void printstack();

    /** 
     * @brief extract SELECT query from stack
     *
     * @param columns quantity of columns
     */
    void consumeSelect(string &columns);
    /** 
     * @brief extract INSERT query from stack
     *
     */
    void consumeInsert();
    /** 
     * @brief extract UPDATE query from stack
     *
     */
    void consumeUpdate();
    /** 
     * @brief extract DELETE query from stack
     *
     */
    void consumeDelete();
    /** 
     * @brief extract stored procedure call query from stack
     *
     */
    void consumeStoredProcedure();
    /** 
     * @brief extract expression from stack
     *
     *
     * @return Ast object
     */
    class Ast *consumeExpression();
    /** 
     * @brief extract subquery from stack
     *
     *
     * @return index of vector of queries
     */
    int64_t consumeSubquery();
    /** 
     * @brief extract list of IN values from stack
     *
     */
    void consumeInobject();
    /** 
     * @brief extract columns from stack
     *
     * @param numcolumns number of columns
     */
    void consumeColumns(int64_t numcolumns);
    /** 
     * @brief extract FROM clause from stack
     *
     */
    void consumeFrom();
    /** 
     * @brief extract WHERE clause from stack
     *
     */
    void consumeWhere();
    /** 
     * @brief extract GROUP BY clause from stack
     *
     */
    void consumeGroupby();
    /** 
     * @brief extract HAVING clause from stack
     *
     */
    void consumeHaving();
    /** 
     * @brief extract ORDER BY clause from stack
     *
     */
    void consumeOrderby();

    std::stack<stackmember_s> parsedStack;
    class Statement::query_s *currentQuery;
    class Statement *statementPtr;
};

#endif /* INFINISQLLARXER_H */
