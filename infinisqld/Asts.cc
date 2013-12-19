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
 * @file   Asts.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:04:48 2013
 * 
 * @brief  Abstract Syntax Tree class and Statement class. A SQL statement
 * turns into these in order to be executed.
 */

#include "gch.h"
#include "Asts.h"
#include "infinisql.h"
#include "Transaction.h"
#line 34 "Asts.cc"

Ast::Ast()
{
}

Ast::Ast(class Ast *parentarg,
         operatortypes_e operatortypearg) :
    parent(parentarg), rightchild(NULL), leftchild(NULL), isoperator(true),
    operatortype(operatortypearg)
{
    // for unary operators, make left appear populated already
    if (operatortype==OPERATOR_NEGATION || operatortype==OPERATOR_NOT ||
        operatortype==OPERATOR_TRUE || operatortype==OPERATOR_FALSE ||
        operatortype==OPERATOR_UNKNOWN || operatortype==OPERATOR_ISNULL ||
        operatortype==OPERATOR_IN || operatortype==OPERATOR_EXISTS ||
        operatortype==OPERATOR_UNIQUE || operatortype==OPERATOR_ISNOTNULL ||
        operatortype==OPERATOR_NOTIN)
    {
        leftchild = this;
    }
}

Ast::Ast(class Ast *parentarg, string &operandarg) : parent(parentarg),
                               rightchild(NULL), leftchild(NULL),
                               isoperator(false),
                               operand(operandarg)
{
}

Ast::Ast(const Ast &orig)
{
    cp(orig);
}

Ast &Ast::operator= (const Ast &orig)
{
    cp(orig);
    return *this;
}

void Ast::cp(const Ast &orig)
{
    parent = orig.parent;
    isoperator = orig.isoperator;
    operatortype = orig.operatortype;
    operand = orig.operand;
    predicateResults = orig.predicateResults;

    if (orig.leftchild != NULL)
    {
        leftchild = new class Ast;
        *leftchild = *orig.leftchild;
    }
    else
    {
        leftchild=NULL;
    }

    if (orig.rightchild != NULL)
    {
        rightchild = new class Ast;
        *rightchild = *orig.rightchild;
    }
    else
    {
        rightchild=NULL;
    }
}

Ast::~Ast()
{
    if (isoperator==true)
    {
        if (leftchild != NULL)
        {
            if (leftchild != this)
            {
                delete leftchild;
            }
        }

        if (rightchild != NULL)
        {
            delete rightchild;
        }
    }
}

