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

class Larxer
{
public:
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
        TYPE_NOLOCK = 70
    };

    struct stackmember_s
    {
        stacktypes_e type;
        std::string val;
    };

    Larxer(char *instr, class TransactionAgent *taPtr, class Schema *schemaPtr);
    virtual ~Larxer();

    void pushstack(stacktypes_e type);
    void pushstack(stacktypes_e type, int64_t val);
    void pushstack(stacktypes_e type, long double val);
    void pushstack(stacktypes_e type, const char *val);
    void pushstack(stacktypes_e type, string &val);
    void pushoperand(char operandtype);
    void pushoperand(char operandtype, int64_t val);
    void pushoperand(char operandtype, long double val);
    void pushoperand(char operandtype, const char *val);
    void pushaggregate(char aggregatetype, const char *val);
    stackmember_s popstack();
    int64_t getintval(string &val);
    long double getfloatval(string &val);
    void eatstack(class TransactionAgent *taPtr, class Schema *schemaPtr);
    void printstack();

    void consumeSelect(string &columns);
    void consumeInsert();
    void consumeUpdate();
    void consumeDelete();
    void consumeStoredProcedure();
    class Ast *consumeExpression();
    int64_t consumeSubquery();
    void consumeInobject();
    void consumeColumns(int64_t numcolumns);
    void consumeFrom();
    void consumeWhere();
    void consumeGroupby();
    void consumeHaving();
    void consumeOrderby();

    std::stack<stackmember_s> parsedStack;
    class Statement::query_s *currentQuery;
    class Statement *statementPtr;
};

#endif /* INFINISQLLARXER_H */
