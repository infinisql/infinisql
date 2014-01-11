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
 * @file   Larxer.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:24:38 2013
 * 
 * @brief  Portmanteau of "lexer" and "parser". This class tokenizes, parses
 * and converts SQL into executable Statement with Abstract Syntax Trees.
 */

#include "gch.h"
#include "Larxer.h"
#include <stdint.h>
#line 33 "Larxer.cc"

Larxer::Larxer(char *instr, class TransactionAgent *taPtr,
               class Schema *schemaPtr)
{
    struct perlarxer pld;
    pld.larxerPtr = this;
    flexinit(&pld);
    flexbuffer(instr, strlen(instr), pld.scaninfo);

    // must clear stack before parsing statement
    while (!parsedStack.empty())
    {
        parsedStack.pop();
    }

    if (yyparse(&pld))
    {
        statementPtr=NULL;
        flexdestroy(pld.scaninfo);
        return;
    }

    flexdestroy(pld.scaninfo);
    eatstack(taPtr, schemaPtr);
}

Larxer::~Larxer()
{
}

void Larxer::pushstack(stacktypes_e type)
{
    stackmember_s m = {type, string()};
    parsedStack.push(m);
}

void Larxer::pushstack(stacktypes_e type, int64_t val)
{
    string stackstr(sizeof(val), '0');
    memcpy(&stackstr[0], &val, sizeof(val));
    parsedStack.push({type, stackstr});
}

void Larxer::pushstack(stacktypes_e type, long double val)
{
    printf("%s %i pushstack float %Lf\n", __FILE__, __LINE__, val);
    string stackstr(sizeof(val), '0');
    memcpy(&stackstr[0], &val, sizeof(val));
    parsedStack.push({type, stackstr});
}

void Larxer::pushstack(stacktypes_e type, const char *val)
{
    parsedStack.push({type, string(val)});
}

void Larxer::pushstack(stacktypes_e type, string &val)
{
    parsedStack.push({type, val});
}

void Larxer::pushoperand(char operandtype)
{
    string str(&operandtype, 1);
    pushstack(Larxer::TYPE_operand, str);
}

void Larxer::pushoperand(char operandtype, int64_t val)
{
    string str(1 + sizeof(val), char(0));
    str[0] = operandtype;
    memcpy(&str[1], &val, sizeof(val));
    pushstack(Larxer::TYPE_operand, str);
}

void Larxer::pushoperand(char operandtype, long double val)
{
    string str(1 + sizeof(val), char(0));
    str[0] = operandtype;
    memcpy(&str[1], &val, sizeof(val));
    pushstack(Larxer::TYPE_operand, str);
}

void Larxer::pushoperand(char operandtype, const char *val)
{
    string str(1 + strlen(val), char(0));
    str[0] = operandtype;
    memcpy(&str[1], val, strlen(val));
    pushstack(Larxer::TYPE_operand, str);
}

void Larxer::pushaggregate(char aggregatetype, const char *val)
{
    string str(2 + strlen(val), char(0));
    str[0] = OPERAND_AGGREGATE;
    str[1] = aggregatetype;
    memcpy(&str[2], val, strlen(val));
    pushstack(Larxer::TYPE_operand, str);
}

Larxer::stackmember_s Larxer::popstack()
{
    stackmember_s item = parsedStack.top();
    parsedStack.pop();
    return item;
}

int64_t Larxer::getintval(string &val)
{
    int64_t rv;
    memcpy(&rv, &val[0], sizeof(rv));
    return rv;
}

long double Larxer::getfloatval(string &val)
{
    long double rv;
    memcpy(&rv, &val[0], sizeof(rv));
    return rv;
}

