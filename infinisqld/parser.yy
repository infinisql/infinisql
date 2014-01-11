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

%define api.pure
%parse-param {struct perlarxer *pld}

%{
#include "larx.h"
#include "gch.h"
#include "Larxer.h"
#include "Schema.h"

#define YYLEX_PARAM pld->scaninfo
#define PUSHSTACK(X) pld->larxerPtr->pushstack(X)
#define PUSHSTACK2(X, Y) pld->larxerPtr->pushstack(X, Y)
#define PUSHOPERATOR(X) pld->larxerPtr->pushstack(Larxer::TYPE_operator, (int64_t)X)
#define PUSHOPERANDSUBQUERY pld->larxerPtr->pushoperand(OPERAND_SUBQUERY)
#define PUSHOPERANDNULL pld->larxerPtr->pushoperand(OPERAND_NULL)
#define PUSHOPERAND(X, Y) pld->larxerPtr->pushoperand(X, Y)
#define PUSHAGGREGATE(X, Y) pld->larxerPtr->pushaggregate(X, Y)

void yyerror(void *, char *);
typedef void* yyscan_t;
int yylex(YYSTYPE *, yyscan_t);
%}

%union
{
  long int integer;
  long double floating;
  char *str;
}

%token LARX_ABSOLUTE
%token LARX_ACTION
%token LARX_ADD
%token LARX_ALL
%token LARX_ALLOCATE
%token LARX_ALTER
%token LARX_ANY
%token LARX_ARE
%token LARX_AS
%token LARX_ASC
%token LARX_ASSERTION
%token LARX_AT
%token LARX_AUTHORIZATION
%token LARX_AVG
%token LARX_BEGIN
%token LARX_BETWEEN
%token LARX_BIGINT
%token LARX_BIT
%token LARX_BIT_LENGTH
%token LARX_BOOLEAN
%token LARX_BOTH
%token LARX_BY
%token LARX_CASCADE
%token LARX_CASCADED
%token LARX_CASE
%token LARX_CAST
%token LARX_CATALOG
%token LARX_CHAR
%token LARX_CHARACTER
%token LARX_CHARACTER_LENGTH
%token LARX_CHAR_LENGTH
%token LARX_CHECK
%token LARX_CLOSE
%token LARX_COALESCE
%token LARX_COLLATE
%token LARX_COLLATION
%token LARX_COLUMN
%token LARX_COMMIT
%token LARX_CONNECT
%token LARX_CONNECTION
%token LARX_CONSTRAINT
%token LARX_CONSTRAINTS
%token LARX_CONTINUE
%token LARX_CONVERT
%token LARX_CORRESPONDING
%token LARX_COUNT
%token LARX_CREATE
%token LARX_CROSS
%token LARX_CURRENT
%token LARX_CURRENT_DATE
%token LARX_CURRENT_TIME
%token LARX_CURRENT_TIMESTAMP
%token LARX_CURRENT_USER
%token LARX_CURSOR
%token LARX_DATE
%token LARX_DATETIME
%token LARX_DAY
%token LARX_DEALLOCATE
%token LARX_DEC
%token LARX_DECIMAL
%token LARX_DECLARE
%token LARX_DEFAULT
%token LARX_DEFERRABLE
%token LARX_DEFERRED
%token LARX_DELETE
%token LARX_DESC
%token LARX_DESCRIBE
%token LARX_DESCRIPTOR
%token LARX_DIAGNOSTICS
%token LARX_DISCONNECT
%token LARX_DISTINCT
%token LARX_DOMAIN
%token LARX_DOUBLE
%token LARX_DROP
%token LARX_ELSE
%token LARX_END
%token LARX_END_EXEC
%token LARX_ESCAPE
%token LARX_EXCEPT
%token LARX_EXCEPTION
%token LARX_EXEC
%token LARX_EXECUTE
%token LARX_EXISTS
%token LARX_EXTERNAL
%token LARX_EXTRACT
%token LARX_FALSE
%token LARX_FETCH
%token LARX_FIRST
%token LARX_FLOAT
%token LARX_FOR
%token LARX_FOREIGN
%token LARX_FOUND
%token LARX_FROM
%token LARX_FULL
%token LARX_GET
%token LARX_GLOBAL
%token LARX_GO
%token LARX_GOTO
%token LARX_GRANT
%token LARX_GROUP
%token LARX_HAVING
%token LARX_HOUR
%token LARX_IDENTITY
%token LARX_IF
%token LARX_IMMEDIATE
%token LARX_IN
%token LARX_INDICATOR
%token LARX_INITIALLY
%token LARX_INNER
%token LARX_INPUT
%token LARX_INSENSITIVE
%token LARX_INSERT
%token LARX_INT
%token LARX_INTEGER
%token LARX_INTERSECT
%token LARX_INTERVAL
%token LARX_INTO
%token LARX_IS
%token LARX_ISOLATION
%token LARX_JOIN
%token LARX_KEY
%token LARX_LANGUAGE
%token LARX_LAST
%token LARX_LEADING
%token LARX_LEFT
%token LARX_LEVEL
%token LARX_LIKE
%token LARX_LOCAL
%token LARX_LOWER
%token LARX_MATCH
%token LARX_MAX
%token LARX_MIN
%token LARX_MINUTE
%token LARX_MODULE
%token LARX_MONTH
%token LARX_NAMES
%token LARX_NATIONAL
%token LARX_NATURAL
%token LARX_NCHAR
%token LARX_NEXT
%token LARX_NO
%token LARX_NULL
%token LARX_NULLIF
%token LARX_NUMERIC
%token LARX_OCTET_LENGTH
%token LARX_OF
%token LARX_ON
%token LARX_ONLY
%token LARX_OPEN
%token LARX_OPTION
%token LARX_ORDER
%token LARX_OUTER
%token LARX_OUTPUT
%token LARX_OVERLAPS
%token LARX_PAD
%token LARX_PARTIAL
%token LARX_POSITION
%token LARX_PRECISION
%token LARX_PREPARE
%token LARX_PRESERVE
%token LARX_PRIMARY
%token LARX_PRIOR
%token LARX_PRIVILEGES
%token LARX_PROCEDURE
%token LARX_PUBLIC
%token LARX_READ
%token LARX_REAL
%token LARX_REFERENCES
%token LARX_RELATIVE
%token LARX_RESTRICT
%token LARX_REVOKE
%token LARX_RIGHT
%token LARX_ROLLBACK
%token LARX_ROWS
%token LARX_SCHEMA
%token LARX_SCROLL
%token LARX_SECOND
%token LARX_SECTION
%token LARX_SELECT
%token LARX_SESSION
%token LARX_SESSION_USER
%token LARX_SET
%token LARX_SIZE
%token LARX_SMALLINT
%token LARX_SOME
%token LARX_SPACE
%token LARX_SQL
%token LARX_SQLCODE
%token LARX_SQLERROR
%token LARX_SQLSTATE
%token LARX_SUBSTRING
%token LARX_SUM
%token LARX_SYSTEM_USER
%token LARX_TABLE
%token LARX_TEMPORARY
%token LARX_TEXT
%token LARX_THEN
%token LARX_TIME
%token LARX_TIMESTAMP
%token LARX_TIMEZONE_HOUR
%token LARX_TIMEZONE_MINUTE
%token LARX_TO
%token LARX_TRAILING
%token LARX_TRANSACTION
%token LARX_TRANSLATE
%token LARX_TRANSLATION
%token LARX_TRIM
%token LARX_TRUE
%token LARX_UNION
%token LARX_UNIQUE
%token LARX_UNKNOWN
%token LARX_UPDATE
%token LARX_UPPER
%token LARX_USAGE
%token LARX_USER
%token LARX_USING
%token LARX_VALUE
%token LARX_VALUES
%token LARX_VARCHAR
%token LARX_VARYING
%token LARX_VIEW
%token LARX_WHEN
%token LARX_WHENEVER
%token LARX_WHERE
%token LARX_WITH
%token LARX_WORK
%token LARX_WRITE
%token LARX_YEAR
%token LARX_ZONE

