/* type OID's taken from PostgreSQL-9.2.4/src/include/catalog/pg_type.h
 * This file falls under the following Copyright and license
 *
 * PostgreSQL Database Management System
 * (formerly known as Postgres, then as Postgres95)
 * 
 * Portions Copyright (c) 1996-2013, PostgreSQL Global Development Group
 *
 * Portions Copyright (c) 1994, The Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written agreement
 * is hereby granted, provided that the above copyright notice and this
 * paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/**
 * @file   pgoids.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:44:24 2013
 * 
 * @brief  Symbols necessary to implement the PostgreSQL Frontend/Backend
 * Protocol.
 * 
 * This file has the same license as the PostgreSQL project, and is not covered
 * by the GPL. This is because the contents were lifted from PostgreSQL source.
 */

#ifndef PGOID_H
#define PGOID_H

#define BOOLOID                 16 // boolean, 'true'/'false'
#define BYTEAOID                17 // variable-length string, binary values
                                   // escaped
#define CHAROID                 18 // single character
#define NAMEOID                 19 // 63-byte type for storing system identifiers
#define INT8OID                 20 // ~18 digit integer, 8-byte storage
#define INT2OID                 21 // -32 thousand to 32 thousand, 2-byte storage
#define INT2VECTOROID   22 // array of int2, used in system tables
#define INT4OID                 23 // -2 billion to 2 billion integer, 4-byte
                                   // storage
#define REGPROCOID              24 // registered procedure"
#define TEXTOID                 25 // variable-length string, no limit specified
#define OIDOID                  26 // object identifier(oid), maximum 4 billion
#define TIDOID          27 // (block, offset), physical location of tuple
#define XIDOID 28 // transaction id
#define CIDOID 29 // command identifier type, sequence in transaction id
#define OIDVECTOROID    30 // array of oids, used in system tables
#define JSONOID 114
#define XMLOID 142 // XML content
#define PGNODETREEOID   194 // string representing an internal node tree
#define POINTOID                600 // geometric point '(x, y)'
#define LSEGOID                 601 // geometric line segment '(pt1,pt2)'
#define PATHOID                 602 // geometric path '(pt1,...)'
#define BOXOID                  603 // geometric box '(lower left,upper right)'
#define POLYGONOID              604 // geometric polygon '(pt1,...)'
#define LINEOID                 628 // geometric line (not implemented)
#define FLOAT4OID 700 // single-precision floating point number, 4-byte storage
#define FLOAT8OID 701 // double-precision floating point number, 8-byte storage
#define ABSTIMEOID              702 // absolute, limited-range date and time
                                    // (Unix system time)
#define RELTIMEOID              703 // relative, limited-range time interval
                                    // (Unix delta time)
#define TINTERVALOID    704 // (abstime,abstime), time interval
#define UNKNOWNOID              705
#define CIRCLEOID               718 // geometric circle '(center,radius)'
#define CASHOID 790 // monetary amounts, $d,ddd.cc
#define MACADDROID 829 // XX:XX:XX:XX:XX:XX, MAC address
#define INETOID 869 // IP address/netmask, host address, netmask optional
#define CIDROID 650 // network IP address/netmask, network address
#define INT4ARRAYOID            1007
#define TEXTARRAYOID            1009
#define FLOAT4ARRAYOID 1021
#define ACLITEMOID              1033 // access control list
#define CSTRINGARRAYOID         1263
#define BPCHAROID               1042 // char(length), blank-padded string,
                                     // fixed storage length
#define VARCHAROID              1043 // varchar(length), non-blank-padded
                                     // string, variable storage length
#define DATEOID                 1082 // date
#define TIMEOID                 1083 // time of day
#define TIMESTAMPOID    1114 // date and time
#define TIMESTAMPTZOID  1184 // date and time with time zone
#define INTERVALOID             1186 // @ <number> <units>, time interval
#define TIMETZOID               1266 // time of day with time zone
#define BITOID   1560 // fixed-length bit string
#define VARBITOID         1562 // variable-length bit string
#define NUMERICOID              1700 // numeric(precision, decimal),
                                     // arbitrary precision number
#define REFCURSOROID    1790 // reference to cursor (portal name)
#define REGPROCEDUREOID 2202 // registered procedure (with args)
#define REGOPEROID              2203 // registered operator
#define REGOPERATOROID  2204 // registered operator (with args)
#define REGCLASSOID             2205 // registered class
#define REGTYPEOID              2206 // registered type
#define REGTYPEARRAYOID 2211
#define TSVECTOROID             3614 // text representation for text search
#define GTSVECTOROID    3642 // GiST index internal text representation for
                             // text search
#define TSQUERYOID              3615 // GiST index internal text representation
                                     // for text search
#define REGCONFIGOID    3734 // registered text search configuration
#define REGDICTIONARYOID        3769 // registered text search dictionary
#define INT4RANGEOID            3904 // range of integers
#define RECORDOID               2249
#define RECORDARRAYOID  2287
#define CSTRINGOID              2275
#define ANYOID                  2276
#define ANYARRAYOID             2277
#define VOIDOID                 2278
#define TRIGGEROID              2279
#define LANGUAGE_HANDLEROID             2280
#define INTERNALOID             2281
#define OPAQUEOID               2282
#define ANYELEMENTOID   2283
#define ANYNONARRAYOID  2776
#define ANYENUMOID              3500
#define FDW_HANDLEROID  3115
#define ANYRANGEOID             3831

#endif /* PHOID_H */