void Larxer::eatstack(class TransactionAgent *taPtr, class Schema *schemaPtr)
{
    statementPtr = new class Statement(taPtr, schemaPtr);
    statementPtr->queries.push_back(Statement::query_s {});
    currentQuery = &statementPtr->queries[0];
    currentQuery->instance = 0;

    while (!parsedStack.empty())
    {
        stackmember_s item = popstack();

        switch (item.type)
        {
        case TYPE_SELECT:
            consumeSelect(item.val);
            break;

        case TYPE_INSERT:
            consumeInsert();
            break;

        case TYPE_UPDATE:
            consumeUpdate();
            break;

        case TYPE_DELETE:
            consumeDelete();
            break;

        case TYPE_CREATE:
        	consumeCreate();
        	break;

        case TYPE_DROP:
        	consumeDrop();
            break;

        case TYPE_ALTER:
        	consumeAlter();
            break;

        case TYPE_storedprocedure:
            consumeStoredProcedure();
            break;

        case TYPE_COMMIT:
            currentQuery->type = CMD_COMMIT;
            break;

        case TYPE_BEGIN:
            currentQuery->type = CMD_BEGIN;
            break;

        case TYPE_ROLLBACK:
            currentQuery->type = CMD_ROLLBACK;
            break;

        default:
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, item.type);
        }
    }
}

/** create Ast, return its root node */
class Ast *Larxer::consumeExpression()
{
    class Ast *rootnode = NULL;
    class Ast *currentnode = NULL;

    if (parsedStack.empty())
    {
        printf("%s %i anomaly\n", __FILE__, __LINE__);
        return NULL;
    }

    stackmember_s item = popstack();

    if (item.type==TYPE_EXPRESSION)
    {
        /* if called as search_expression, then the next object might be an
         * expression, which is already being handled ;-)
         */
        item = popstack();
    }

    switch (item.type)
    {
    case TYPE_operator:
        rootnode = new class Ast(NULL,
                                 (operatortypes_e)getintval(item.val));
        currentnode = rootnode;
        break;

    case TYPE_operand:
        rootnode = new class Ast(NULL, item.val);
        return rootnode;
//        break;

    default:
        printf("%s %i anomaly %i\n", __FILE__, __LINE__, item.type);
        parsedStack.push(item);
        delete rootnode;
        return NULL;
    }

    while (1)
    {
        while (currentnode->rightchild != NULL && currentnode->leftchild != NULL)
        {
            if (currentnode->parent == NULL)
            {
                // no place left to allocate new node, therefore ast is complete
                return rootnode;
            }

            currentnode = currentnode->parent;
        }

        if (parsedStack.empty())
        {
            printf("%s %i anomaly\n", __FILE__, __LINE__);
            return NULL;
        }

        item = popstack();

        class Ast *newnode=NULL;

        switch (item.type)
        {
        case TYPE_operator:
            newnode = new class Ast(currentnode,
                                    (operatortypes_e)getintval(item.val));
            newnode->parent = currentnode;

            if (currentnode->rightchild == NULL)
            {
                currentnode->rightchild = newnode;
            }
            else
            {
                currentnode->leftchild = newnode;
            }

            currentnode = newnode;
            break;

        case TYPE_operand:
            newnode = new class Ast(currentnode, item.val);

            if (currentnode->rightchild == NULL)
            {
                currentnode->rightchild = newnode;
            }
            else
            {
                currentnode->leftchild = newnode;
            }

            if (item.val[0] == OPERAND_SUBQUERY) // subquery
            {
                int64_t sq = consumeSubquery();
                newnode->operand.append(sizeof(sq), char(0));
                memcpy(&newnode->operand[1], &sq, sizeof(sq));
            }

            break;

        case TYPE_EXPRESSION: // noop, get next item
            break;

        case TYPE_inobject:
            consumeInobject();
            break;

        case TYPE_inbegin:
            //        return rootnode;
            break;

        default:
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, item.type);
            parsedStack.push(item);
            return rootnode;
        }
    }
}

