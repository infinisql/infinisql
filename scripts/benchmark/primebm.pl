#! /usr/bin/env perl

# Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
# All rights reserved. No warranty, explicit or implicit, provided.
#
# This file is part of InfiniSQL (tm). It is available either under the
# GNU Affero Public License or under a commercial license. Contact the
# copyright holder for information about a commercial license if terms
# of the GNU Affero Public License do not suit you.
#
# This copy of InfiniSQL is free software: you can redistribute it and/or
# modify it under the terms of the GNU Affero Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# InfiniSQL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero Public License for more details.
#
# You should have received a copy of the GNU Affero Public License
# along with InfiniSQL. It should be in the top level of the source
# directory in a file entitled "COPYING".
# If not, see <http://www.gnu.org/licenses/>.

use Getopt::Std;
getopt('h');
$HOSTNAME=$opt_h;

#$HOSTNAME=$ARGV[0];
require '../infinisql.plib';

&connect($HOSTNAME, 11521);
print "logging in\n";
&describeresponse(&send("login", "_global", "admin", "passw0rd"));
print "createdomain benchmark:\n";
&describeresponse(&send("createdomain", "benchmark"));
print "creating user\n";
&describeresponse(&send("createuser", "benchmark", "benchmark", "benchmark"));
print "logout\n";
&send("logout");
print "connecting\n";
&connect($HOSTNAME, 11521);
print "logging in domain benchmark:\n";
&describeresponse(&send("login", "benchmark", "benchmark", "benchmark"));
print "creating schema\n";
&describeresponse(&send("createschema"));
print "createtable pgbench_accounts, pgbench_branches, pgbench_history,\n",
    "pbbench_tellers:\n";
&describeresponse(&send("createtable", "pgbench_accounts"));
&describeresponse(&send("createtable", "pgbench_branches"));
&describeresponse(&send("createtable", "pgbench_history"));
&describeresponse(&send("createtable", "pgbench_tellers"));

print "columns for pgbench_accounts:\n";
&describeresponse(&send("addcolumn", "1", "int", "0", "aid", "uniquenotnull"));
&describeresponse(&send("addcolumn", "1", "int", "0", "bid", "none"));
&describeresponse(&send("addcolumn", "1", "int", "0", "abalance", "none"));
&describeresponse(&send("addcolumn", "1", "charx", "72", "filler", "none"));

print "columns for pgbench_branches:\n";
&describeresponse(&send("addcolumn", "2", "int", "0", "bid", "uniquenotnull"));
&describeresponse(&send("addcolumn", "2", "int", "0", "bbalance", "none"));
&describeresponse(&send("addcolumn", "2", "charx", "80", "filler", "none"));

print "columns for pgbench_history:\n";
&describeresponse(&send("addcolumn", "3", "int", "0", "tid", "none"));
&describeresponse(&send("addcolumn", "3", "int", "0", "bid", "none"));
&describeresponse(&send("addcolumn", "3", "int", "0", "aid", "none"));
&describeresponse(&send("addcolumn", "3", "int", "0", "delta", "none"));
&describeresponse(&send("addcolumn", "3", "charx", "14", "filler", "none"));

print "columns for pgbench_tellers:\n";
&describeresponse(&send("addcolumn", "4", "int", "0", "tid", "uniquenotnull"));
&describeresponse(&send("addcolumn", "4", "int", "0", "bid", "none"));
&describeresponse(&send("addcolumn", "4", "int", "0", "tbalance", "none"));
&describeresponse(&send("addcolumn", "4", "charx", "72", "filler", "none"));

print "\n";
$stmtname="pgbench_updateaccounts";
$stmt="UPDATE pgbench_accounts SET abalance = abalance + :0 where aid = :1";
print "compiling $stmtname: $stmt\n";
&describeresponse(&send("compile", $stmtname, $stmt));

$stmtname="pgbench_selectaccounts";
$stmt="SELECT abalance from pgbench_accounts WHERE aid = :0";
print "compiling $stmtname: $stmt\n";
&describeresponse(&send("compile", $stmtname, $stmt));

$stmtname="pgbench_updatetellers";
$stmt="UPDATE pgbench_tellers SET tbalance = tbalance + :0 where tid = :1";
print "compiling $stmtname: $stmt\n";
&describeresponse(&send("compile", $stmtname, $stmt));

$stmtname="pgbench_updatebranches";
$stmt="UPDATE pgbench_branches SET bbalance = bbalance + :0 where bid = :1";
print "compiling $stmtname: $stmt\n";
&describeresponse(&send("compile", $stmtname, $stmt));

$stmtname="pgbench_inserthistory";
$stmt="INSERT INTO pgbench_history VALUES (:0, :1, :2, :3, '')";
print "compiling $stmtname: $stmt\n";
&describeresponse(&send("compile", $stmtname, $stmt));

print "\nloading stored procedure Pgbench\n";
&describeresponse(&send("loadprocedure", "$PROCDIR/PgbenchProc.so", "Pgbench"));

print "\nloading stored procedure PgbenchNoinsertProc\n";
&describeresponse(&send("loadprocedure", "$PROCDIR/PgbenchNoinsertProc.so", "PgbenchNoinsert"));

print "\ncreating table keyvaltable\n";
&describeresponse(&send("createtable", "keyvaltable"));
&describeresponse(&send("addcolumn", "5", "int", "0", "keyentry", "uniquenotnull"));
&describeresponse(&send("addcolumn", "5", "varchar", "0", "val", "none"));

print "\ncompiling keyval statements for SetkeyProc\n";
$stmtname="keyval_insert";
$stmt="INSERT INTO keyvaltable VALUES (:0, :1)";
print "compiling $stmtname: $stmt\n";
&describeresponse(&send("compile", $stmtname, $stmt));

$stmtname="keyval_update";
$stmt="UPDATE keyvaltable SET val = :1 where keyentry = :0";
print "compiling $stmtname: $stmt\n";
&describeresponse(&send("compile", $stmtname, $stmt));

$stmtname="keyval_select";
$stmt = "SELECT val FROM keyvaltable WHERE keyentry = :0 NO LOCK";
print "compiling $stmtname: $stmt\n";
&describeresponse(&send("compile", $stmtname, $stmt));

print "\nloading stored procedure Setkey\n";
&describeresponse(&send("loadprocedure", "$PROCDIR/SetkeyProc.so", "Setkey"));

print "\nloading stored procedure Getkey\n";
&describeresponse(&send("loadprocedure", "$PROCDIR/GetkeyProc.so", "Getkey"));

