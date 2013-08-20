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

require 'infinisql.plib';
use Getopt::Std;
getopt('h');
$HOSTNAME=$opt_h;

&connect($HOSTNAME, 11521);
print "logging in\n";
&describeresponse(&send("login", "_global", "admin", "passw0rd"));
print "createdomain texas:\n";
&describeresponse(&send("createdomain", "texas"));
print "creating user\n";
&describeresponse(&send("createuser", "texas", "mayor", "austin"));
print "logout\n";
&send("logout");
print "connecting\n";
&connect($HOSTNAME, 11521);
print "logging in domain texas:\n";
&describeresponse(&send("login", "texas", "mayor", "austin"));
print "creating schema\n";
&describeresponse(&send("createschema"));
print "createtable:\n";
&describeresponse(&send("createtable", "mastertable"));
print "add column1\n";
&describeresponse(&send("addcolumn", "1", "int", "0", "accountid", "uniquenotnull"));
print "add column2\n";
&describeresponse(&send("addcolumn", "1", "bool", "0", "isactive", "none"));
print "add column3\n";
&describeresponse(&send("addcolumn", "1", "int", "0", "balance", "none"));

print "creating table manycolumns\n";
&describeresponse(&send("createtable", "manycolumns"));
&describeresponse(&send("addcolumn", "2", "int", "0", "intcol1", "uniquenotnull"));
&describeresponse(&send("addcolumn", "2", "bool", "0", "boolcol1", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "2", "float", "0", "floatcol1", "uniquenotnull"));
&describeresponse(&send("addcolumn", "2", "char", "0", "charcol1", "uniquenotnull"));
&describeresponse(&send("addcolumn", "2", "charx", "10", "charxcol1", "uniquenotnull"));
&describeresponse(&send("addcolumn", "2", "varchar", "0", "varcharcol1", "uniquenotnull"));
&describeresponse(&send("addcolumn", "2", "int", "0", "intcol2", "unique"));
&describeresponse(&send("addcolumn", "2", "bool", "0", "boolcol2", "nonunique"));
&describeresponse(&send("addcolumn", "2", "float", "0", "floatcol2", "unique"));
&describeresponse(&send("addcolumn", "2", "char", "0", "charcol2", "unique"));
&describeresponse(&send("addcolumn", "2", "charx", "10", "charxcol2", "unique"));
&describeresponse(&send("addcolumn", "2", "varchar", "0", "varcharcol2", "unique"));
&describeresponse(&send("addcolumn", "2", "int", "0", "intcol3", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "2", "bool", "0", "boolcol3", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "2", "float", "0", "floatcol3", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "2", "char", "0", "charcol3", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "2", "charx", "10", "charxcol3", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "2", "varchar", "0", "varcharcol3", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "2", "int", "0", "intcol4", "nonunique"));
&describeresponse(&send("addcolumn", "2", "bool", "0", "boolcol4", "nonunique"));
&describeresponse(&send("addcolumn", "2", "float", "0", "floatcol4", "nonunique"));
&describeresponse(&send("addcolumn", "2", "char", "0", "charcol4", "nonunique"));
&describeresponse(&send("addcolumn", "2", "charx", "10", "charxcol4", "nonunique"));
&describeresponse(&send("addcolumn", "2", "varchar", "0", "varcharcol4", "nonunique"));

print "creating table liketable\n";
&describeresponse(&send("createtable", "liketable"));
&describeresponse(&send("addcolumn", "3", "charx", "30", "charxcol", "unique"));
&describeresponse(&send("addcolumn", "3", "varchar", "0", "varcharcol", "unique"));

print "creating table nulltable\n";
&describeresponse(&send("createtable", "nulltable"));
&describeresponse(&send("addcolumn", "4", "varchar", "0", "keycol", "uniquenotnull"));
&describeresponse(&send("addcolumn", "4", "int", "0", "intunique", "unique"));
&describeresponse(&send("addcolumn", "4", "int", "0", "intnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "4", "float", "0", "floatunique", "unique"));
&describeresponse(&send("addcolumn", "4", "float", "0", "floatnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "4", "bool", "0", "boolunique", "unique"));
&describeresponse(&send("addcolumn", "4", "bool", "0", "boolnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "4", "char", "0", "charunique", "unique"));
&describeresponse(&send("addcolumn", "4", "char", "0", "charnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "4", "charx", "10", "charxunique", "unique"));
&describeresponse(&send("addcolumn", "4", "charx", "10", "charxnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "4", "varchar", "0", "varcharunique", "unique"));
&describeresponse(&send("addcolumn", "4", "varchar", "0", "varcharnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "4", "int", "0", "intnone", "none"));

print "creating table nonuniquetable\n";
&describeresponse(&send("createtable", "nonuniquetable"));
&describeresponse(&send("addcolumn", "5", "varchar", "0", "keycol", "uniquenotnull"));
&describeresponse(&send("addcolumn", "5", "int", "0", "intnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "5", "int", "0", "intnonuniquenotnull", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "5", "bool", "0", "boolnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "5", "bool", "0", "boolnonuniquenotnull", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "5", "float", "0", "floatnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "5", "float", "0", "floatnonuniquenotnull", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "5", "char", "0", "charnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "5", "char", "0", "charnonuniquenotnull", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "5", "charx", "20", "charxnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "5", "charx", "20", "charxnonuniquenotnull", "nonuniquenotnull"));
&describeresponse(&send("addcolumn", "5", "varchar", "0", "varcharnonunique", "nonunique"));
&describeresponse(&send("addcolumn", "5", "varchar", "0", "varcharnonuniquenotnull", "nonuniquenotnull"));

print "creating tables for update/delete testing\n";
$tablenum=6;
foreach my $fieldtype ("int", "bool", "float", "char", "charx", "varchar") {
  foreach my $idxtype ("unique", "uniquenotnull", "nonunique", "nonuniquenotnull") {
    if ($fieldtype eq "bool" && ($idxtype eq "unique" || $idxtype eq "uniquenotnull")) {
      next;
    }
    &describeresponse(&send("createtable", $fieldtype . $idxtype . "table"));
    my $fieldlen="0";
    if ($fieldtype eq "charx") {
      $fieldlen="20";
    }
    &describeresponse(&send("addcolumn", "$tablenum", $fieldtype, $fieldlen,
        "$fieldtype${idxtype}1", $idxtype));
    &describeresponse(&send("addcolumn", "$tablenum", $fieldtype, $fieldlen,
        "$fieldtype${idxtype}2", $idxtype));
   $tablenum++;
  }
}

print "creating table for transfer procedures\n";
&describeresponse(&send("createtable", "accountstable"));
&describeresponse(&send("addcolumn", "$tablenum", "int", "0", "accountid", "uniquenotnull"));
&describeresponse(&send("addcolumn", "$tablenum", "int", "0", "balance", "none"));
$tablenum++;

# &describeresponse(&send("compile", $sid, $stmt));
$stmtname="debitbuyer";
$stmt="UPDATE accountstable SET balance = balance - :0 where accountid = :1";
print "compiling statement $stmtname:\n\t$stmt\n";
&describeresponse(&send("compile", $stmtname, $stmt));
$stmtname="creditseller";
$stmt="UPDATE accountstable SET balance = balance + :0 where accountid = :1";
print "compiling statement $stmtname:\n\t$stmt\n";
&describeresponse(&send("compile", $stmtname, $stmt));

print "loading SqlProc procedure\n";
&describeresponse(&send("loadprocedure", "$PROCDIR/SqlProc.so", "SqlProc"));