%token LARX_LOCK

%token LARX_ne
%token LARX_gte
%token LARX_lte
%token LARX_concat

%token <integer> LARX_intval
%token <floating> LARX_floatval
%token <str> LARX_stringval
%token <str> LARX_identifier
%token <integer> LARX_parameter

%left '+' '-' LARX_concat LARX_AND LARX_OR
%left '*' '/'
%right '^'
%nonassoc LARX_uminus LARX_uplus LARX_NOT LARX_IS

%type <integer> columns aggregate_expression_list ddl_column_name data_type_name

%start stmt

%%

/* the stmt is implicit because the stack is finished */
stmt: select_stmt optional_sql_terminator
    | insert_stmt optional_sql_terminator
    | update_stmt optional_sql_terminator
    | delete_stmt optional_sql_terminator
    | create_stmt optional_sql_terminator
    | drop_stmt optional_sql_terminator
    | alter_stmt optional_sql_terminator
    | LARX_COMMIT optional_work optional_sql_terminator
      { PUSHSTACK(Larxer::TYPE_COMMIT); }
    | LARX_BEGIN optional_work optional_sql_terminator
      { PUSHSTACK(Larxer::TYPE_BEGIN); }
    | LARX_ROLLBACK optional_work optional_sql_terminator
      { PUSHSTACK(Larxer::TYPE_ROLLBACK); }
    ;