// return the index in the vector of queries (statementVars)
int64_t Larxer::consumeSubquery()
{
    ssize_t currentQueryInstance = currentQuery->instance;
    statementPtr->queries.push_back(Statement::query_s());
    size_t i = statementPtr->queries.size()-1;
    currentQuery = &statementPtr->queries[i];
    currentQuery->instance = i;

    stackmember_s item = popstack();

    if (item.type != TYPE_SELECT)
    {
        printf("%s %i anomaly %i\n", __FILE__, __LINE__, item.type);
        return -2;
    }

    consumeSelect(item.val);
    currentQuery = &statementPtr->queries[currentQueryInstance];

    return i;
}

void Larxer::consumeInobject()
{
    if (parsedStack.top().type == TYPE_SUBQUERY)
    {
        parsedStack.pop();
        currentQuery->inobject.issubquery = true;
        currentQuery->inobject.subquery = consumeSubquery();
        return;
    }
    else
    {
        currentQuery->inobject.issubquery = false;

        while (parsedStack.top().type == TYPE_EXPRESSION)
        {
            parsedStack.pop();
            currentQuery->inobject.expressionlist.push_back(consumeExpression());
        }
    }
}

void Larxer::printstack()
{
    while (!parsedStack.empty())
    {
        printf("\tTYPE: %i\n", parsedStack.top().type);

        switch (parsedStack.top().type)
        {
        case TYPE_operator:
            printf("\n");
            int64_t arg;
            memcpy(&arg, &parsedStack.top().val[0], sizeof(arg));

            switch ((operatortypes_e)arg)
            {
            case OPERATOR_NONE:
                printf("NOOP (anomaly) ");
                break;

            case OPERATOR_CONCATENATION:
                printf("CONCATENATION ");
                break;

            case OPERATOR_ADDITION:
                printf("+ ");
                break;

            case OPERATOR_SUBTRACTION:
                printf("- ");
                break;

            case OPERATOR_MULTIPLICATION:
                printf("* ");
                break;

            case OPERATOR_DIVISION:
                printf("/ ");
                break;

            case OPERATOR_NEGATION:
                printf("NEGATE -");
                break;

            case OPERATOR_AND:
                printf("AND ");
                break;

            case OPERATOR_OR:
                printf("OR ");
                break;

            case OPERATOR_NOT:
                printf("NOT ");
                break;

            case OPERATOR_TRUE:
                printf("TRUE ");
                break;

            case OPERATOR_FALSE:
                printf("FALSE ");
                break;

            case OPERATOR_UNKNOWN:
                printf("UNKNOWN ");
                break;

            case OPERATOR_EQ:
                printf("= ");
                break;

            case OPERATOR_NE:
                printf("<> ");
                break;

            case OPERATOR_LT:
                printf("< ");
                break;

            case OPERATOR_GT:
                printf("> ");
                break;

            case OPERATOR_LTE:
                printf("<= ");
                break;

            case OPERATOR_GTE:
                printf(">= ");
                break;

            case OPERATOR_BETWEEN:
                printf("BETWEEN ");
                break;

            case OPERATOR_ISNULL:
                printf("ISNULL ");
                break;

            case OPERATOR_IN:
                printf("IN ");
                break;

            case OPERATOR_LIKE:
                printf("LIKE ");
                break;

            case OPERATOR_EXISTS:
                printf("EXISTS ");
                break;

            case OPERATOR_UNIQUE:
                printf("UNIQUE ");
                break;

            case OPERATOR_BETWEENAND:
                printf("BETWEENAND ");
                break;

            default:
                ;
            }

            break;

        case TYPE_operand:
            switch (parsedStack.top().val[0])
            {
            case OPERAND_STRING:
                printf("%s ", parsedStack.top().val.c_str()+1);
                break;

            case OPERAND_IDENTIFIER:
                printf("%s ", parsedStack.top().val.c_str()+1);
                break;

            case OPERAND_PARAMETER:
            {
                int64_t a;
                memcpy(&a, &parsedStack.top().val[1], sizeof(a));
                printf("$%li ", a);
            }
            break;

            case OPERAND_SUBQUERY:
                printf("subquery, ");
                break;

            case OPERAND_INTEGER:
            {
                int64_t a;
                memcpy(&a, &parsedStack.top().val[1], sizeof(a));
                printf("%li ", a);
            }
            break;

            case OPERAND_BOOLEAN:
                if (parsedStack.top().val[1]=='t')
                {
                    printf("boolean true\n");
                }
                else
                {
                    printf("boolean false\n");
                }

                break;

            case OPERAND_FLOAT:
            {
                long double a;
                memcpy(&a, &parsedStack.top().val[1], sizeof(a));
                printf("$%Lf ", a);
            }
            break;

            default:
                ;
            }

            break;

        default:
            ;
        }

        parsedStack.pop();
    }

    printf("\n");
}