bool Ast::evaluate(class Ast **nextAstNode, class Statement *statementPtr)
{
    if (isoperator==false)
    {
        if (operand[0]==OPERAND_PARAMETER)
        {
            int64_t paramnum;
            memcpy(&paramnum, &operand[1], sizeof(paramnum));
            operand = statementPtr->parameters[paramnum];
        }

        *nextAstNode=parent;
        return true;
    }

    /* do left 1st, particularly for AND optimization */

    // unary operators ignore leftchild
    if (operatortype != OPERATOR_NEGATION && operatortype != OPERATOR_NOT &&
        operatortype != OPERATOR_TRUE && operatortype != OPERATOR_FALSE &&
        operatortype != OPERATOR_UNKNOWN && operatortype != OPERATOR_ISNULL &&
        operatortype != OPERATOR_IN && operatortype != OPERATOR_EXISTS &&
        operatortype != OPERATOR_UNIQUE && operatortype != OPERATOR_ISNOTNULL &&
        operatortype != OPERATOR_NOTIN)
    {
        if (leftchild->isoperator==true)
        {
            *nextAstNode=leftchild;
            return false;
        }

        if (leftchild->operand[0]==OPERAND_PARAMETER)
        {
            int64_t paramnum;
            memcpy(&paramnum, &leftchild->operand[1], sizeof(paramnum));
            printf("%s %i paramnum %li\n", __FILE__, __LINE__, paramnum);
            leftchild->operand = statementPtr->parameters[paramnum];
        }
        else if (leftchild->operand[0]==OPERAND_SUBQUERY)
        {
            statementPtr->subqueryScalar(this);
        }
    }

    if (rightchild->isoperator==true)
    {
        *nextAstNode=rightchild;
        return false;
    }

    if (rightchild->operand[0]==OPERAND_PARAMETER)
    {
        int64_t paramnum;
        memcpy(&paramnum, &rightchild->operand[1], sizeof(paramnum));
        rightchild->operand = statementPtr->parameters[paramnum];
    }
    else if (rightchild->operand[0]==OPERAND_SUBQUERY)
    {
        statementPtr->subqueryScalar(this);
    }

    // execute!
    switch (operatortype)
    {
    case OPERATOR_CONCATENATION:
    {
        if (leftchild->operand[0] != OPERAND_STRING ||
            rightchild->operand[0] != OPERAND_STRING)
        {
            printf("%s %i bad operand %c\n", __FILE__, __LINE__, operand[0]);
            return true;
        }

        operand=OPERAND_STRING;
        operand.append(leftchild->operand.substr(1, string::npos));
        operand.append(rightchild->operand.substr(1, string::npos));
        isoperator=false;
        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
    }
    break;

    case OPERATOR_ADDITION:
        if (leftchild->operand[0]==OPERAND_FLOAT ||
            rightchild->operand[0]==OPERAND_FLOAT)
        {
            toFloat(leftchild->operand, leftchild->operand);
            toFloat(rightchild->operand, rightchild->operand);
        }

        switch (leftchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val = *(int64_t *)(leftchild->operand.c_str()+1) +
                *(int64_t *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            int64_t val = *(long double *)(leftchild->operand.c_str()+1) +
                *(long double *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        default:
            printf("%s %i not an arithmetic type %c\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return false;
        }

        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
        break;

    case OPERATOR_SUBTRACTION:
        if (leftchild->operand[0]==OPERAND_FLOAT ||
            rightchild->operand[0]==OPERAND_FLOAT)
        {
            toFloat(leftchild->operand, leftchild->operand);
            toFloat(rightchild->operand, rightchild->operand);
        }

        switch (leftchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val = *(int64_t *)(leftchild->operand.c_str()+1) -
                *(int64_t *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            int64_t val = *(long double *)(leftchild->operand.c_str()+1) -
                *(long double *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        default:
            printf("%s %i not an arithmetic type %c\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return false;
        }

        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
        break;

    case OPERATOR_MULTIPLICATION:
        if (leftchild->operand[0]==OPERAND_FLOAT ||
            rightchild->operand[0]==OPERAND_FLOAT)
        {
            toFloat(leftchild->operand, leftchild->operand);
            toFloat(rightchild->operand, rightchild->operand);
        }

        switch (leftchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val = *(int64_t *)(leftchild->operand.c_str()+1) *
                *(int64_t *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            int64_t val = *(long double *)(leftchild->operand.c_str()+1) *
                *(long double *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        default:
            printf("%s %i not an arithmetic type %c\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return false;
        }

        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
        break;

    case OPERATOR_DIVISION:
        if (leftchild->operand[0]==OPERAND_FLOAT ||
            rightchild->operand[0]==OPERAND_FLOAT)
        {
            toFloat(leftchild->operand, leftchild->operand);
            toFloat(rightchild->operand, rightchild->operand);
        }

        switch (leftchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val = *(int64_t *)(leftchild->operand.c_str()+1) /
                *(int64_t *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            int64_t val = *(long double *)(leftchild->operand.c_str()+1) /
                *(long double *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        default:
            printf("%s %i not an arithmetic type %c\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return false;
        }

        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
        break;

    case OPERATOR_NEGATION:
        if (rightchild->operand[0]==OPERAND_FLOAT)
        {
            toFloat(rightchild->operand, rightchild->operand);
        }

        switch (rightchild->operand[0])
            //      switch (leftchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val = 0 - *(int64_t *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            int64_t val = 0 - *(long double *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        default:
            printf("%s %i not an arithmetic type %c\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return false;
        }

        delete rightchild;
        rightchild=NULL;
        break;

    case OPERATOR_AND:
    {
        boost::unordered_map< int64_t, vector<int64_t> > enginerowids;

        boost::unordered_map<uuRecord_s, returnRow_s>::iterator it;

        for (it = leftchild->predicateResults.begin();
             it != leftchild->predicateResults.end(); it++)
        {
            if (rightchild->predicateResults.count(it->first))
            {
                predicateResults[it->first] = it->second;
            }
            else
            {
                // unlock if it's not already staged in this transaction
                if (!statementPtr->transactionPtr->stagedRows.count(it->first))
                {
                    enginerowids[it->first.engineid].push_back(it->first.rowid);
                }
            }
        }

        // send rollback messages
        boost::unordered_map< int64_t, vector<int64_t> >::iterator it2;
        rowOrField_s rowOrField = {};
        rowOrField.isrow=true;
        rowOrField.tableid=statementPtr->currentQuery->tableid;

        for (it2 = enginerowids.begin(); it2 != enginerowids.end(); it2++)
        {
            class MessageCommitRollback *msg = new class MessageCommitRollback();

            for (size_t n=0; n < it2->second.size(); n++)
            {
                rowOrField.rowid=it2->second[n];
                msg->rofs.push_back(rowOrField);
            }

            statementPtr->transactionPtr->sendTransaction(ROLLBACKCMD,
                                                          PAYLOADCOMMITROLLBACK,
                                                          0, it2->first, msg);
        }

        isoperator=false;
        printf("%s %i this %p deleting leftchild %p rightchild %p\n", __FILE__,
               __LINE__, this, leftchild, rightchild);
        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
    }
    break;

    case OPERATOR_OR:
        predicateResults = leftchild->predicateResults;
        predicateResults.insert(rightchild->predicateResults.begin(),
                                rightchild->predicateResults.end());
        isoperator=false;
        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
        break;

    case OPERATOR_NOT:
        /* NOT requires a call to retrieve all rows NOT in the
         * operand's result set */
        printf("%s %i operator %i not implemented.\n", __FILE__, __LINE__,
               operatortype);
        isoperator=false;
        delete rightchild;
        rightchild=NULL;
        return false;
//        break;

    case OPERATOR_TRUE:
        /* IS TRUE is gratuitous*/
        printf("%s %i operator %i not implemented.\n", __FILE__, __LINE__,
               operatortype);
        isoperator=false;
        delete rightchild;
        rightchild=NULL;
        return false;
//        break;

    case OPERATOR_FALSE:
        /* IS FALSE requires a call to retrieve all rows like NOT, and
         * all NOT NULL */
        printf("%s %i operator %i not implemented.\n", __FILE__, __LINE__,
               operatortype);
        isoperator=false;
        delete rightchild;
        rightchild=NULL;
        return false;
//        break;

    case OPERATOR_UNKNOWN:
        /* IS UNKNOWN is a variation on ISNULL predicates */
        printf("%s %i operator %i not implemented.\n", __FILE__, __LINE__,
               operatortype);
        isoperator=false;
        delete rightchild;
        rightchild=NULL;
        return false;
//        break;

    case OPERATOR_EQ:
        isoperator=false;

        if (statementPtr->stagedPredicate(operatortype,
                                          statementPtr->currentQuery->tableid,
                                          leftchild->operand,
                                          rightchild->operand,
                                          statementPtr->currentQuery->results.inValues,
                                          statementPtr->transactionPtr->stagedRows,
                                          predicateResults)==false)
        {
            /*
             * if (parent != NULL && parent->operatortype==OPERATOR_AND &&
             * this==parent->rightchild)
             */
            if (0) // revert this back when andPredicate is tested
            {
                statementPtr->andPredicate(operatortype,
                                           statementPtr->currentQuery->tableid,
                                           leftchild->operand,
                                           rightchild->operand,
                                           statementPtr->currentQuery->results.inValues,
                                           parent->leftchild->predicateResults, predicateResults);
                delete leftchild;
                leftchild=NULL;
                delete rightchild;
                rightchild=NULL;
            }
            else
            {
                statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                           operatortype,
                                                           statementPtr->currentQuery->tableid,
                                                           leftchild->operand,
                                                           rightchild->operand,
                                                           statementPtr->currentQuery->locktype,
                                                           statementPtr->currentQuery->results.inValues,
                                                           this,
                                                           predicateResults);
                *nextAstNode = NULL;
                delete leftchild;
                leftchild=NULL;
                delete rightchild;
                rightchild=NULL;
                return false;
            }
        }
        else
        {
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
        }

        break;

    case OPERATOR_NE:
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       leftchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults, predicateResults);
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       leftchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_LT:
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       leftchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults, predicateResults);
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       leftchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_GT:
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       leftchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults,
                                       predicateResults);
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       leftchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_LTE:
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       leftchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults, predicateResults);
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       leftchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_GTE:
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand, rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       leftchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults, predicateResults);
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       leftchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_BETWEEN:
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       leftchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults,
                                       predicateResults);
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       leftchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_NOTBETWEEN:
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       leftchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults,
                                       predicateResults);
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       leftchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_ISNULL: // unary operator
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       rightchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults, predicateResults);
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       rightchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_ISNOTNULL: // unary operator
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       rightchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults, predicateResults);
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       rightchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_IN: // unary operator
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        // resolve inobject
        if (statementPtr->currentQuery->inobject.issubquery==true)
        {
            statementPtr->subqueryIn(this);
        }
        else
        {
            // fieldid is in the rightoperand, then get type
            int64_t fieldid;
            memcpy(&fieldid, &rightchild->operand[1], sizeof(fieldid));
            fieldtype_e fieldtype = statementPtr->schemaPtr->tables[statementPtr->currentQuery->tableid]->fields[fieldid].type;

            for (size_t n=0;
                 n < statementPtr->currentQuery->inobject.expressionlist.size();
                 n++)
            {
                fieldValue_s fieldValue = {};
                class Ast &astRef =
                    *statementPtr->currentQuery->inobject.expressionlist[n];
                statementPtr->searchExpression(0, &astRef);

                if (astRef.operand[0]==OPERAND_NULL)
                {
                    fieldValue.isnull=true;
                }
                else
                {
                    switch (fieldtype)
                    {
                    case INT:
                        memcpy(&fieldValue.value.integer, &astRef.operand[1],
                               sizeof(int64_t));
                        break;

                    case UINT:
                        memcpy(&fieldValue.value.uinteger, &astRef.operand[1],
                               sizeof(int64_t));
                        break;

                    case BOOL:
                    {
                        int64_t torf;
                        memcpy(&torf, &astRef.operand[1], sizeof(torf));
                        fieldValue.value.boolean = (bool)torf;
                    }
                    break;

                    case FLOAT:
                        toFloat(astRef.operand, fieldValue);
                        /*
                          memcpy(&fieldValue.value.floating, &astRef.operand[1],
                          sizeof(long double));
                        */
                        break;

                    case CHAR:
                        fieldValue.value.character = astRef.operand[1];
                        break;

                    case CHARX:
                        fieldValue.str = astRef.operand.substr(1, string::npos);
                        break;

                    case VARCHAR:
                        fieldValue.str = astRef.operand.substr(1, string::npos);
                        break;

                    default:
                        printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                               fieldtype);
                    }
                }

                statementPtr->currentQuery->results.inValues.push_back(fieldValue);
                delete &astRef;
            }

            statementPtr->currentQuery->inobject.expressionlist.clear();
        }

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       rightchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults,
                                       predicateResults);
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       rightchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_NOTIN: // unary operator
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        // resolve inobject
        if (statementPtr->currentQuery->inobject.issubquery==true)
        {
            statementPtr->subqueryIn(this);
        }
        else
        {
            // fieldid is in the rightoperand, then get type
            int64_t fieldid;
            memcpy(&fieldid, &rightchild->operand[1], sizeof(fieldid));
            fieldtype_e fieldtype =
                statementPtr->schemaPtr->tables[statementPtr->currentQuery->tableid]->fields[fieldid].type;

            for (size_t n=0;
                 n < statementPtr->currentQuery->inobject.expressionlist.size();
                 n++)
            {
                fieldValue_s fieldValue = {};
                class Ast &astRef =
                    *statementPtr->currentQuery->inobject.expressionlist[n];
                statementPtr->searchExpression(0, &astRef);

                if (astRef.operand[0]==OPERAND_NULL)
                {
                    fieldValue.isnull=true;
                }
                else
                {
                    switch (fieldtype)
                    {
                    case INT:
                        memcpy(&fieldValue.value.integer, &astRef.operand[1],
                               sizeof(int64_t));
                        break;

                    case UINT:
                        memcpy(&fieldValue.value.uinteger, &astRef.operand[1],
                               sizeof(int64_t));
                        break;

                    case BOOL:
                    {
                        int64_t torf;
                        memcpy(&torf, &astRef.operand[1], sizeof(torf));
                        fieldValue.value.boolean = (bool)torf;
                    }
                    break;

                    case FLOAT:
                        toFloat(astRef.operand, fieldValue);
                        break;

                    case CHAR:
                        fieldValue.value.character = astRef.operand[1];
                        break;

                    case CHARX:
                        fieldValue.str = astRef.operand.substr(1, string::npos);
                        break;

                    case VARCHAR:
                        fieldValue.str = astRef.operand.substr(1, string::npos);
                        break;

                    default:
                        printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                               fieldtype);
                    }
                }

                statementPtr->currentQuery->results.inValues.push_back(fieldValue);
                delete &astRef;
            }

            statementPtr->currentQuery->inobject.expressionlist.clear();
        }

        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       rightchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults,
                                       predicateResults);
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       rightchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_LIKE:
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       leftchild->operand,
                                       rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults,
                                       predicateResults);
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            string str=rightchild->operand.substr(1, string::npos);
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       leftchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_NOTLIKE:
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        isoperator=false;

        /*
          if (parent != NULL && parent->operatortype==OPERATOR_AND &&
          this==parent->rightchild)
        */
        if (0) // revert this back when andPredicate is tested
        {
            statementPtr->andPredicate(operatortype,
                                       statementPtr->currentQuery->tableid,
                                       leftchild->operand, rightchild->operand,
                                       statementPtr->currentQuery->results.inValues,
                                       parent->leftchild->predicateResults,
                                       predicateResults);
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
        }
        else
        {
            statementPtr->transactionPtr->sqlPredicate(statementPtr,
                                                       operatortype,
                                                       statementPtr->currentQuery->tableid,
                                                       leftchild->operand,
                                                       rightchild->operand,
                                                       statementPtr->currentQuery->locktype,
                                                       statementPtr->currentQuery->results.inValues,
                                                       this, predicateResults);
            *nextAstNode = NULL;
            delete leftchild;
            leftchild=NULL;
            delete rightchild;
            rightchild=NULL;
            return false;
        }

        break;

    case OPERATOR_EXISTS: // unary operator
    {
        // need to process subqueries first
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        statementPtr->subqueryExists(this);
        delete rightchild;
        rightchild=NULL;
    }
    break;

    case OPERATOR_UNIQUE: // unary operator
    {
        //need to process subqueries first
        statementPtr->stagedPredicate(operatortype,
                                      statementPtr->currentQuery->tableid,
                                      leftchild->operand,
                                      rightchild->operand,
                                      statementPtr->currentQuery->results.inValues,
                                      statementPtr->transactionPtr->stagedRows,
                                      predicateResults);

        statementPtr->subqueryUnique(this);
        delete rightchild;
        rightchild=NULL;
    }
    break;

    case OPERATOR_BETWEENAND:
        switch (leftchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+2*sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            memcpy(&operand[1], &leftchild->operand[1], sizeof(int64_t));
            memcpy(&operand[1+sizeof(int64_t)], &rightchild->operand[1],
                   sizeof(int64_t));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+2*sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            memcpy(&operand[1], &leftchild->operand[1], sizeof(long double));
            memcpy(&operand[1+sizeof(long double)], &rightchild->operand[1],
                   sizeof(long double));
            isoperator=false;
        }
        break;

        case OPERAND_STRING:
        {
            if (rightchild->operand[0] != OPERAND_STRING)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return false;
            }

            operand.resize(1+sizeof(size_t)+(leftchild->operand.size()-1) +
                           (rightchild->operand.size()-1), (char)0);
            operand[0] = OPERAND_STRING;
            size_t len = leftchild->operand.size()-1;
            memcpy(&operand[1], &len, sizeof(len));
            memcpy(&operand[1+sizeof(len)], &leftchild->operand[1], len);
            memcpy(&operand[1+sizeof(len)+len], &rightchild->operand[1],
                   rightchild->operand.size()-1);
            isoperator=false;
        }
        break;

        default:
            printf("%s %i bad type %c\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return false;
        }

        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
        break;

    default:
        printf("%s %i anomaly %i\n", __FILE__, __LINE__, operatortype);
    }

    *nextAstNode = parent;
    return true;
}

// to do updates like: set fieldA=fieldA+5
void Ast::evaluateAssignment(vector<fieldValue_s> &fieldValues,
                             class Statement *statementPtr)
{
    if (isoperator==false)
    {
        normalizeAssignmentOperand(fieldValues, statementPtr);
        return;
    }