select_stmt: LARX_SELECT columns from_clause
      where_clause group_by_clause having_clause for_update_clause
      no_lock_clause order_by_clause { PUSHSTACK2(Larxer::TYPE_SELECT, $2); }
    | LARX_SELECT set_quantifier columns from_clause
      where_clause group_by_clause having_clause for_update_clause
      no_lock_clause order_by_clause { PUSHSTACK2(Larxer::TYPE_SELECT, $3); }
    | LARX_SELECT identifier '(' operandlist ')'
      { PUSHSTACK(Larxer::TYPE_storedprocedure); } ;

insert_stmt: LARX_INSERT LARX_INTO identifier LARX_VALUES
    '(' expressionlist ')' { PUSHSTACK(Larxer::TYPE_INSERT); } ;

update_stmt: LARX_UPDATE identifier LARX_SET setassignmentlist where_clause
    { PUSHSTACK(Larxer::TYPE_UPDATE); } ;

delete_stmt: LARX_DELETE LARX_FROM identifier where_clause
    { PUSHSTACK(Larxer::TYPE_DELETE); }
    ;

create_stmt: LARX_CREATE optional_temp_clause LARX_TABLE optional_if_not_exists_clause 
    identifier '(' column_name_list ')' 
    { PUSHSTACK(Larxer::TYPE_CREATE); }
    ;

drop_stmt: LARX_DROP LARX_TABLE identifier
    { PUSHSTACK(Larxer::TYPE_DROP); } 
    ;

alter_stmt: LARX_ALTER LARX_TABLE identifier
    { PUSHSTACK(Larxer::TYPE_ALTER); } 
    ;

optional_sql_terminator:
    | ';'
    ;

optional_work:
    | LARX_WORK
    ;

optional_temp_clause:
    | LARX_TEMPORARY { PUSHSTACK(Larxer::TYPE_temporary_table); }
    ;

optional_if_not_exists_clause:
    | LARX_IF LARX_NOT LARX_EXISTS { PUSHSTACK(Larxer::TYPE_if_not_exists); }
    ;

column_name_list: ddl_column_name
    | column_name_list ',' ddl_column_name
    ;