void Larxer::consumeSelect(string &columns)
{
    currentQuery->type = CMD_SELECT;

    while (!parsedStack.empty())
    {
        stackmember_s item = popstack();

        switch (item.type)
        {
        case TYPE_COLUMNS:
            consumeColumns(getintval(columns));

            if (currentQuery->isforupdate==true)
            {
                currentQuery->locktype=WRITELOCK;
            }
            else if (currentQuery->hasnolock==true)
            {
                currentQuery->locktype=NOLOCK;
            }

            {
                currentQuery->locktype=READLOCK;
            }

            return; // end of select statement
//            break;

        case TYPE_FROM:
            consumeFrom();
            break;

        case TYPE_WHERE:
            consumeWhere();
            break;

        case TYPE_GROUPBY:
            consumeGroupby();
            break;

        case TYPE_HAVING:
            consumeHaving();
            break;

        case TYPE_FORUPDATE:
            currentQuery->isforupdate=true;
            break;

        case TYPE_NOLOCK:
            currentQuery->hasnolock=true;
            break;

        case TYPE_ORDERBY:
            consumeOrderby();
            break;

        default:
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, item.type);
        }
    }
}

void Larxer::consumeInsert()
{
    currentQuery->type = CMD_INSERT;
    currentQuery->locktype = WRITELOCK;

    while (!parsedStack.empty())
    {
        stackmember_s item = popstack();

        if (item.type == TYPE_EXPRESSION)
        {
            currentQuery->insertColumns.push_back(consumeExpression());
        }
        else
        {
            parsedStack.push(item);
            consumeFrom();
        }
    }
}

void Larxer::consumeUpdate()
{
    currentQuery->type = CMD_UPDATE;
    currentQuery->locktype = WRITELOCK;

    while (!parsedStack.empty())
    {
        stackmember_s item = popstack();

        if (item.type == TYPE_WHERE)
        {
            consumeWhere();
        }
        else
        {
            if (item.type != TYPE_assignment)
            {
                parsedStack.push(item);
                consumeFrom();
                return;
            }

            item = popstack(); // expression
            class Ast *expr = consumeExpression();
            stackmember_s item2 = popstack(); // identifier
            currentQuery->assignments[item2.val.substr(1, string::npos)] = expr;
        }
    }
}

void Larxer::consumeDelete()
{
    currentQuery->type = CMD_DELETE;
    currentQuery->locktype = WRITELOCK;

    while (!parsedStack.empty())
    {
        stackmember_s item = popstack();

        if (item.type == TYPE_WHERE)
        {
            consumeWhere();
        }
        else
        {
            parsedStack.push(item);
            consumeFrom();
        }
    }
}

void Larxer::consumeCreate()
{
    currentQuery->type = CMD_CREATE;
    currentQuery->locktype = WRITELOCK;

    while (!parsedStack.empty())
    {
        stackmember_s item = popstack();


    }
}