    if (operatortype != OPERATOR_NEGATION) // not unary operator
    {
        leftchild->evaluateAssignment(fieldValues, statementPtr);
    }

    rightchild->evaluateAssignment(fieldValues, statementPtr);

    switch (operatortype)
    {
    case OPERATOR_CONCATENATION:
    {
        if (leftchild->operand[0] != OPERAND_STRING ||
            rightchild->operand[0] != OPERAND_STRING)
        {
            printf("%s %i bad operand %c\n", __FILE__, __LINE__, operand[0]);
            return;
        }

        operand=OPERAND_STRING;
        operand.append(leftchild->operand.substr(1, string::npos));
        operand.append(rightchild->operand.substr(1, string::npos));
        isoperator=false;
        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
    }
    break;

    case OPERATOR_ADDITION:
        if (leftchild->operand[0]==OPERAND_FLOAT ||
            rightchild->operand[0]==OPERAND_FLOAT)
        {
            toFloat(leftchild->operand, leftchild->operand);
            toFloat(rightchild->operand, rightchild->operand);
        }

        switch (leftchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return;
            }

            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val = *(int64_t *)(leftchild->operand.c_str()+1) +
                *(int64_t *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return;
            }

            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            int64_t val = *(long double *)(leftchild->operand.c_str()+1) +
                *(long double *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        default:
            printf("%s %i not an arithmetic type %c\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return;
        }

        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
        break;

    case OPERATOR_SUBTRACTION:
        if (leftchild->operand[0]==OPERAND_FLOAT ||
            rightchild->operand[0]==OPERAND_FLOAT)
        {
            toFloat(leftchild->operand, leftchild->operand);
            toFloat(rightchild->operand, rightchild->operand);
        }

        switch (leftchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return;
            }

            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val = *(int64_t *)(leftchild->operand.c_str()+1) -
                *(int64_t *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return;
            }

            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            int64_t val = *(long double *)(leftchild->operand.c_str()+1) -
                *(long double *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        default:
            printf("%s %i not an arithmetic type %c\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return;
        }

        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
        break;

    case OPERATOR_MULTIPLICATION:
        if (leftchild->operand[0]==OPERAND_FLOAT ||
            rightchild->operand[0]==OPERAND_FLOAT)
        {
            toFloat(leftchild->operand, leftchild->operand);
            toFloat(rightchild->operand, rightchild->operand);
        }

        switch (leftchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return;
            }

            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val = *(int64_t *)(leftchild->operand.c_str()+1) *
                *(int64_t *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return;
            }

            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            int64_t val = *(long double *)(leftchild->operand.c_str()+1) *
                *(long double *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        default:
            printf("%s %i not an arithmetic type %c\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return;
        }

        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
        break;

    case OPERATOR_DIVISION:
        if (leftchild->operand[0]==OPERAND_FLOAT ||
            rightchild->operand[0]==OPERAND_FLOAT)
        {
            toFloat(leftchild->operand, leftchild->operand);
            toFloat(rightchild->operand, rightchild->operand);
        }

        switch (leftchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return;
            }

            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val = *(int64_t *)(leftchild->operand.c_str()+1) /
                *(int64_t *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return;
            }

            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            int64_t val = *(long double *)(leftchild->operand.c_str()+1) /
                *(long double *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        default:
            printf("%s %i not an arithmetic type %c\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return;
        }

        delete leftchild;
        leftchild=NULL;
        delete rightchild;
        rightchild=NULL;
        break;

    case OPERATOR_NEGATION:
        if (rightchild->operand[0]==OPERAND_FLOAT)
        {
            toFloat(rightchild->operand, rightchild->operand);
        }

        switch (rightchild->operand[0])
        {
        case OPERAND_INTEGER:
        {
            if (rightchild->operand[0] != OPERAND_INTEGER)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return;
            }

            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val = 0 -
                *(int64_t *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        case OPERAND_FLOAT:
        {
            if (rightchild->operand[0] != OPERAND_FLOAT)
            {
                printf("%s %i operand mismatch %c %i\n", __FILE__, __LINE__,
                       leftchild->operand[0], rightchild->operand[0]);
                return;
            }

            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            int64_t val = 0 -
                *(long double *)(rightchild->operand.c_str()+1);
            memcpy(&operand[1], &val, sizeof(val));
            isoperator=false;
        }
        break;

        default:
            printf("%s %i not an arithmetic type '%c'\n", __FILE__, __LINE__,
                   leftchild->operand[0]);
            return;
        }

        delete rightchild;
        rightchild=NULL;
        break;

    default:
        printf("%s %i can't evaluate %i in set assignment\n", __FILE__, __LINE__,
               operatortype);
    }
}

void Ast::normalizeSetAssignmentOperand(vector<fieldValue_s> &fieldValues,
                                        class Statement *statementPtr)
{
    switch (operand[0])
    {
    case OPERAND_FIELDID:
    {
        int64_t fieldid;
        memcpy(&fieldid, &operand[1], sizeof(fieldid));

        switch (statementPtr->schemaPtr->tables[statementPtr->currentQuery->tableid]->fields[fieldid].type)
        {
        case INT:
            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            memcpy(&operand[1], &fieldValues[fieldid].value.integer,
                   sizeof(int64_t));
            break;

        case UINT:
            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            memcpy(&operand[1], &fieldValues[fieldid].value.uinteger,
                   sizeof(int64_t));
            break;

        case BOOL:
            operand.resize(1+sizeof(int64_t), (char)0);
            operand[0] = OPERAND_INTEGER;
            int64_t val;

            if (fieldValues[fieldid].value.boolean==true)
            {
                val=1;
            }
            else
            {
                val=0;
            }

            memcpy(&operand[1], &val, sizeof(int64_t));
            break;

        case FLOAT:
            operand.resize(1+sizeof(long double), (char)0);
            operand[0] = OPERAND_FLOAT;
            memcpy(&operand[1], &fieldValues[fieldid].value.floating,
                   sizeof(int64_t));
            break;

        case CHAR:
            operand.resize(1+sizeof(char), (char)0);
            operand[0] = OPERAND_STRING;
            operand[1] = fieldValues[fieldid].value.character;
            break;

        case CHARX:
            operand.assign(1, OPERAND_STRING);
            operand.append(fieldValues[fieldid].str);
            break;

        case VARCHAR:
            operand.assign(1, OPERAND_STRING);
            operand.append(fieldValues[fieldid].str);
            break;

        default:
            printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                   statementPtr->schemaPtr->tables[statementPtr->currentQuery->tableid]->fields[fieldid].type);
        }
    }
    break;

    case OPERAND_SUBQUERY:
        statementPtr->subqueryScalar(this);
        break;

    case OPERAND_PARAMETER:
    {
        int64_t paramnum;
        memcpy(&paramnum, &operand[1], sizeof(paramnum));
        operand = statementPtr->parameters[paramnum];
    }
    break;

    default:
        return;
    }
}

void Ast::toFloat(const string &inoperand, fieldValue_s &outField)
{
    switch (inoperand[0])
    {
    case OPERAND_INTEGER:
    {
        int64_t val;
        memcpy(&val, &inoperand[1], sizeof(val));
        outField.value.floating=(long double)val;
    }
    break;

    case OPERAND_FLOAT:
        memcpy(&outField.value.floating, &inoperand[1], sizeof(long double));
        break;

    default:
        outField.value.floating=0;
    }
}

void Ast::toFloat(const string &inoperand, string &outoperand)
{
    string instr=inoperand;

    switch (instr[0])
    {
    case OPERAND_INTEGER:
    {
        outoperand.resize(1+sizeof(int64_t), OPERAND_FLOAT);
        int64_t val;
        memcpy(&val, &instr[1], sizeof(val));
        memcpy(&outoperand[1], &val, sizeof(val));
    }
    break;

    case OPERAND_FLOAT:
        outoperand=instr;
        break;

    default:
        outoperand.resize(1+sizeof(long double), OPERAND_FLOAT);
        long double val;
        memcpy(&val, &instr[1], sizeof(val));
        memcpy(&outoperand[1], &val, sizeof(val));
    }
}

Statement::Statement()
{
}

Statement::Statement(class TransactionAgent *taPtrarg,
                     class Schema *schemaPtrarg) :
    taPtr(taPtrarg), schemaPtr(schemaPtrarg), transactionPtr(NULL),
    currentQuery(NULL)
{
    reentry = reentry_s();
}

Statement::Statement(const Statement &orig)
{
    cp(orig);
}

Statement &Statement::operator= (const Statement &orig)
{
    cp(orig);
    return *this;
}

/* copy of a statement should occur only prior to execution */
void Statement::cp(const Statement &orig)
{
    taPtr = orig.taPtr;
    schemaPtr = orig.schemaPtr;
    reentry = orig.reentry;
    transactionPtr = orig.transactionPtr;
    currentQuery = orig.currentQuery;

    for (size_t n=0; n < orig.queries.size(); n++)
    {
        queries.push_back(cpquery(orig.queries[n]));
    }

    parameters = orig.parameters;
    queryindex = orig.queryindex;
}

Statement::query_s Statement::cpquery(const query_s &orig)
{
    query_s newstmt;

    newstmt.instance = orig.instance;
    newstmt.type = orig.type;
    newstmt.isforupdate = orig.isforupdate;
    newstmt.hasnolock = orig.hasnolock;
    newstmt.haswhere = orig.haswhere;
    newstmt.hasgroupby= orig.hasgroupby;
    newstmt.hashaving = orig.hashaving;
    newstmt.hasorderby = orig.hasorderby;
    newstmt.table = orig.table;
    newstmt.tableid = orig.tableid;
    newstmt.locktype = orig.locktype;
    newstmt.groupByList = orig.groupByList;
    newstmt.fromColumns = orig.fromColumns;
    newstmt.fromColumnids = orig.fromColumnids;
    newstmt.orderbylist = orig.orderbylist;

    newstmt.inobject.issubquery = orig.inobject.issubquery;
    newstmt.inobject.subquery = orig.inobject.subquery;
    size_t siz = orig.inobject.expressionlist.size();
    newstmt.inobject.expressionlist.resize(siz, NULL);

    for (size_t n=0; n < siz; n++)
    {
        newstmt.inobject.expressionlist[n] = new class Ast;
        *newstmt.inobject.expressionlist[n] = *orig.inobject.expressionlist[n];
    }

    if (orig.searchCondition != NULL)
    {
        newstmt.searchCondition = new class Ast;
        *newstmt.searchCondition = *orig.searchCondition;
    }
    else
    {
        newstmt.searchCondition=NULL;
    }

    /* don't copy assignments, as fieldid assumedly has already been
     * resolved, and it makes keeping track of new Ast * objects difficult
     */

    boost::unordered_map<int64_t, class Ast *>::const_iterator it;

    for (it = orig.fieldidAssignments.begin();
         it != orig.fieldidAssignments.end(); it++)
    {

        newstmt.fieldidAssignments[it->first] = new class Ast;
        *newstmt.fieldidAssignments[it->first] = *it->second;
    }

    for (size_t n=0; n < orig.insertColumns.size(); n++)
    {
        newstmt.insertColumns.push_back(new class Ast);
        *newstmt.insertColumns[n] = *orig.insertColumns[n];
    }

    /* results should not need to be copied, as they are created only by
     * an executing statement
     */
    return newstmt;
}

Statement::~Statement()
{
    for (size_t n=0; n < queries.size(); n++)
    {
        query_s &queryRef = queries[n];

        if (queryRef.searchCondition != NULL)
        {
            delete queryRef.searchCondition;
        }

        for (size_t m=0; m < queryRef.inobject.expressionlist.size(); m++)
        {
            delete queryRef.inobject.expressionlist[m];
        }

        boost::unordered_map<int64_t, class Ast *>::iterator it;

        for (it = queryRef.fieldidAssignments.begin();
             it != queryRef.fieldidAssignments.end(); it++)
        {
            delete it->second;
        }

        for (size_t m=0; m < queryRef.insertColumns.size(); m++)
        {
            delete queryRef.insertColumns[m];
        }
    }
}

bool Statement::resolveTableFields()
{
    for (size_t n=0; n < queries.size(); n++)
    {
        currentQuery=&queries[n];

        switch (currentQuery->type)
        {
        case CMD_SELECT:
            break;

        case CMD_INSERT:
            break;

        case CMD_UPDATE:
            break;

        case CMD_DELETE:
            break;

        default: // noop, must be BEGIN, COMMIT, etc.
            return true;
        }

        if (resolveTableFields2()==false)
        {
            return false;
        }
    }

    return true;
}

bool Statement::resolveTableFields2()
{
    // resolve tableid
    if (schemaPtr->tableNameToId.count(currentQuery->table))
    {
        currentQuery->tableid =
            schemaPtr->tableNameToId[currentQuery->table];
    }
    else
    {
        return false;
    }

    class Table &tableRef = *schemaPtr->tables[currentQuery->tableid];

    // resolve fieldids in select columns
    if (currentQuery->type==CMD_SELECT)
    {
        for (ssize_t n=currentQuery->fromColumns.size()-1; n >= 0; n--)
        {
            string &fromColumnRef = currentQuery->fromColumns[n];

            switch (fromColumnRef[0])
            {
            case '*':
            {
                for (size_t n=0;
                     n < tableRef.fields.size();
                     n++)
                {
                    currentQuery->fromColumnids.push_back({OPERAND_FIELDID,
                                (int64_t)n, tableRef.fields[n].name
                                });
                }
            }
            break;

            case OPERAND_IDENTIFIER: // regular identifier
            {
                string fname = fromColumnRef.substr(1, string::npos);
                int64_t fid = getfieldid(currentQuery->tableid, fname);

                if (fid==-1)
                {
                    printf("%s %i tableid field not found %li %s\n", __FILE__,
                           __LINE__, currentQuery->tableid, fname.c_str());
                    return false;
                }
                else
                {
                    currentQuery->fromColumnids.push_back({OPERAND_FIELDID, fid,
                                tableRef.fields[fid].name
                                });
                }
            }
            break;

            case OPERAND_AGGREGATE: // aggregate
            {
                string fname = fromColumnRef.substr(2, string::npos);
                int64_t fid = getfieldid(currentQuery->tableid, fname);

                if (fid==-1)
                {
                    printf("%s %i tableid field not found %li %s\n", __FILE__,
                           __LINE__, currentQuery->tableid, fname.c_str());
                    return false;
                }
                else
                {
                    currentQuery->fromColumnids.push_back({fromColumnRef[1], fid,
                                tableRef.fields[fid].name
                                });
                }
            }
            break;

            default:
                printf("%s %i anomaly %c\n", __FILE__, __LINE__,
                       fromColumnRef[0]);
                return false;
            }
        }
    }
    else if (currentQuery->type==CMD_UPDATE)
    {
        boost::unordered_map<string, class Ast *>::iterator it;

        for (it = currentQuery->assignments.begin();
             it != currentQuery->assignments.end(); it++)
        {
            int64_t fid = getfieldid(currentQuery->tableid, it->first);

            if (fid==-1)
            {
                printf("%s %i tableid field not found %li %s\n", __FILE__,
                       __LINE__, currentQuery->tableid, it->first.c_str());
                return false;
            }
            else
            {
                currentQuery->fieldidAssignments[fid] = it->second;
            }

            if (resolveFieldNames(it->second)==false)
            {
                return false;
            }
        }
    }

    if (currentQuery->haswhere==true)
    {
        if (resolveFieldNames(currentQuery->searchCondition)==false)
        {
            return false;
        }
    }

    return true;
}

bool Statement::resolveFieldNames(class Ast *myPosition)
{
    direction_e direction = FROM_ABOVE;
    bool isbreakout=false;

    while (1)
    {
        if (myPosition->isoperator==false &&
            myPosition->operand[0]==OPERAND_IDENTIFIER)
        {
            // CONVERT
            string fname = myPosition->operand.substr(1, string::npos);
            int64_t fid = getfieldid(currentQuery->tableid, fname);

            if (fid==-1)
            {
                printf("%s %i tableid field not found %li %s\n", __FILE__,
                       __LINE__, currentQuery->tableid, fname.c_str());
                return false;
            }
            else
            {
                myPosition->operand.assign(1 + sizeof(fid), char(0));
                myPosition->operand[0] = OPERAND_FIELDID;
                memcpy(&myPosition->operand[1], &fid, sizeof(fid));
            }

            direction=FROM_RIGHT;
        }

        switch (direction)
        {
        case FROM_ABOVE:
            if (myPosition->leftchild==NULL || myPosition==myPosition->leftchild)
            {
                direction=FROM_LEFT;
            }
            else
            {
                myPosition=myPosition->leftchild;
                //          direction=FROM_ABOVE; already set
            }

            break;

        case FROM_LEFT:
            if (myPosition->rightchild==NULL)
            {
                direction=FROM_RIGHT;
            }
            else
            {
                myPosition=myPosition->rightchild;
                direction=FROM_ABOVE;
            }

            break;

        case FROM_RIGHT:
            if (myPosition->parent==NULL)
            {
                isbreakout=true;
            }
            else
            {
                if (myPosition==myPosition->parent->leftchild)
                {
                    direction=FROM_LEFT;
                }
                else // i am parent's right child
                {
                    direction=FROM_RIGHT;
                }

                myPosition=myPosition->parent;
            }

            break;

        default:
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, direction);
            return false;
        }

        if (isbreakout==true)
        {
            return true;
            //      break;
        }
    }
}

int64_t Statement::getfieldid(int64_t tableid, const string &fieldName)
{
    if (!schemaPtr->fieldNameToId.count(tableid))
    {
        return -1;
    }

    boost::unordered_map<string, int64_t> &ref =
        schemaPtr->fieldNameToId[tableid];
    boost::unordered_map<string, int64_t>::iterator it;
    it = ref.find(fieldName);

    if (it==ref.end())
    {
        return -1;
    }

    return ref[fieldName];
}

bool Statement::stagedPredicate(operatortypes_e op, int64_t tableid,
                                string &leftoperand, string &rightoperand,
                                vector<fieldValue_s> &inValues,
                                const boost::unordered_map<uuRecord_s,
                                stagedRow_s> &stagedRows,
                                boost::unordered_map<uuRecord_s,
                                returnRow_s> &results)
{
    bool equalhit=false;

    if (op != OPERATOR_IN && op != OPERATOR_NOTIN && op != OPERATOR_ISNULL &&
        op != OPERATOR_ISNOTNULL)
    {
        if (leftoperand[0] != OPERAND_FIELDID)
        {
            printf("%s %i left operand is not fieldid, it is '%c'\n",
                   __FILE__, __LINE__, leftoperand[0]);
            return equalhit;
        }
    }

    int64_t fieldid;
    memcpy(&fieldid, &leftoperand[1], sizeof(fieldid));

    class Table &tableRef = *schemaPtr->tables[tableid];
    vector<fieldValue_s> fieldValues;

    boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

    for (it = stagedRows.begin(); it != stagedRows.end(); it++)
    {
        const uuRecord_s &uurRef = it->first;

        if (uurRef.tableid != tableid)
        {
            continue;
        }

        const stagedRow_s &stagedRowRef = it->second;
        returnRow_s returnRow;
        stagedRow2ReturnRow(stagedRowRef, returnRow);

        tableRef.unmakerow((string *)&returnRow.row, &fieldValues);
        fieldValue_s &lhs = fieldValues[fieldid];

        switch (tableRef.fields[fieldid].type)
        {
        case INT:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                // indexRef.getequal
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer==rhsval)
                {
                    results[uurRef] = returnRow;
                    equalhit=true;
                }
            }
            break;

            case OPERATOR_NE:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer!=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_GT:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer>rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_LT:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer<rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_GTE:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer>=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_LTE:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer<=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_BETWEEN:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                int64_t rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.integer >= rhsval && lhs.value.integer <= rhsval2)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_NOTBETWEEN:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                int64_t rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.integer < rhsval || lhs.value.integer > rhsval2)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_IN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = rRow;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = rRow;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
                return equalhit;
            }
        }
        break;

        case BOOL:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                // indexRef.getequal
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean==rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_NE:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean!=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_GT:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean>rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_LT:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean<rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_GTE:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean>=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_LTE:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean<=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_BETWEEN:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                bool rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.boolean >= rhsval && lhs.value.boolean <= rhsval2)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_NOTBETWEEN:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                bool rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.boolean < rhsval || lhs.value.boolean > rhsval2)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_IN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = rRow;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = rRow;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
                return equalhit;
            }
        }
        break;

        case FLOAT:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                // indexRef.getequal
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating==rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_NE:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating!=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_GT:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating>rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_LT:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating<rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_GTE:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating>=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_LTE:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating<=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_BETWEEN:
            {
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                long double rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.floating >= rhsval && lhs.value.floating <=
                    rhsval2)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_NOTBETWEEN:
            {
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                long double rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.floating < rhsval || lhs.value.floating > rhsval2)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_IN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = rRow;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = rRow;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
                return equalhit;
            }
        }
        break;

        case CHAR:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                // indexRef.getequal
                char rhsval=rightoperand[1];

                if (lhs.value.character==rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_NE:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character!=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_GT:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character>rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_LT:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character<rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_GTE:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character>=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_LTE:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character<=rhsval)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_BETWEEN:
            {
                char rhsval=rightoperand[1+sizeof(int64_t)];
                char rhsval2=rightoperand[1+sizeof(int64_t)+1];

                if (lhs.value.character >= rhsval && lhs.value.character <=
                    rhsval2)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_NOTBETWEEN:
            {
                char rhsval=rightoperand[1+sizeof(int64_t)];
                char rhsval2=rightoperand[1+sizeof(int64_t)+1];

                if (lhs.value.character < rhsval || lhs.value.character >
                    rhsval2)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_IN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = rRow;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = rRow;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
                return equalhit;
            }
        }
        break;

        case CHARX:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                // indexRef.getequal
                string rhsval=rightoperand.substr(1, string::npos);

                if (!lhs.str.compare(rhsval))
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_NE:
            {
                string rhsval=rightoperand.substr(1, string::npos);

                if (lhs.str.compare(rhsval))
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_IN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = rRow;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = rRow;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
                return equalhit;
            }
        }
        break;

        case VARCHAR:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                // indexRef.getequal
                string rhsval=rightoperand.substr(1, string::npos);

                if (!lhs.str.compare(rhsval))
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_NE:
            {
                string rhsval=rightoperand.substr(1, string::npos);

                if (lhs.str.compare(rhsval))
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_IN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = rRow;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

                for (it = stagedRows.begin(); it != stagedRows.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const stagedRow_s &sRowRef = it->second;
                    returnRow_s rRow;
                    stagedRow2ReturnRow(sRowRef, rRow);
                    tableRef.unmakerow(&rRow.row, &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = rRow;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRow;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
                return equalhit;
            }
        }
        break;

        default:
            printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                   tableRef.fields[fieldid].type);
            return equalhit;
        }
    }

    return equalhit;
}