ddl_column_name:
    identifier data_type_name { PUSHSTACK2(Larxer::TYPE_data_type, $2); }
    ;

set_quantifier: LARX_DISTINCT { PUSHSTACK(Larxer::TYPE_DISTINCT); }
    | LARX_ALL { PUSHSTACK(Larxer::TYPE_ALL); } ;

columns: asterisk { PUSHSTACK2(Larxer::TYPE_COLUMNS, (int64_t)1); $$ = 1; }
    | aggregate_expression_list { PUSHSTACK2(Larxer::TYPE_COLUMNS, $1); $$ = $1; } ;

asterisk: '*' { PUSHSTACK(Larxer::TYPE_ASTERISK); } ;

from_clause: LARX_FROM identifier { PUSHSTACK(Larxer::TYPE_FROM); } ;

/* each individual item already pushed */
aggregate_expression_list: aggregate { $$ = 1; }
    | identifier { $$ = 1; }
    | aggregate_expression_list ',' aggregate { $$ = $1 + 1; }
    | aggregate_expression_list ',' identifier { $$ = $1 + 1; } ;

/* identifier already pushed */
identifierlist: identifier
    | identifierlist ',' identifier ;

identifier: LARX_identifier
      {
        PUSHOPERAND(OPERAND_IDENTIFIER, $1);
        free($1);
      }
    | LARX_identifier '.' LARX_identifier
      {
        string s($1);
        s.append(1, '.');
        s.append($3);
        PUSHOPERAND(OPERAND_IDENTIFIER, s.c_str());
        free($1);
        free($3);
      }
    | LARX_identifier '.' asterisk
      {
        string s($1);
        s.append(".*");
        PUSHOPERAND(OPERAND_IDENTIFIER, s.c_str());
        free($1);
      }
    ;

data_type_name:
      LARX_CHARACTER LARX_VARYING { $$ = Schema::data_type_e::CHARACTER_VARYING; }
    | LARX_TEXT      { $$ = Schema::data_type_e::TEXT;     }
    | LARX_BIGINT    { $$ = Schema::data_type_e::BIGINT;   }
    | LARX_INTEGER   { $$ = Schema::data_type_e::INT;      }
    | LARX_INT       { $$ = Schema::data_type_e::INT;      }
    | LARX_DOUBLE    { $$ = Schema::data_type_e::DOUBLE;   }
    | LARX_FLOAT     { $$ = Schema::data_type_e::FLOAT;    }
    | LARX_DECIMAL   { $$ = Schema::data_type_e::DECIMAL;  }
    | LARX_BOOLEAN   { $$ = Schema::data_type_e::BOOLEAN;  }
    | LARX_DATETIME  { $$ = Schema::data_type_e::DATETIME; }
    | LARX_DATE      { $$ = Schema::data_type_e::DATE;     }
    | LARX_INTERVAL  { $$ = Schema::data_type_e::INTERVAL; }
    ;

/* aggregate function and identifier already pushed */
aggregate: LARX_AVG '(' LARX_identifier ')' { PUSHAGGREGATE(AGGREGATE_AVG, $3); free($3); }
    | LARX_COUNT '(' LARX_identifier ')' { PUSHAGGREGATE(AGGREGATE_COUNT, $3); free($3); }
    | LARX_MAX '(' LARX_identifier ')' { PUSHAGGREGATE(AGGREGATE_MAX, $3); free($3); }
    | LARX_MIN '(' LARX_identifier ')' { PUSHAGGREGATE(AGGREGATE_MIN, $3); free($3); }
    | LARX_SUM '(' LARX_identifier ')' { PUSHAGGREGATE(AGGREGATE_SUM, $3); free($3); }
    ;

where_clause:
    | LARX_WHERE search_condition { PUSHSTACK(Larxer::TYPE_WHERE); } ;

group_by_clause:
    | LARX_GROUP LARX_BY identifierlist { PUSHSTACK(Larxer::TYPE_GROUPBY); } ;

