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

%option noyywrap nodefault case-insensitive yylineno
%option reentrant
%option bison-bridge
%option nounput noinput
%{
#include "infinisql_gch.h"
#include "infinisql_Larxer.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define YY_EXTRA_TYPE struct perlarxer *
%}

%%

ABSOLUTE { return LARX_ABSOLUTE; }
ACTION { return LARX_ACTION; }
ADD { return LARX_ADD; }
ALL { return LARX_ALL; }
ALLOCATE { return LARX_ALLOCATE; }
ALTER { return LARX_ALTER; }
AND { return LARX_AND; }
ANY { return LARX_ANY; }
ARE { return LARX_ARE; }
AS { return LARX_AS; }
ASC { return LARX_ASC; }
ASCENDING { return LARX_ASC; }
ASSERTION { return LARX_ASSERTION; }
AT { return LARX_AT; }
AUTHORIZATION { return LARX_AUTHORIZATION; }
AVG { return LARX_AVG; }
BEGIN { return LARX_BEGIN; }
BETWEEN { return LARX_BETWEEN; }
BIT { return LARX_BIT; }
BIT_LENGTH { return LARX_BIT_LENGTH; }
BOTH { return LARX_BOTH; }
BY { return LARX_BY; }
CASCADE { return LARX_CASCADE; }
CASCADED { return LARX_CASCADED; }
CASE { return LARX_CASE; }
CAST { return LARX_CAST; }
CATALOG { return LARX_CATALOG; }
CHAR { return LARX_CHAR; }
CHARACTER { return LARX_CHARACTER; }
CHARACTER_LENGTH { return LARX_CHARACTER_LENGTH; }
CHAR_LENGTH { return LARX_CHAR_LENGTH; }
CHECK { return LARX_CHECK; }
CLOSE { return LARX_CLOSE; }
COALESCE { return LARX_COALESCE; }
COLLATE { return LARX_COLLATE; }
COLLATION { return LARX_COLLATION; }
COLUMN { return LARX_COLUMN; }
COMMIT { return LARX_COMMIT; }
CONNECT { return LARX_CONNECT; }
CONNECTION { return LARX_CONNECTION; }
CONSTRAINT { return LARX_CONSTRAINT; }
CONSTRAINTS { return LARX_CONSTRAINTS; }
CONTINUE { return LARX_CONTINUE; }
CONVERT { return LARX_CONVERT; }
COUNT { return LARX_COUNT; }
CORRESPONDING { return LARX_CORRESPONDING; }
CREATE { return LARX_CREATE; }
CROSS { return LARX_CROSS; }
CURRENT { return LARX_CURRENT; }
CURRENT_DATE { return LARX_CURRENT_DATE; }
CURRENT_TIME { return LARX_CURRENT_TIME; }
CURRENT_TIMESTAMP { return LARX_CURRENT_TIMESTAMP; }
CURRENT_USER { return LARX_CURRENT_USER; }
CURSOR { return LARX_CURSOR; }
DATE { return LARX_DATE; }
DAY { return LARX_DAY; }
DEALLOCATE { return LARX_DEALLOCATE; }
DEC { return LARX_DEC; }
DECIMAL { return LARX_DECIMAL; }
DECLARE { return LARX_DECLARE; }
DEFAULT { return LARX_DEFAULT; }
DEFERRABLE { return LARX_DEFERRABLE; }
DEFERRED { return LARX_DEFERRED; }
DELETE { return LARX_DELETE; }
DESC { return LARX_DESC; }
DESCENDING { return LARX_DESC; }
DESCRIBE { return LARX_DESCRIBE; }
DESCRIPTOR { return LARX_DESCRIPTOR; }
DIAGNOSTICS { return LARX_DIAGNOSTICS; }
DISCONNECT { return LARX_DISCONNECT; }
DISTINCT { return LARX_DISTINCT; }
DOMAIN { return LARX_DOMAIN; }
DOUBLE { return LARX_DOUBLE; }
DROP { return LARX_DROP; }
ELSE { return LARX_ELSE; }
END { return LARX_COMMIT; }
END-EXEC { return LARX_END_EXEC; }
ESCAPE { return LARX_ESCAPE; }
EXCEPT { return LARX_EXCEPT; }
EXCEPTION { return LARX_EXCEPTION; }
EXEC { return LARX_EXEC; }
EXECUTE { return LARX_EXECUTE; }
EXISTS { return LARX_EXISTS; }
EXTERNAL { return LARX_EXTERNAL; }
EXTRACT { return LARX_EXTRACT; }
FALSE { return LARX_FALSE; }
FETCH { return LARX_FETCH; }
FIRST { return LARX_FIRST; }
FLOAT { return LARX_FLOAT; }
FOR { return LARX_FOR; }
FOREIGN { return LARX_FOREIGN; }
FOUND { return LARX_FOUND; }
FROM { return LARX_FROM; }
FULL { return LARX_FULL; }
GET { return LARX_GET; }
GLOBAL { return LARX_GLOBAL; }
GO { return LARX_GO; }
GOTO { return LARX_GOTO; }
GRANT { return LARX_GRANT; }
GROUP { return LARX_GROUP; }
HAVING { return LARX_HAVING; }
HOUR { return LARX_HOUR; }
IDENTITY { return LARX_IDENTITY; }
IMMEDIATE { return LARX_IMMEDIATE; }
IN { return LARX_IN; }
INDICATOR { return LARX_INDICATOR; }
INITIALLY { return LARX_INITIALLY; }
INNER { return LARX_INNER; }
INPUT { return LARX_INPUT; }
INSENSITIVE { return LARX_INSENSITIVE; }
INSERT { return LARX_INSERT; }
INT { return LARX_INT; }
INTEGER { return LARX_INTEGER; }
INTERSECT { return LARX_INTERSECT; }
INTERVAL { return LARX_INTERVAL; }
INTO { return LARX_INTO; }
IS { return LARX_IS; }
ISOLATION { return LARX_ISOLATION; }
JOIN { return LARX_JOIN; }
KEY { return LARX_KEY; }
LANGUAGE { return LARX_LANGUAGE; }
LAST { return LARX_LAST; }
LEADING { return LARX_LEADING; }
LEFT { return LARX_LEFT; }
LEVEL { return LARX_LEVEL; }
LIKE { return LARX_LIKE; }
LOCAL { return LARX_LOCAL; }
LOWER { return LARX_LOWER; }
MATCH { return LARX_MATCH; }
MAX { return LARX_MAX; }
MIN { return LARX_MIN; }
MINUTE { return LARX_MINUTE; }
MODULE { return LARX_MODULE; }
MONTH { return LARX_MONTH; }
NAMES { return LARX_NAMES; }
NATIONAL { return LARX_NATIONAL; }
NATURAL { return LARX_NATURAL; }
NCHAR { return LARX_NCHAR; }
NEXT { return LARX_NEXT; }
NO { return LARX_NO; }
NOT { return LARX_NOT; }
NULL { return LARX_NULL; }
NULLIF { return LARX_NULLIF; }
NUMERIC { return LARX_NUMERIC; }
OCTET_LENGTH { return LARX_OCTET_LENGTH; }
OF { return LARX_OF; }
ON { return LARX_ON; }
ONLY { return LARX_ONLY; }
OPEN { return LARX_OPEN; }
OPTION { return LARX_OPTION; }
OR { return LARX_OR; }
ORDER { return LARX_ORDER; }
OUTER { return LARX_OUTER; }
OUTPUT { return LARX_OUTPUT; }
OVERLAPS { return LARX_OVERLAPS; }
PAD { return LARX_PAD; }
PARTIAL { return LARX_PARTIAL; }
POSITION { return LARX_POSITION; }
PRECISION { return LARX_PRECISION; }
PREPARE { return LARX_PREPARE; }
PRESERVE { return LARX_PRESERVE; }
PRIMARY { return LARX_PRIMARY; }
PRIOR { return LARX_PRIOR; }
PRIVILEGES { return LARX_PRIVILEGES; }
PROCEDURE { return LARX_PROCEDURE; }
PUBLIC { return LARX_PUBLIC; }
READ { return LARX_READ; }
REAL { return LARX_REAL; }
REFERENCES { return LARX_REFERENCES; }
RELATIVE { return LARX_RELATIVE; }
RESTRICT { return LARX_RESTRICT; }
REVOKE { return LARX_REVOKE; }
RIGHT { return LARX_RIGHT; }
ROLLBACK { return LARX_ROLLBACK; }
ROWS { return LARX_ROWS; }
SCHEMA { return LARX_SCHEMA; }
SCROLL { return LARX_SCROLL; }
SECOND { return LARX_SECOND; }
SECTION { return LARX_SECTION; }
SELECT { return LARX_SELECT; }
SESSION { return LARX_SESSION; }
SESSION_USER { return LARX_SESSION_USER; }
SET { return LARX_SET; }
SIZE { return LARX_SIZE; }
SMALLINT { return LARX_SMALLINT; }
SOME { return LARX_SOME; }
SPACE { return LARX_SPACE; }
SQL { return LARX_SQL; }
SQLCODE { return LARX_SQLCODE; }
SQLERROR { return LARX_SQLERROR; }
SQLSTATE { return LARX_SQLSTATE; }
SUBSTRING { return LARX_SUBSTRING; }
SUM { return LARX_SUM; }
SYSTEM_USER { return LARX_SYSTEM_USER; }
TABLE { return LARX_TABLE; }
TEMPORARY { return LARX_TEMPORARY; }
THEN { return LARX_THEN; }
TIME { return LARX_TIME; }
TIMESTAMP { return LARX_TIMESTAMP; }
TIMEZONE_HOUR { return LARX_TIMEZONE_HOUR; }
TIMEZONE_MINUTE { return LARX_TIMEZONE_MINUTE; }
TO { return LARX_TO; }
TRAILING { return LARX_TRAILING; }
TRANSACTION { return LARX_TRANSACTION; }
TRANSLATE { return LARX_TRANSLATE; }
TRANSLATION { return LARX_TRANSLATION; }
TRIM { return LARX_TRIM; }
TRUE { return LARX_TRUE; }
UNION { return LARX_UNION; }
UNIQUE { return LARX_UNIQUE; }
UNKNOWN { return LARX_UNKNOWN; }
UPDATE { return LARX_UPDATE; }
UPPER { return LARX_UPPER; }
USAGE { return LARX_USAGE; }
USER { return LARX_USER; }
USING { return LARX_USING; }
VALUE { return LARX_VALUE; }
VALUES { return LARX_VALUES; }
VARCHAR { return LARX_VARCHAR; }
VARYING { return LARX_VARYING; }
VIEW { return LARX_VIEW; }
WHEN { return LARX_WHEN; }
WHENEVER { return LARX_WHENEVER; }
WHERE { return LARX_WHERE; }
WITH { return LARX_WITH; }
WORK { return LARX_WORK; }
WRITE { return LARX_WRITE; }
YEAR { return LARX_YEAR; }
ZONE { return LARX_ZONE; }