/* to be called for predicates ANDed with results of other predicates
 * this avoids traffic to engines */
void Statement::andPredicate(operatortypes_e op, int64_t tableid,
                             string &leftoperand, string &rightoperand,
                             vector<fieldValue_s> &inValues,
                             const boost::unordered_map<uuRecord_s,
                             returnRow_s> &andResults,
                             boost::unordered_map<uuRecord_s,
                             returnRow_s> &results)
{
    if (leftoperand[0] != OPERAND_FIELDID)
    {
        printf("%s %i left operand is not fieldid, it is %c\n", __FILE__,
               __LINE__, leftoperand[0]);
        return;
    }

    int64_t fieldid;
    memcpy(&fieldid, &leftoperand[1], sizeof(fieldid));

    class Table &tableRef = *schemaPtr->tables[tableid];
    vector<fieldValue_s> fieldValues;

    boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

    for (it = andResults.begin(); it != andResults.end(); it++)
    {
        const uuRecord_s &uurRef = it->first;
        const returnRow_s &returnRowRef = it->second;
        tableRef.unmakerow((string *)&returnRowRef.row, &fieldValues);
        fieldValue_s &lhs = fieldValues[fieldid];

        switch (tableRef.fields[fieldid].type)
        {
        case INT:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer==rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NE:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer!=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_GT:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer>rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LT:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer<rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_GTE:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer>=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LTE:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.integer<=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_BETWEEN:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                int64_t rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.integer >= rhsval && lhs.value.integer <= rhsval2)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NOTBETWEEN:
            {
                int64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                int64_t rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.integer < rhsval || lhs.value.integer > rhsval2)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_IN:
            {
                /* do after AST is walkable since inobject contains a
                 * vector of Ast */
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = returnRowRef;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                // do after AST is walkable since inobject contains a vector of Ast *
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = returnRowRef;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
            }
        }
        break;

        case UINT:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                uint64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.uinteger==rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NE:
            {
                uint64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.uinteger!=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_GT:
            {
                uint64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.uinteger>rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LT:
            {
                uint64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.uinteger<rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_GTE:
            {
                uint64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.uinteger>=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LTE:
            {
                uint64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.uinteger<=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_BETWEEN:
            {
                uint64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                uint64_t rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.uinteger >= rhsval && lhs.value.uinteger <=
                    rhsval2)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NOTBETWEEN:
            {
                uint64_t rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                uint64_t rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.uinteger < rhsval || lhs.value.uinteger > rhsval2)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_IN:
            {
                /* do after AST is walkable since inobject contains a vector
                 * of Ast */
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = returnRowRef;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                // do after AST is walkable since inobject contains a vector of Ast *
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = returnRowRef;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
            }
        }
        break;

        case BOOL:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean==rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NE:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean!=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_GT:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean>rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LT:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean<rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_GTE:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean>=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LTE:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.boolean<=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_BETWEEN:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                bool rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.boolean >= rhsval && lhs.value.boolean <= rhsval2)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NOTBETWEEN:
            {
                bool rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                bool rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.boolean < rhsval || lhs.value.boolean > rhsval2)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_IN:
            {
                // why do IN on a bool field?
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = returnRowRef;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                // why do NOT IN on a bool field?
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = returnRowRef;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
            }
        }
        break;

        case FLOAT:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating==rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NE:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating!=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_GT:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating>rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LT:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating<rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_GTE:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating>=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LTE:
            {
                Ast::toFloat(rightoperand, rightoperand);
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));

                if (lhs.value.floating<=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_BETWEEN:
            {
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                long double rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.floating >= rhsval && lhs.value.floating <=
                    rhsval2)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NOTBETWEEN:
            {
                long double rhsval;
                memcpy(&rhsval, &rightoperand[1], sizeof(rhsval));
                long double rhsval2;
                memcpy(&rhsval2, &rightoperand[1+sizeof(rhsval)],
                       sizeof(rhsval2));

                if (lhs.value.floating < rhsval || lhs.value.floating > rhsval2)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_IN:
            {
                /* do after AST is walkable since inobject contains a vector
                 * of Ast */
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = returnRowRef;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                /* do after AST is walkable since inobject contains a vector
                 * of Ast */
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = returnRowRef;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
            }
        }
        break;

        case CHAR:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character==rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NE:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character!=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_GT:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character>rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LT:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character<rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_GTE:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character>=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LTE:
            {
                char rhsval=rightoperand[1];

                if (lhs.value.character<=rhsval)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_BETWEEN:
            {
                char rhsval=rightoperand[1+sizeof(int64_t)];
                char rhsval2=rightoperand[1+sizeof(int64_t)+1];

                if (lhs.value.character >= rhsval && lhs.value.character <=
                    rhsval2)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NOTBETWEEN:
            {
                char rhsval=rightoperand[1+sizeof(int64_t)];
                char rhsval2=rightoperand[1+sizeof(int64_t)+1];

                if (lhs.value.character < rhsval || lhs.value.character >
                    rhsval2)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_IN:
            {
                /* do after AST is walkable since inobject contains a vector
                 * of Ast */
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = returnRowRef;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                /* do after AST is walkable since inobject contains a vector
                 * of Ast */
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = returnRowRef;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
            }
        }
        break;

        case CHARX:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                string rhsval=rightoperand.substr(1, string::npos);

                if (!lhs.str.compare(rhsval))
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NE:
            {
                string rhsval=rightoperand.substr(1, string::npos);

                if (lhs.str.compare(rhsval))
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_IN:
            {
                /* do after AST is walkable since inobject contains a vector
                 * of Ast */
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = returnRowRef;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                /* do after AST is walkable since inobject contains a vector
                 * of Ast */
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = returnRowRef;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LIKE:
            {
                string rhsval=rightoperand.substr(1, string::npos);
                like2Regex(rhsval);
                pcrecpp::RE re(rhsval);

                if (re.FullMatch(lhs.str)==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NOTLIKE:
            {
                string rhsval=rightoperand.substr(1, string::npos);
                like2Regex(rhsval);
                pcrecpp::RE re(rhsval);
                rhsval.insert(0, "^((?!");
                rhsval.append(").)*$");

                if (re.FullMatch(lhs.str)==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
            }
        }
        break;

        case VARCHAR:
        {
            switch (op)
            {
            case OPERATOR_EQ:
            {
                string rhsval=rightoperand.substr(1, string::npos);

                if (!lhs.str.compare(rhsval))
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NE:
            {
                string rhsval=rightoperand.substr(1, string::npos);

                if (lhs.str.compare(rhsval))
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_IN:
            {
                /* do after AST is walkable since inobject contains a vector
                 * of Ast */
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            results[uurRef] = returnRowRef;
                        }
                    }
                }
            }
            break;

            case OPERATOR_NOTIN:
            {
                /* do after AST is walkable since inobject contains a vector
                 * of Ast */
                int64_t fieldid;
                memcpy(&fieldid, &rightoperand[1], sizeof(fieldid));
                vector<fieldValue_s> fieldValues;
                class Table &tableRef = *schemaPtr->tables[tableid];
                fieldtype_e fieldtype = tableRef.fields[fieldid].type;
                boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

                for (it = andResults.begin(); it != andResults.end(); it++)
                {
                    const uuRecord_s &uurRef = it->first;
                    const returnRow_s &returnRowRef = it->second;
                    tableRef.unmakerow((string *)&returnRowRef.row,
                                       &fieldValues);
                    bool notin=true;

                    for (size_t n=0; inValues.size(); n++)
                    {
                        if (compareFields(fieldtype, fieldValues[fieldid],
                                          inValues[n])==true)
                        {
                            notin=false;
                            break;
                        }
                    }

                    if (notin==true)
                    {
                        results[uurRef] = returnRowRef;
                    }
                }
            }
            break;

            case OPERATOR_ISNULL:
            {
                if (lhs.isnull==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_ISNOTNULL:
            {
                if (lhs.isnull==false)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_LIKE:
            {
                string rhsval=rightoperand.substr(1, string::npos);
                like2Regex(rhsval);
                pcrecpp::RE re(rhsval);

                if (re.FullMatch(lhs.str)==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            case OPERATOR_NOTLIKE:
            {
                string rhsval=rightoperand.substr(1, string::npos);
                like2Regex(rhsval);
                pcrecpp::RE re(rhsval);
                rhsval.insert(0, "^((?!");
                rhsval.append(").)*$");

                if (re.FullMatch(lhs.str)==true)
                {
                    results[uurRef] = returnRowRef;
                }
            }
            break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__, op);
            }
        }
        break;

        default:
            fprintf(logfile, "anomaly: %i %s %i\n",
                    tableRef.fields[fieldid].type, __FILE__, __LINE__);
        }
    }
}

/* create new transaction if transactionPtrarg is NULL return transactionPtr
 * Pg will probably need to have another version of this function
 */
void Statement::execute(class ApiInterface *reentryObject, apifPtr reentryfptr,
                        int64_t reentrypoint, void *reentrydata,
                        class Transaction *transactionPtrarg,
                        const vector<string> &parametersarg)
{
    reentry.reentryObject=reentryObject;
    reentry.reentryfptr=reentryfptr;
    reentry.reentrypoint=reentrypoint;
    reentry.reentrydata=reentrydata;

    reentryObject->results.cmdtype=queries[0].type;

    switch (reentryObject->results.cmdtype)
    {
    case CMD_SELECT:
        if (transactionPtrarg==NULL)
        {
            transactionPtr = new class Transaction(taPtr, schemaPtr->domainid);
        }
        else
        {
            transactionPtr = transactionPtrarg;
        }

        break;

    case CMD_INSERT:
        if (transactionPtrarg==NULL)
        {
            transactionPtr = new class Transaction(taPtr, schemaPtr->domainid);
        }
        else
        {
            transactionPtr = transactionPtrarg;
        }

        break;

    case CMD_UPDATE:
        if (transactionPtrarg==NULL)
        {
            transactionPtr = new class Transaction(taPtr, schemaPtr->domainid);
        }
        else
        {
            transactionPtr = transactionPtrarg;
        }

        break;

    case CMD_DELETE:
        if (transactionPtrarg==NULL)
        {
            transactionPtr = new class Transaction(taPtr, schemaPtr->domainid);
        }
        else
        {
            transactionPtr = transactionPtrarg;
        }

        break;

    default:
        transactionPtr = transactionPtrarg;
    }

    parameters = parametersarg;
    queryindex = queries.size()-1;
    startQuery();
}

/* entrypoint=1, from a select, =2 for evaluating INSERT values. otherwise,
 * from IN & NOTIN or INSERT, =0
 */
void Statement::searchExpression(int64_t entrypoint, class Ast *astNode)
{
    class Ast *nextAstNode=NULL;

    while (1)
    {
        if (astNode->evaluate(&nextAstNode, this)==true)
        {
            // node evaluated successfully
            if (nextAstNode==NULL) // rootnode evaluated, carry on with statement
            {
                switch (entrypoint)
                    if (entrypoint==1)
                    {
                    case 1:
                        currentQuery->results.searchResults =
                            currentQuery->searchCondition->predicateResults;
                        delete currentQuery->searchCondition;
                        currentQuery->searchCondition=NULL;
                        branchtotype();
                        break;

                    case 2:
                        branchtotype();
                        break;

                    default:
                        ;
                    }

                return;
            }

            // non-rootnode evaluated, so set up for the next
            astNode=nextAstNode;
        }
        else
        {
            // node not evaluated successfully
            if (nextAstNode==NULL)
            {
                /* backgrounded query, will continue */
                return;
            }

            // evaluate a child
            astNode=nextAstNode;
        }
    }
}

void Statement::branchtotype()
{
    switch (currentQuery->type)
    {
    case CMD_SELECT:
    {
        if (currentQuery->haswhere==false)
        {
            /* select everything, checking existing staged rows first */
            boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

            for (it = transactionPtr->stagedRows.begin();
                 it != transactionPtr->stagedRows.end(); it++)
            {
                const uuRecord_s &uurRef = it->first;
                const stagedRow_s &stagedRowRef = it->second;
                returnRow_s returnRow;
                stagedRow2ReturnRow(stagedRowRef, returnRow);
                currentQuery->results.searchResults[uurRef]=returnRow;
            }

            transactionPtr->sqlSelectAll(this, currentQuery->tableid,
                                         currentQuery->locktype,
                                         PRIMITIVE_SQLSELECTALL,
                                         currentQuery->results.searchResults);
        }
        else
        {
            continueSelect(1, NULL);
        }
    }
    break;

    case CMD_INSERT:
    {
        class Table &tableRef = *schemaPtr->tables[currentQuery->tableid];

        size_t numfields = currentQuery->insertColumns.size();
        currentQuery->results.insertValues.reserve(numfields);

        for (size_t n=0; n < numfields; n++)
        {
            fieldValue_s fieldValue = {};
            class Ast &astRef = *currentQuery->insertColumns[numfields-1-n];
            searchExpression(0, &astRef);

            if (astRef.operand[0]==OPERAND_NULL)
            {
                class Field &fieldRef =
                    tableRef.fields[n];

                if (fieldRef.indextype==UNIQUENOTNULL ||
                    fieldRef.indextype==NONUNIQUENOTNULL ||
                    fieldRef.indextype==UNORDEREDNOTNULL)
                {
                    reenter(APISTATUS_NULLCONSTRAINT);
                    return;
                }

                fieldValue.isnull=true;
                currentQuery->results.insertValues.push_back(fieldValue);
            }
            else
            {
                switch (tableRef.fields[n].type)
                {
                case INT:
                    memcpy(&fieldValue.value.integer, &astRef.operand[1],
                           sizeof(int64_t));
                    currentQuery->results.insertValues.push_back(fieldValue);
                    break;

                case UINT:
                    memcpy(&fieldValue.value.uinteger, &astRef.operand[1],
                           sizeof(int64_t));
                    currentQuery->results.insertValues.push_back(fieldValue);
                    break;

                case BOOL:
                    if (astRef.operand[1]=='t')
                    {
                        fieldValue.value.boolean=true;
                    }
                    else
                    {
                        fieldValue.value.boolean=false;
                    }

                    currentQuery->results.insertValues.push_back(fieldValue);
                    break;

                case FLOAT:
                    Ast::toFloat(astRef.operand, fieldValue);
                    currentQuery->results.insertValues.push_back(fieldValue);
                    break;

                case CHAR:
                    fieldValue.value.character=astRef.operand[1];
                    currentQuery->results.insertValues.push_back(fieldValue);
                    break;

                case CHARX:
                    fieldValue.str=astRef.operand.substr(1, string::npos);
                    currentQuery->results.insertValues.push_back(fieldValue);
                    break;

                case VARCHAR:
                    fieldValue.str=astRef.operand.substr(1, string::npos);
                    currentQuery->results.insertValues.push_back(fieldValue);
                    break;

                default:
                    printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                           tableRef.fields[n].type);
                }
            }
        }

        if (tableRef.makerow(&currentQuery->results.insertValues,
                             &currentQuery->results.newrow)==false)
        {
            reenter(APISTATUS_NOTOK);
        }

        currentQuery->results.newrowengineid = transactionPtr->getengine(
            tableRef.fields[0].type, currentQuery->results.insertValues[0]);

        if (transactionPtr->pendingcmd != NOCOMMAND)
        {
            reenter(APISTATUS_PENDING);
            return;
        }

        transactionPtr->pendingcmdid = transactionPtr->getnextpendingcmdid();
        transactionPtr->pendingcmd = PRIMITIVE_SQLINSERT;
        transactionPtr->sqlcmdstate = (Transaction::sqlcmdstate_s)
            {
                0
            };
        transactionPtr->sqlcmdstate.statement=this;
        transactionPtr->sqlcmdstate.tableid = currentQuery->tableid;

        class MessageSubtransactionCmd *msg =
            new class MessageSubtransactionCmd();
        class MessageSubtransactionCmd &msgref = *msg;
        msgref.subtransactionStruct.tableid = currentQuery->tableid;
        msgref.row = currentQuery->results.newrow;
        transactionPtr->sendTransaction(NEWROW, PAYLOADSUBTRANSACTION, 1,
                                        currentQuery->results.newrowengineid,
                                        msg);
    }
    break;

    case CMD_UPDATE:
    {
        if (currentQuery->haswhere==false)
        {
            boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

            for (it = transactionPtr->stagedRows.begin();
                 it != transactionPtr->stagedRows.end(); it++)
            {
                const uuRecord_s &uurRef = it->first;
                const stagedRow_s &stagedRowRef = it->second;
                returnRow_s returnRow;
                stagedRow2ReturnRow(stagedRowRef, returnRow);
                currentQuery->results.searchResults[uurRef]=returnRow;
            }

            transactionPtr->sqlSelectAll(this, currentQuery->tableid, WRITELOCK,
                                         PRIMITIVE_SQLSELECTALLFORUPDATE,
                                         currentQuery->results.searchResults);
            return;
        }
        else
        {
            continueUpdate(1, NULL);
        }
    }
    break;

    case CMD_DELETE:
    {
        if (currentQuery->haswhere==false)
        {
            boost::unordered_map<uuRecord_s, stagedRow_s>::const_iterator it;

            for (it = transactionPtr->stagedRows.begin();
                 it != transactionPtr->stagedRows.end(); it++)
            {
                const uuRecord_s &uurRef = it->first;
                const stagedRow_s &stagedRowRef = it->second;
                returnRow_s returnRow;
                stagedRow2ReturnRow(stagedRowRef, returnRow);
                currentQuery->results.searchResults[uurRef]=returnRow;
            }

            transactionPtr->sqlSelectAll(this, currentQuery->tableid, WRITELOCK,
                                         PRIMITIVE_SQLSELECTALLFORDELETE,
                                         currentQuery->results.searchResults);
        }
        else
        {
            continueDelete(1, NULL);
        }
    }
    break;

    case CMD_STOREDPROCEDURE:
    {
        reentry.reentryObject->results.cmdtype = CMD_STOREDPROCEDURE;

        if (taPtr->domainidsToProcedures.count(schemaPtr->domainid))
        {
            domainProceduresMap &procsMapRef =
                taPtr->domainidsToProcedures[schemaPtr->domainid];

            if (procsMapRef.count(currentQuery->storedProcedure))
            {
                procedures_s &proceduresRef =
                    procsMapRef[currentQuery->storedProcedure];
                spclasscreate spC =
                    (spclasscreate)proceduresRef.procedurecreator;
                spclasscreate spD =
                    (spclasscreate)proceduresRef.proceduredestroyer;
                // make sure to delete this statement in the procedure!
                spC(NULL, reentry.reentryObject, (void *)spD);
            }
            else
            {
                reenter(STATUS_NOTOK);
                return;
            }
        }
        else
        {
            reenter(STATUS_NOTOK);
            return;
        }
    }
    break;

    case CMD_BEGIN:
        startQuery();
        break;

    case CMD_COMMIT:
        startQuery();
        break;

    case CMD_ROLLBACK:
        startQuery();
        break;

    default:
        printf("%s %i unhandled statement type %i\n", __FILE__, __LINE__,
               currentQuery->type);
        reenter(STATUS_NOTOK);
        return;
    }
}

void Statement::reenter(int64_t status)
{
    class ApiInterface *reentryObject=reentry.reentryObject;
    apifPtr reentryfptr=reentry.reentryfptr;
    int64_t reentrypoint=reentry.reentrypoint;
    void *reentrydata=reentry.reentrydata;

    if (transactionPtr != NULL)
    {
        transactionPtr->pendingcmd=NOCOMMAND;
        transactionPtr->pendingcmdid=0;
    }

    delete this;
    reentryObject->statementPtr=NULL;
    reentryObject->results.statementStatus=status;
    reentryObject->results.transactionPtr=transactionPtr;
    (*reentryObject.*reentryfptr)(reentrypoint, reentrydata);
}

void Statement::continueSelect(int64_t entrypoint, class Ast *ignorethis)
{
    /* there should be nothing special to do for selectall vs predicate search
     * because the searchResults have already been populated
     */
    class Table &tableRef = *schemaPtr->tables[currentQuery->tableid];

    boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

    for (it = currentQuery->results.searchResults.begin();
         it != currentQuery->results.searchResults.end(); it++)
    {
        const uuRecord_s &uurRef = it->first;
        const returnRow_s &returnRowRef = it->second;
        vector<fieldValue_s> foundFields;
        tableRef.unmakerow((string *)&returnRowRef.row, &foundFields);
        vector<fieldValue_s> returnFields;

        for (size_t n=0; n < currentQuery->fromColumnids.size(); n++)
        {
            returnFields.push_back(foundFields[currentQuery->fromColumnids[n].fieldid]);
        }

        currentQuery->results.selectResults[uurRef] = returnFields;

        if (!transactionPtr->stagedRows.count(uurRef))
        {
            stagedRow_s srow = {};
            srow.cmd=NOCOMMAND;
            srow.locktype=returnRowRef.locktype;
            srow.originalRow=returnRowRef.row;
            srow.originalrowid=returnRowRef.rowid;

            transactionPtr->stagedRows[uurRef]=srow;
        }
    }

    startQuery();
}

void Statement::continueDelete(int64_t entrypoint, class Ast *ignorethis)
{
    switch (entrypoint)
    {
    case 1:
    {
        transactionPtr->sqlcmdstate = (Transaction::sqlcmdstate_s)
            {
                0
            };
        transactionPtr->sqlcmdstate.statement=this;
        transactionPtr->pendingcmdid = transactionPtr->getnextpendingcmdid();
        transactionPtr->pendingcmd = PRIMITIVE_SQLDELETE;

        boost::unordered_map<uuRecord_s, returnRow_s>::const_iterator it;

        for (it = currentQuery->results.searchResults.begin();
             it != currentQuery->results.searchResults.end(); it++)
        {
            const uuRecord_s &uurRef = it->first;
            const returnRow_s &returnRowRef = it->second;
            class MessageSubtransactionCmd *msg =
                new class MessageSubtransactionCmd();
            msg->subtransactionStruct.tableid = uurRef.tableid;
            msg->subtransactionStruct.rowid = uurRef.rowid;
            msg->subtransactionStruct.engineid = uurRef.engineid;
            transactionPtr->sendTransaction(DELETEROW, PAYLOADSUBTRANSACTION, 1,
                                            uurRef.engineid, msg);
            stagedRow_s &stagedRowRef = transactionPtr->stagedRows[uurRef];
            stagedRowRef.cmd=DELETE;
            stagedRowRef.originalRow=returnRowRef.row;
            transactionPtr->stagedRows[uurRef].cmd=DELETE;
        }

        transactionPtr->sqlcmdstate.eventwaitcount =
            currentQuery->results.searchResults.size();
    }
    break;

    case 2:
        startQuery();
        break;

    default:
        printf("%s %i anomaly %li\n", __FILE__, __LINE__, entrypoint);
    }
}

void Statement::continueUpdate(int64_t entrypoint, class Ast *ignorethis)
{
    switch (entrypoint)
    {
    case 1:
    {
        /* walk through each search result, doing update on each one
         * and calling Ast::evaluateAssignment(fieldValues) for each
         * field to modify
         */
        currentQuery->results.updateIterator=
            currentQuery->results.searchResults.begin();
    }

    //    break; fall right through

    case 2:
    {
        /* walk through each search result, doing update on each one
         * and calling Ast::evaluateAssignment(fieldValues) for each
         * field to modify
         */
        if (transactionPtr->pendingcmd != NOCOMMAND)
        {
            printf("%s %i APISTATUS_PENDING pendingcmd %i\n", __FILE__, __LINE__, transactionPtr->pendingcmd);
            abortQuery(APISTATUS_PENDING);
            return;
        }

        transactionPtr->pendingcmdid = transactionPtr->getnextpendingcmdid();
        transactionPtr->sqlcmdstate = (Transaction::sqlcmdstate_s)
            {
                0
            };
        transactionPtr->sqlcmdstate.statement=this;

        class Table &tableRef = *schemaPtr->tables[currentQuery->tableid];

        if (currentQuery->results.updateIterator !=
            currentQuery->results.searchResults.end())
        {
            vector<fieldValue_s> fieldValues;
            const returnRow_s &returnRowRef=
                currentQuery->results.updateIterator->second;
            tableRef.unmakerow((string *)&returnRowRef.row, &fieldValues);

            // do update stuff
            // boost::unordered_map<int64_t, class Ast *> fieldidAssignments
            boost::unordered_map<int64_t, class Ast *>::iterator it;

            for (it = currentQuery->fieldidAssignments.begin();
                 it != currentQuery->fieldidAssignments.end(); it++)
            {
                it->second->evaluateAssignment(fieldValues, this);
                // fieldid is it->first; class Ast * is it->second;
                class Field &fieldRef = tableRef.fields[it->first];
                fieldValue_s fieldValue = {};

                if (it->second->operand[0]==OPERAND_NULL)
                {
                    fieldValue.isnull=true;
                }
                else
                {
                    switch (fieldRef.type)
                    {
                    case INT:
                        memcpy(&fieldValue.value.integer,
                               &it->second->operand[1],
                               sizeof(int64_t));
                        break;

                    case UINT:
                        memcpy(&fieldValue.value.uinteger,
                               &it->second->operand[1],
                               sizeof(int64_t));
                        break;

                    case BOOL:
                    {
                        int64_t boolval;
                        memcpy(&boolval, &it->second->operand[1],
                               sizeof(int64_t));
                        fieldValue.value.boolean = (bool)boolval;
                    }
                    break;

                    case FLOAT:
                        Ast::toFloat(it->second->operand, fieldValue);
                        break;

                    case CHAR:
                        fieldValue.value.character=it->second->operand[1];
                        break;

                    case CHARX:
                        fieldValue.str=it->second->operand.substr(1,
                                                                  string::npos);
                        break;

                    case VARCHAR:
                        fieldValue.str=it->second->operand.substr(1,
                                                                  string::npos);
                        break;

                    default:
                        printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                               tableRef.fields[it->first].type);
                        return;
                    }
                }

                currentQuery->results.setFields[it->first]=fieldValue;
            }

            const uuRecord_s &uurRef=
                currentQuery->results.updateIterator->first;

            if (!currentQuery->results.setFields.count(0))
            {
                // update
                transactionPtr->pendingcmd = PRIMITIVE_SQLUPDATE;
                stagedRow_s stagedRow= {};
                vector<fieldValue_s> fieldValues;
                tableRef.unmakerow((string *)&returnRowRef.row, &fieldValues);

                for (size_t n=0; n < fieldValues.size(); n++)
                {
                    class Field &fieldRef = tableRef.fields[n];

                    if (currentQuery->results.setFields.count(n))
                    {
                        fieldValues[n] = currentQuery->results.setFields[n];

                        // index stuff
                        if (fieldRef.index.isunique==true)
                        {
                            lockFieldValue_s lockFieldValue = {};
                            lockFieldValue.engineid =
                                transactionPtr->getengine(fieldRef.type,
                                                          fieldValues[n]);
                            // locktype could potentially change
                            lockFieldValue.locktype = INDEXLOCK;
                            lockFieldValue.fieldVal = fieldValues[n];
                            stagedRow.uniqueIndices[n]=lockFieldValue;

                            transactionPtr->sqlcmdstate.eventwaitcount++;
                            class MessageSubtransactionCmd *msg =
                                new class MessageSubtransactionCmd();
                            msg->subtransactionStruct.isrow = false;
                            msg->subtransactionStruct.tableid = uurRef.tableid;
                            msg->subtransactionStruct.rowid = uurRef.rowid;
                            msg->subtransactionStruct.engineid = uurRef.engineid;
                            msg->subtransactionStruct.fieldid = n;
                            msg->fieldVal = lockFieldValue.fieldVal;
                            transactionPtr->sendTransaction(UNIQUEINDEX,
                                                            PAYLOADSUBTRANSACTION,
                                                            1,
                                                            lockFieldValue.engineid,
                                                            msg);
                        }
                    }
                }

                stagedRow.originalRow=returnRowRef.row;
                stagedRow.originalrowid=uurRef.rowid;
                stagedRow.newrowid=uurRef.rowid;
                stagedRow.previoussubtransactionid=
                    returnRowRef.previoussubtransactionid;
                stagedRow.locktype=WRITELOCK;
                stagedRow.originalengineid=uurRef.engineid;
                stagedRow.newengineid=uurRef.engineid;
                tableRef.makerow(&fieldValues, &stagedRow.newRow);
                stagedRow.cmd=UPDATE;
                transactionPtr->stagedRows[uurRef]=stagedRow;

                transactionPtr->sqlcmdstate.eventwaitcount++;
                class MessageSubtransactionCmd *msg =
                    new class MessageSubtransactionCmd();
                msg->subtransactionStruct.tableid = uurRef.tableid;
                msg->subtransactionStruct.rowid = uurRef.rowid;
                msg->row = stagedRow.newRow;
                transactionPtr->sendTransaction(UPDATEROW, PAYLOADSUBTRANSACTION,
                                                1, uurRef.engineid, msg);
            }
            else
            {
                //replace
                stagedRow_s &stagedRowRef = transactionPtr->stagedRows[uurRef];
                stagedRowRef.cmd=UPDATE;
                stagedRowRef.originalRow=returnRowRef.row;
                stagedRowRef.originalrowid=uurRef.rowid;
                stagedRowRef.originalengineid=uurRef.engineid;
                stagedRowRef.previoussubtransactionid=
                    returnRowRef.previoussubtransactionid;
                stagedRowRef.locktype=WRITELOCK;

                transactionPtr->pendingcmd = PRIMITIVE_SQLREPLACE;
                currentQuery->results.originalrowuur = uurRef;
                vector<fieldValue_s> fieldValues;
                tableRef.unmakerow((string *)&returnRowRef.row, &fieldValues);

                for (size_t n=0; n < fieldValues.size(); n++)
                {
                    if (currentQuery->results.setFields.count(n))
                    {
                        fieldValues[n] = currentQuery->results.setFields[n];
                    }
                }

                tableRef.makerow(&fieldValues, &stagedRowRef.newRow);
                currentQuery->results.newrowengineid =
                    transactionPtr->getengine(tableRef.fields[0].type,
                                              fieldValues[0]);
                stagedRowRef.newengineid=currentQuery->results.newrowengineid;

                class MessageSubtransactionCmd *msg =
                    new class MessageSubtransactionCmd();
                class MessageSubtransactionCmd &msgref = *msg;
                msgref.subtransactionStruct.tableid = currentQuery->tableid;
                msgref.row = stagedRowRef.newRow;
                transactionPtr->sendTransaction(NEWROW, PAYLOADSUBTRANSACTION, 1,
                                                stagedRowRef.newengineid, msg);
            }

            currentQuery->results.updateIterator++;

            return;
        }

        startQuery();
    }
    break;

    default:
        printf("%s %i anomaly %li\n", __FILE__, __LINE__, entrypoint);
    }
}

void Statement::continueInsert(int64_t entrypoint, class Ast *ignorethis)
{
    startQuery();
}

void Statement::startQuery()
{
    if (queryindex >= 0)
    {
        currentQuery = &queries[queryindex--];

        //    if (currentQuery->searchCondition != NULL)
        if (currentQuery->haswhere==true)
        {
            searchExpression(1, currentQuery->searchCondition);
        }
        else
        {
            branchtotype();
        }
    }
    else
    {
        // reenter
        /* put the final query results somewhere, like staged rows,
         * locking every row in every query, then put the results
         * somewhere the user can find them */
        switch (currentQuery->type)
        {
        case CMD_SELECT:
        {
            reentry.reentryObject->results.cmdtype = CMD_SELECT;
            reentry.reentryObject->results.statementStatus = STATUS_OK;
            reentry.reentryObject->results.selectResults =
                currentQuery->results.selectResults;
            class Table &tableRef = *schemaPtr->tables[currentQuery->tableid];

            for (size_t n=0; n < currentQuery->fromColumnids.size(); n++)
            {
                reentry.reentryObject->results.selectFields.push_back(
                    {tableRef.fields[currentQuery->fromColumnids[n].fieldid].type, currentQuery->fromColumnids[n].name});
            }
        }
        break;

        case CMD_INSERT:
        {
            reentry.reentryObject->results.cmdtype = CMD_INSERT;
            reentry.reentryObject->results.statementStatus = STATUS_OK;
            returnRow_s returnRow;
            returnRow.rowid = currentQuery->results.newrowuur.rowid;
            returnRow.previoussubtransactionid = 0;
            returnRow.locktype = WRITELOCK;
            returnRow.row = currentQuery->results.newrow;
            reentry.reentryObject->results.statementResults[currentQuery->results.newrowuur] =
                returnRow;
        }
        break;

        case CMD_UPDATE:
            reentry.reentryObject->results.cmdtype = CMD_UPDATE;
            reentry.reentryObject->results.statementStatus = STATUS_OK;
            reentry.reentryObject->results.statementResults =
                currentQuery->results.searchResults;
            break;

        case CMD_DELETE:
            reentry.reentryObject->results.cmdtype = CMD_DELETE;
            reentry.reentryObject->results.statementStatus = STATUS_OK;
            reentry.reentryObject->results.statementResults =
                currentQuery->results.searchResults;
            break;

        case CMD_BEGIN:
            reentry.reentryObject->results.cmdtype = CMD_BEGIN;
            reentry.reentryObject->results.statementStatus = STATUS_OK;
            break;

        case CMD_COMMIT:
            reentry.reentryObject->results.cmdtype = CMD_COMMIT;
            reentry.reentryObject->results.statementStatus = STATUS_OK;
            break;

        case CMD_ROLLBACK:
            reentry.reentryObject->results.cmdtype = CMD_ROLLBACK;
            reentry.reentryObject->results.statementStatus = STATUS_OK;
            break;

        default:
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, currentQuery->type);
            return;
        }

        reenter(STATUS_OK);
    }
}

void Statement::subqueryScalar(class Ast *astnode)
{
    if (astnode->operand[0] != OPERAND_SUBQUERY)
    {
        return;
    }

    int64_t queryinstance;
    memcpy(&queryinstance, &astnode->operand[1], sizeof(queryinstance));

    query_s &queryRef = queries[queryinstance];
    boost::unordered_map< uuRecord_s, vector<fieldValue_s> > &selectResultsRef =
        queryRef.results.selectResults;

    switch (selectResultsRef.size())
    {
    case 0:
        astnode->operand.assign(1, OPERAND_NULL);
        break;

    case 1:
    {
        boost::unordered_map< uuRecord_s,
                              vector<fieldValue_s> >::const_iterator it;
        it = selectResultsRef.begin();
        const vector<fieldValue_s> &fieldValuesRef = it->second;

        if (fieldValuesRef.size()==1)
        {
            // get the field type table.fields[0].type fieldtype_e
            switch (schemaPtr->tables[queryRef.tableid]->fields[0].type)
            {
            case INT:
                astnode->operand.resize(1+sizeof(int64_t), (char)0);
                astnode->operand[0] = OPERAND_INTEGER;
                memcpy(&astnode->operand[1], &fieldValuesRef[0].value.integer,
                       sizeof(int64_t));
                break;

            case UINT:
                astnode->operand.resize(1+sizeof(int64_t), (char)0);
                astnode->operand[0] = OPERAND_INTEGER;
                memcpy(&astnode->operand[1], &fieldValuesRef[0].value.uinteger,
                       sizeof(int64_t));
                break;

            case BOOL:
                astnode->operand.resize(2, OPERAND_BOOLEAN);

                if (fieldValuesRef[0].value.boolean==true)
                {
                    astnode->operand[1]='t';
                }
                else
                {
                    astnode->operand[1]='f';
                }

                break;

            case FLOAT:
                astnode->operand.resize(1+sizeof(long double), (char)0);
                astnode->operand[0] = OPERAND_FLOAT;
                memcpy(&astnode->operand[1], &fieldValuesRef[0].value.floating,
                       sizeof(int64_t));
                break;

            case CHAR:
                astnode->operand.resize(1+sizeof(char), (char)0);
                astnode->operand[0] = OPERAND_STRING;
                astnode->operand[1] = fieldValuesRef[0].value.character;
                break;

            case CHARX:
                astnode->operand.assign(1, OPERAND_STRING);
                astnode->operand.append(fieldValuesRef[0].str);
                break;

            case VARCHAR:
                astnode->operand.assign(1, OPERAND_STRING);
                astnode->operand.append(fieldValuesRef[0].str);
                break;

            default:
                printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                       schemaPtr->tables[queryRef.tableid]->fields[0].type);
            }

            return;
        }
        else
        {
            printf("%s %i too many columns in scalar subquery %lu\n", __FILE__,
                   __LINE__, fieldValuesRef.size());
        }
    }
    break;

    default:
        printf("%s %i too many returned rows in scalar subquery %lu\n", __FILE__,
               __LINE__, selectResultsRef.size());
    }
}

void Statement::subqueryUnique(class Ast *astnode)
{
    int64_t queryinstance;
    memcpy(&queryinstance, &astnode->operand[1], sizeof(queryinstance));
    query_s &queryRef = queries[queryinstance];
    size_t numfields = queryRef.fromColumnids.size();
    class Table &tableRef = *schemaPtr->tables[queryRef.tableid];

    boost::unordered_map< uuRecord_s, vector<fieldValue_s> >::iterator it;
    boost::unordered_map< uuRecord_s, vector<fieldValue_s> >::iterator it2;

    for (it = queryRef.results.selectResults.begin();
         it != queryRef.results.selectResults.end(); it++)
    {
        const uuRecord_s &uurRef1 = it->first;
        const vector<fieldValue_s> &fieldValues = it->second;

        for (it2 = queryRef.results.selectResults.begin();
             it2 != queryRef.results.selectResults.end(); it++)
        {
            const uuRecord_s &uurRef2 = it2->first;
            const vector<fieldValue_s> &fieldValues2 = it->second;

            if (uurRef1==uurRef2)
            {
                continue;
            }

            bool ismatch=true;

            for (size_t n=0; n < numfields; n++)
            {
                if (compareFields(
                        tableRef.fields[queryRef.fromColumnids[n].fieldid].type,
                        fieldValues[n], fieldValues2[n])==false)
                {
                    ismatch=false;
                    break;
                }
            }

            if (ismatch==true)
            {
                astnode->operand.resize(OPERAND_BOOLEAN, 2);
                astnode->operand[1]='f';
                return;
            }
        }
    }

    astnode->operand.resize(OPERAND_BOOLEAN, 2);
    astnode->operand[1] = 't';
}

void Statement::subqueryExists(class Ast *astnode)
{
    int64_t queryinstance;
    memcpy(&queryinstance, &astnode->operand[1], sizeof(queryinstance));

    astnode->operand.resize(OPERAND_BOOLEAN, 2);

    if (queries[queryinstance].results.selectResults.size())
    {
        astnode->operand[1]='t';
    }
    else
    {
        astnode->operand[1]='f';
    }
}

void Statement::subqueryIn(class Ast *astnode)
{
    int64_t queryinstance;
    memcpy(&queryinstance, &astnode->operand[1], sizeof(queryinstance));
    query_s &queryRef = queries[queryinstance];

    boost::unordered_map< uuRecord_s, vector<fieldValue_s> >::iterator it;

    for (it = queryRef.results.selectResults.begin();
         it != queryRef.results.selectResults.end(); it++)
    {
        queryRef.results.inValues.push_back(it->second[0]);
    }
}

void Statement::abortQuery(int64_t status)
{
    // entire transaction should be rolled back if non-zero status
    reenter(status);
}