having_clause:
    | LARX_HAVING search_condition { PUSHSTACK(Larxer::TYPE_HAVING); } ;

for_update_clause:
    | LARX_FOR LARX_UPDATE { PUSHSTACK(Larxer::TYPE_FORUPDATE); } ;

no_lock_clause:
    | LARX_NO LARX_LOCK { PUSHSTACK(Larxer::TYPE_NOLOCK); } ;

order_by_clause:
    | LARX_ORDER LARX_BY sort_specificationlist
      { PUSHSTACK(Larxer::TYPE_ORDERBY); }
    ;

/* sub-condition pushed */
sort_specificationlist: sort_specification
    | sort_specificationlist ',' sort_specification ;

/* already pushed */
sort_specification: identifier order
    { PUSHSTACK(Larxer::TYPE_SORTSPECIFICATION); } ;

order: { PUSHSTACK(Larxer::TYPE_ASC); }
    | LARX_ASC { PUSHSTACK(Larxer::TYPE_ASC); }
    | LARX_DESC { PUSHSTACK(Larxer::TYPE_DESC); }
    ;

/* each individual expression already pushed */
expressionlist: expression
    | expressionlist ',' expression ;

/* this is so expressions are delimited on stack */
expression: expression2 { PUSHSTACK(Larxer::TYPE_EXPRESSION); } ;

expression2: expression2 LARX_concat expression2 { PUSHOPERATOR(OPERATOR_CONCATENATION); }
    | expression2 '+' expression2 { PUSHOPERATOR(OPERATOR_ADDITION); }
    | expression2 '-' expression2 { PUSHOPERATOR(OPERATOR_SUBTRACTION); }
    | expression2 '*' expression2 { PUSHOPERATOR(OPERATOR_MULTIPLICATION); }
    | expression2 '/' expression2 { PUSHOPERATOR(OPERATOR_DIVISION); }
    | '(' expression2 ')' /* no need to push anything here */
    | '-' '(' expression2 ')' {PUSHOPERATOR(OPERATOR_NEGATION); }
    | '+' '(' expression2 ')'
/*    | '-' expression2 %prec LARX_uminus { PUSHOPERATOR(OPERATOR_NEGATION); }
    | '+' expression2 %prec LARX_uplus */
    | operand ; /* has already been pushed by operand rule */

operandlist: operand
    | operandlist ',' operand ;

operand: number
    | LARX_stringval { PUSHOPERAND(OPERAND_STRING, $1); free($1); }
    | identifier /* has already been pushed by identifier rule */
    | LARX_parameter { PUSHOPERAND(OPERAND_PARAMETER, $1); }
    | subquery /* already pushed by subquery rule */
    | aggregate /* already pushed by aggregate rule */
    | LARX_NULL { PUSHOPERANDNULL; }
    | LARX_TRUE { PUSHOPERAND(OPERAND_BOOLEAN, "t"); }
    | LARX_FALSE { PUSHOPERAND(OPERAND_BOOLEAN, "f"); }
    ;

number: LARX_intval { PUSHOPERAND(OPERAND_INTEGER, $1); }
    | LARX_floatval { PUSHOPERAND(OPERAND_FLOAT, $1); }
    | '-' LARX_intval {PUSHOPERAND(OPERAND_INTEGER, 0-$2); }
    | '-' LARX_floatval {PUSHOPERAND(OPERAND_FLOAT, 0-$2); }
    | '+' LARX_intval {PUSHOPERAND(OPERAND_INTEGER, $2); }
    | '+' LARX_floatval {PUSHOPERAND(OPERAND_FLOAT, $2); }
    ;

search_condition: search_condition2
    { PUSHSTACK(Larxer::TYPE_search_condition); } ;