LOCK { return LARX_LOCK; }

"--".* ;
'(''|[^'])*' { yylval->str = strndup(yytext+1, strlen(yytext)-2);
    return LARX_stringval; }
":"[0-9]+ { yylval->integer = atol(yytext+1);
    return LARX_parameter; }
[0-9]+ { yylval->integer = atol(yytext); return LARX_intval; }
[0-9]*"."[0-9]+ { yylval->floating = strtold(yytext, NULL);
    return LARX_floatval; }
[A-Za-z][a-zA-Z0-9_]* { yylval->str = strdup(yytext); return LARX_identifier; }
\"(^\"|\"\")*\" { yylval->str = strndup(yytext+1, strlen(yytext)-2);
    return LARX_identifier; }

"<>" { return LARX_ne; }
">=" { return LARX_gte; }
"<=" { return LARX_lte; }
"||" { return LARX_concat; }

"$" { return '$'; }
"\"" { return '"'; }
"%" { return '%'; }
"&" { return '&'; }
"'" { return '\''; }
"(" { return '('; }
")" { return ')'; }
"*" { return '*'; }
"+" { return '+'; }
"," { return ','; }
"-" { return '-'; }
"." { return '.'; }
"/" { return '/'; }
":" { return ':'; }
";" { return ';'; }
"<" { return '<'; }
">" { return '>'; }
"=" { return '='; }
"?" { return '?'; }
"_" { return '_'; }
"|" { return '|'; }
"[" { return '['; }
"]" { return ']'; }
"\\" { return '\\'; }

[ \t\n\r]+ ;

. { printf("flex bad character: %c\n", yytext[0]); }

%%

void flexinit(struct perlarxer *pld)
{
  yylex_init_extra(pld, &pld->scaninfo);
}

void flexbuffer(char *instr, size_t len, void *scaninfo)
{
  yy_scan_bytes(instr, len, scaninfo);
}

void flexdestroy(void *scaninfo)
{
  yylex_destroy(scaninfo);
}