void Larxer::consumeDrop()
{
    currentQuery->type = CMD_DROP;
    currentQuery->locktype = WRITELOCK;

    while (!parsedStack.empty())
    {
        stackmember_s item = popstack();

    }
}


void Larxer::consumeAlter()
{
    currentQuery->type = CMD_ALTER;
    currentQuery->locktype = WRITELOCK;

    while (!parsedStack.empty())
    {
        stackmember_s item = popstack();

    }
}


void Larxer::consumeColumns(int64_t numcolumns)
{
    for (int64_t n=0; n < numcolumns; n++)
    {
        if (parsedStack.empty())
        {
            printf("%s %i anomaly parsedStack empty\n", __FILE__, __LINE__);
            return;
        }

        stackmember_s item = popstack();

        switch (item.type)
        {
        case TYPE_ASTERISK:
            currentQuery->fromColumns.push_back("*");
            break;

        case TYPE_operand:
            currentQuery->fromColumns.push_back(item.val);
            break;

        default:
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, item.type);
        }
    }
}

void Larxer::consumeFrom()
{
    if (!parsedStack.empty())
    {
        stackmember_s item = popstack();

        if (item.type==TYPE_operand)
        {
            currentQuery->table = item.val.substr(1, string::npos);
        }
        else
        {
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, item.type);
        }
    }
    else
    {
        printf("%s %i anomaly parsedStack empty\n", __FILE__, __LINE__);
    }
}

void Larxer::consumeWhere()
{
    currentQuery->haswhere = true;

    if (!parsedStack.empty())
    {
        stackmember_s item = popstack();

        if (item.type==TYPE_search_condition)
        {
            currentQuery->searchCondition = consumeExpression();
        }
        else
        {
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, item.type);
        }
    }
    else
    {
        printf("%s %i anomaly parsedStack empty\n", __FILE__, __LINE__);
    }
}

void Larxer::consumeGroupby()
{
    currentQuery->hasgroupby = true;

    while (1)
    {
        if (parsedStack.empty())
        {
            return;
        }

        stackmember_s item = popstack();

        if (item.type != TYPE_operand)
        {
            parsedStack.push(item);
            return;
        }

        if (item.val[0] != OPERAND_IDENTIFIER)
        {
            printf("%s %i anomaly %c\n", __FILE__, __LINE__, item.val[0]);
            return;
        }

        currentQuery->groupByList.push_back(item.val.substr(1, string::npos));
    }
}

void Larxer::consumeHaving()
{
    currentQuery->hashaving = true;

    if (!parsedStack.empty())
    {
        stackmember_s item = popstack();

        if (item.type==TYPE_search_condition)
        {
            currentQuery->searchCondition = consumeExpression();
        }
        else
        {
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, item.type);
        }
    }
    else
    {
        printf("%s %i anomaly parsedStack empty\n", __FILE__, __LINE__);
    }
}

void Larxer::consumeOrderby()
{
    currentQuery->hasorderby = true;

    while (1)
    {
        stackmember_s item = popstack();

        if (item.type != TYPE_SORTSPECIFICATION)
        {
            parsedStack.push(item);
            return;
        }

        item = popstack();
        stackmember_s item2 = popstack();

        if (item2.type == TYPE_ASC)
        {
            currentQuery->orderbylist.push_back({true, item.val});
        }
        else
        {
            currentQuery->orderbylist.push_back({false, item.val});
        }
    }
}

void Larxer::consumeStoredProcedure()
{
    currentQuery->type=CMD_STOREDPROCEDURE;
    stackmember_s item;

    while (!parsedStack.empty())
    {
        item=popstack();
        currentQuery->storedProcedureArgs.push_back(item.val);
    }

    currentQuery->storedProcedureArgs.pop_back();
    currentQuery->storedProcedure=item.val.substr(1, string::npos);
    std::reverse(currentQuery->storedProcedureArgs.begin(),
                 currentQuery->storedProcedureArgs.end());

    return;
}