search_condition2: predicate /* already pushed */
    | search_condition2 truthtest /* already pushed */
    | '(' search_condition2 ')' /* no need to push */
    | LARX_NOT search_condition2 { PUSHOPERATOR(OPERATOR_NOT); }
    | search_condition2 LARX_AND search_condition2 { PUSHOPERATOR(OPERATOR_AND); }
    | search_condition2 LARX_OR search_condition2 { PUSHOPERATOR(OPERATOR_OR); }
    ;

/* already pushed */
truthtest: LARX_IS truthvalue ;

truthvalue: LARX_TRUE { PUSHOPERATOR(OPERATOR_TRUE); }
    | LARX_FALSE { PUSHOPERATOR(OPERATOR_FALSE); }
    | LARX_UNKNOWN { PUSHOPERATOR(OPERATOR_UNKNOWN); }
    | LARX_NULL { PUSHOPERATOR(OPERATOR_NULL); }
    ;

/* already pushed */
predicate: comparison_predicate
    | between_predicate
    | null_predicate
    | in_predicate
    | like_predicate
    | existsunique_predicate
    | notbetween_predicate
    | notnull_predicate
    | notin_predicate
    | notlike_predicate ;

comparison_predicate: expression '=' expression { PUSHOPERATOR(OPERATOR_EQ); }
    | expression LARX_ne expression { PUSHOPERATOR(OPERATOR_NE); }
    | expression '<' expression { PUSHOPERATOR(OPERATOR_LT); }
    | expression '>' expression { PUSHOPERATOR(OPERATOR_GT); }
    | expression LARX_lte expression { PUSHOPERATOR(OPERATOR_LTE); }
    | expression LARX_gte expression { PUSHOPERATOR(OPERATOR_GTE); }
    ;

between_predicate: expression LARX_BETWEEN betweenexpressions
        { PUSHOPERATOR(OPERATOR_BETWEEN); } ;

notbetween_predicate: expression LARX_NOT LARX_BETWEEN betweenexpressions
        { PUSHOPERATOR(OPERATOR_NOTBETWEEN); } ;

betweenexpressions:
    expression LARX_AND expression
      { PUSHOPERATOR(OPERATOR_BETWEENAND); }
    ;

null_predicate: expression LARX_IS LARX_NULL
    { PUSHOPERATOR(OPERATOR_ISNULL); } ;

notnull_predicate: expression LARX_IS LARX_NOT LARX_NULL
    { PUSHOPERATOR(OPERATOR_ISNOTNULL); } ;

in_predicate: expression in in_object
    { PUSHOPERATOR(OPERATOR_IN); } ;

notin_predicate: expression LARX_NOT in in_object
    { PUSHOPERATOR(OPERATOR_NOTIN); } ;

in: LARX_IN { PUSHSTACK(Larxer::TYPE_inbegin); } ;

in_object: '(' expressionlist ')' { PUSHSTACK(Larxer::TYPE_inobject); }
    | subquery { PUSHSTACK(Larxer::TYPE_inobject); } ;

subquery: '(' select_stmt ')' { PUSHOPERANDSUBQUERY; } ;

like_predicate: expression LARX_LIKE LARX_stringval
      {
        PUSHOPERAND(OPERAND_STRING, $3);
        free($3);
        PUSHOPERATOR(OPERATOR_LIKE);
       }
    ;

notlike_predicate: expression LARX_NOT LARX_LIKE LARX_stringval
      {
        PUSHOPERAND(OPERAND_STRING, $4);
        free($4);
        PUSHOPERATOR(OPERATOR_NOTLIKE);
       }
    ;

existsunique_predicate: LARX_EXISTS subquery { PUSHOPERATOR(OPERATOR_EXISTS); }
    | LARX_UNIQUE subquery { PUSHOPERATOR(OPERATOR_UNIQUE); } ;

setassignmentlist: setassignment
    | setassignmentlist ',' setassignment ;


setassignment: identifier '=' expression
      { PUSHSTACK(Larxer::TYPE_assignment); }
    ;
%%

void yyerror(void *scanner, char *str)
{
}
