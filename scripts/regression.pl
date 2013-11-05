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

use DBI;
require 'infinisql.plib';

use Getopt::Std;
getopt('h');
$HOSTNAME=$opt_h;

$dbh=DBI->connect("dbi:Pg:dbname=texas;host=$HOSTNAME;port=15432;sslmode=disable", "mayor", "austin", {pg_server_prepare => 0});

$SELECTQUERY="SELECT * FROM mastertable";

$QUERY="INSERT INTO mastertable VALUES (5, false, 88)";
print "$QUERY;\n";
$dbh->do($QUERY);
&outputselect($SELECTQUERY);
$QUERY="UPDATE mastertable set balance=7000";
print "$QUERY;\n";
$dbh->do($QUERY);
&outputselect($SELECTQUERY);

$QUERY="UPDATE mastertable set accountid=33";
print "$QUERY;\n";
$dbh->do($QUERY);
&outputselect($SELECTQUERY);

$QUERY="DELETE FROM mastertable";
print "$QUERY;\n";
$dbh->do($QUERY);
&outputselect($SELECTQUERY);

for (my $n=1; $n <= 10; $n++) {
  my $acctid=$n*5;
  my $balance=$acctid*100;
  $dbh->do("INSERT INTO mastertable VALUES ($acctid, true, $balance)");
}
&outputselect($SELECTQUERY);

&whereselect("accountid=25");
&whereselect("accountid<>25");
&whereselect("accountid<25");
&whereselect("accountid>25");
&whereselect("accountid<=25");
&whereselect("accountid>=25");
&whereselect("accountid BETWEEN 10 and 40");
&whereselect("accountid BETWEEN 22 and 22");
&whereselect("accountid BETWEEN 30 and 30");
&whereselect("accountid BETWEEN 43 and 4");
&whereselect("accountid NOT BETWEEN 10 and 40");
&whereselect("accountid NOT BETWEEN 22 and 22");
&whereselect("accountid NOT BETWEEN 30 and 30");
&whereselect("accountid NOT BETWEEN 43 and 4");
&whereselect("accountid IN (25)");
&whereselect("accountid IN (25, 100000, 38, 40)");
&whereselect("accountid IN (37, 12)");
&whereselect("accountid NOT IN (25)");
&whereselect("accountid NOT IN (25, 100000, 38, 40)");
&whereselect("accountid NOT IN (37, 12)");

# ASCII 65 is 'A'
# $_ = ( ' ' x $numOfChar ) . $_;
for (my $n=1; $n <= 10; $n++) {
  my @VALUES;
  my $acctid=$n*5;
  my $balance=$acctid*100;
  for (my $m=0; $m<4; $m++) {
    my $torf;
    if ($n%2) {
      $torf="true";
    } else {
      $torf="false";
    }
    push(@VALUES, $n*5*(10**$m), $torf, $n*5*(10**$m)+0.5,
        "\'" . chr(64+$n) . "\'", "\'" . (chr(64+$n) x 2) . "\'",
        "\'" . (chr(64+$n) x 3) . "\'");
  }
  $dbh->do("INSERT INTO manycolumns VALUES (" . join(",", @VALUES) . ")");
}
$SELECTQUERY="SELECT * FROM manycolumns";
&outputselect($SELECTQUERY);

foreach my $colnum (0, 1, 2, 3) {
  foreach my $op ("=", "<>", "<", ">", "<=", ">=") {
    # INT
    my $col=sprintf("intcol%i", $colnum+1);
    my $val=25*(10**$colnum);
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op $val");
    # FLOAT
    $col=sprintf("floatcol%i", $colnum+1);
    $val += 0.5;
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op $val");
    # CHAR
    $col=sprintf("charcol%i", $colnum+1);
    $val="\'" . chr(64+5) . "\'";
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op $val");
    # CHARX
    $col=sprintf("charxcol%i", $colnum+1);
    $val="\'" . (chr(64+5) x 2) . "\'";
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op $val");
    # CHARX
    $col=sprintf("varcharcol%i", $colnum+1);
    $val="\'" . (chr(64+5) x 3) . "\'";
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op $val");
    # BOOL
    if ($op eq "=" || $op eq "<>") {
      $col=sprintf("boolcol%i", $colnum+1);
      $val="true";
      $SELECTQUERY="SELECT $col from manycolumns";
      &whereselect("$col $op $val");
    }
  }
}

# BETWEEN, NOT BETWEEN, IN, NOTIN
foreach my $colnum (0, 1, 2, 3) {
  foreach my $op ("BETWEEN", "NOT BETWEEN") {
    # INT
    my $col=sprintf("intcol%i", $colnum+1);
    my $val1=10*(10**$colnum);
    my $val2=40*(10**$colnum);
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op $val1 AND $val2");
    $val1=22*(10**$colnum);
    $val2=22*(10**$colnum);
    &whereselect("$col $op $val1 and $val2");
    $val1=30*(10**$colnum);
    $val2=30*(10**$colnum);
    &whereselect("$col $op $val1 and $val2");
    $val1=43*(10**$colnum);
    $val2=4*(10**$colnum);
    &whereselect("$col $op $val1 and $val2");
    # FLOAT
    my $col=sprintf("floatcol%i", $colnum+1);
    my $val1=10*(10**$colnum) + 0.5;
    my $val2=40*(10**$colnum) + 0.5;
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op $val1 AND $val2");
    $val1=22*(10**$colnum) + 0.5;
    $val2=22*(10**$colnum) + 0.5;
    &whereselect("$col $op $val1 and $val2");
    $val1=30*(10**$colnum) + 0.5;
    $val2=30*(10**$colnum) + 0.5;
    &whereselect("$col $op $val1 and $val2");
    $val1=43*(10**$colnum) + 0.5;
    $val2=4*(10**$colnum) + 0.5;
    &whereselect("$col $op $val1 and $val2");
    # CHAR
    my $col=sprintf("charcol%i", $colnum+1);
    my $val1="\'" . chr(64+2) . "\'";
    my $val2="\'" . chr(64+8) . "\'";
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op $val1 AND $val2");
    my $val1="\'" . chr(64+20) . "\'";
    my $val2="\'" . chr(64+20) . "\'";
    &whereselect("$col $op $val1 and $val2");
    my $val1="\'" . chr(64+6) . "\'";
    my $val2="\'" . chr(64+6) . "\'";
    &whereselect("$col $op $val1 and $val2");
    my $val1="\'" . chr(64+22) . "\'";
    my $val2="\'" . chr(64+17) . "\'";
    &whereselect("$col $op $val1 and $val2");
    # CHARX
    my $col=sprintf("charxcol%i", $colnum+1);
    my $val1="\'" . (chr(64+2) x 2) . "\'";
    my $val2="\'" . (chr(64+8) x 2) . "\'";
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op $val1 AND $val2");
    my $val1="\'" . (chr(64+20) x 2) . "\'";
    my $val2="\'" . (chr(64+20) x 2) . "\'";
    &whereselect("$col $op $val1 and $val2");
    my $val1="\'" . (chr(64+6) x 2) . "\'";
    my $val2="\'" . (chr(64+6) x 2) . "\'";
    &whereselect("$col $op $val1 and $val2");
    my $val1="\'" . (chr(64+22) x 2) . "\'";
    my $val2="\'" . (chr(64+17) x 2) . "\'";
    &whereselect("$col $op $val1 and $val2");
    # VARCHAR
    my $col=sprintf("varcharcol%i", $colnum+1);
    my $val1="\'" . (chr(64+2) x 3) . "\'";
    my $val2="\'" . (chr(64+8) x 3) . "\'";
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op $val1 AND $val2");
    my $val1="\'" . (chr(64+20) x 3) . "\'";
    my $val2="\'" . (chr(64+20) x 3) . "\'";
    &whereselect("$col $op $val1 and $val2");
    my $val1="\'" . (chr(64+6) x 3) . "\'";
    my $val2="\'" . (chr(64+6) x 3) . "\'";
    &whereselect("$col $op $val1 and $val2");
    my $val1="\'" . (chr(64+22) x 3) . "\'";
    my $val2="\'" . (chr(64+17) x 3) . "\'";
    &whereselect("$col $op $val1 and $val2");
  }
}

foreach my $colnum (0, 1, 2, 3) {
  foreach my $op("IN", "NOT IN") {
    # INT
    my $col=sprintf("intcol%i", $colnum+1);
    my @vals = (25*(10**$colnum));
    my $valstr=join(",", @vals);
    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op ($valstr)");
    @vals = (25*(10**$colnum), 100000*(10**$colnum), 38*(10**$colnum),
        40*(10**$colnum));
    $valstr=join(",", @vals);
    &whereselect("$col $op ($valstr)");
    @vals = (37*(10**$colnum), 12*(10**$colnum));
    $valstr=join(",", @vals);
    &whereselect("$col $op ($valstr)");
    # FLOAT
    my $col=sprintf("floatcol%i", $colnum+1);
    my @vals = (25*(10**$colnum)+0.5);
    my $valstr=join(",", @vals);    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op ($valstr)");
    @vals = (25*(10**$colnum)+0.5, 100000*(10**$colnum)+0.5,
        38*(10**$colnum)+0.5, 40*(10**$colnum)+0.5);
    $valstr=join(",", @vals);
    &whereselect("$col $op ($valstr)");
    @vals = (37*(10**$colnum)+0.5, 12*(10**$colnum)+0.5);
    $valstr=join(",", @vals);
    &whereselect("$col $op ($valstr)");
   # CHAR
    my $col=sprintf("charcol%i", $colnum+1);
    my @vals = ("\'" . chr(64+5) . "\'");
    my $valstr=join(",", @vals);    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op ($valstr)");
    @vals = ("\'" . chr(64+5) . "\'", "\'" . chr(64+50) . "\'",
        "\'" . chr(64+15) . "\'", "\'" . chr(64+8) . "\'");
    $valstr=join(",", @vals);
    &whereselect("$col $op ($valstr)");
    @vals = ("\'" . chr(64+37) . "\'", "\'" . chr(64+12) . "\'");
    $valstr=join(",", @vals);
    &whereselect("$col $op ($valstr)");
    # CHARX
    my $col=sprintf("charxcol%i", $colnum+1);
    my @vals = ("\'" . chr(64+5)x2 . "\'");
    my $valstr=join(",", @vals);    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op ($valstr)");
    @vals = ("\'" . chr(64+5)x2 . "\'", "\'" . chr(64+50)x2 . "\'",
        "\'" . chr(64+15)x2 . "\'", "\'" . chr(64+8)x2 . "\'");
    $valstr=join(",", @vals);
    &whereselect("$col $op ($valstr)");
    @vals = ("\'" . chr(64+37)x2 . "\'", "\'" . chr(64+12)x2 . "\'");
    $valstr=join(",", @vals);
    &whereselect("$col $op ($valstr)");
    # VARCHAR
    my $col=sprintf("varcharcol%i", $colnum+1);
    my @vals = ("\'" . chr(64+5)x3 . "\'");
    my $valstr=join(",", @vals);    $SELECTQUERY="SELECT $col from manycolumns";
    &whereselect("$col $op ($valstr)");
    @vals = ("\'" . chr(64+5)x3 . "\'", "\'" . chr(64+50)x3 . "\'",
        "\'" . chr(64+15)x3 . "\'", "\'" . chr(64+8)x3 . "\'");
    $valstr=join(",", @vals);
    &whereselect("$col $op ($valstr)");
    @vals = ("\'" . chr(64+37)x3 . "\'", "\'" . chr(64+12)x3 . "\'");
    $valstr=join(",", @vals);
    &whereselect("$col $op ($valstr)");
  }
}

# LIKE & NOTLIKE
$INSERTQUERY="INSERT INTO liketable VALUES ('abcdefghijklmnopqrstuvwxyz', 'abcdefghijklmnopqrstuvwxyz')";
print "$INSERTQUERY\n";
$dbh->do($INSERTQUERY);
$SELECTQUERY="SELECT * from liketable";
&outputselect($SELECTQUERY);

foreach my $op("LIKE", "NOT LIKE") {
  # CHARX
  $col="charxcol";
  $likestr="\'" . "abcdefghijklmnopqrstuvwxyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABCDEFG" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "%xyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "%EFG" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abc%" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABC%" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abc%xyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABC%EFG" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abc%lmn%xyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABC%EFG%QRS" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "_bcdefghijklmnopqrstuvwxyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "_BCD" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abcdefghijklmnopqrstuvwxy_" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "DEF_" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abcdefghijklm_opqrstuvwxyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABCD_WXY" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abc%jkl_nopqrstuvwxyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABC%FGH_RSTU" . "\'";
  &whereselect("$col $op $likestr");
  # VARCHAR
  $col="varcharcol";
  $likestr="\'" . "abcdefghijklmnopqrstuvwxyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABCDEFG" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "%xyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "%EFG" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abc%" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABC%" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abc%xyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABC%EFG" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abc%lmn%xyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABC%EFG%QRS" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "_bcdefghijklmnopqrstuvwxyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "_BCD" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abcdefghijklmnopqrstuvwxy_" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "DEF_" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abcdefghijklm_opqrstuvwxyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABCD_WXY" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "abc%jkl_nopqrstuvwxyz" . "\'";
  &whereselect("$col $op $likestr");
  $likestr="\'" . "ABC%FGH_RSTU" . "\'";
  &whereselect("$col $op $likestr");
}

$INSERTQUERY="INSERT INTO nulltable VALUES ('has values', 1, 2, 3.3, 4.4, true, false, 'a', 'b', 'aa', 'bb', 'aaa', 'bbb', 5)";
$dbh->do($INSERTQUERY);
$INSERTQUERY="INSERT INTO nulltable VALUES ('null values1', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)";
$dbh->do($INSERTQUERY);
$INSERTQUERY="INSERT INTO nulltable VALUES ('null values2', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)";
$dbh->do($INSERTQUERY);
$SELECTQUERY="SELECT * FROM nulltable";
&outputselect($SELECTQUERY);
$SELECTQUERY="SELECT keycol FROM nulltable";
foreach my $op("IS NULL", "IS NOT NULL") {
  foreach my $col ("intunique", "intnonunique", "floatunique",
      "floatnonunique", "boolunique", "boolnonunique", "charunique",
      "charnonunique", "charxunique", "charxnonunique", "varcharunique",
      "varcharnonunique") {
    &whereselect("$col $op");
  }
}

for ($n=1; $n <= 10; $n++) {
  my @VALUES;
  my $torf;
  if ($n%2) {
    $torf="true";
  } else {
    $torf="false";
  }
  push(@VALUES, $n*5, $n*50, $torf, $torf, $n*5+0.5, $n*50+0.5, "\'" . chr(64+$n) . "\'", "\'" . chr(64+$n+32) . "\'", "\'" . (chr(64+$n)x2) . "\'", "\'" . (chr(64+$n+32)x2) . "\'", "\'" . (chr(64+$n)x3) . "\'", "\'" . (chr(64+$n+32)x3) . "\'");
  $INSERTQUERY="INSERT INTO nonuniquetable VALUES (" . join(",", "'control1-$n'", @VALUES) . ")";
  print "$INSERTQUERY\n";
  $dbh->do($INSERTQUERY);
  $INSERTQUERY="INSERT INTO nonuniquetable VALUES (" . join(",", "'control2-$n'", @VALUES) . ")";
  print "$INSERTQUERY\n";
  $dbh->do($INSERTQUERY);
}
$SELECTQUERY="SELECT * from nonuniquetable";
&outputselect($SELECTQUERY);

foreach $op ("=", "<>", "<", ">", "<=", ">=") {
  # INT
  $col="intnonunique";
  $val=25;
  $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
  &whereselect("$col $op $val");
  $val=250;
  $col="intnonuniquenotnull";
  $SElECTQUERY="SELECT keycol, $col from nonuniquetable";
  &whereselect("$col $op $val");
  # FLOAT
  $col="floatnonunique";
  $val=25.5;
  $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
  &whereselect("$col $op $val");
  $val=250.5;
  $col="floatnonuniquenotnull";
  $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
  &whereselect("$col $op $val");
  # CHAR
  $col="charnonunique";
  $val="\'" . chr(64+5) . "\'";
  $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
  &whereselect("$col $op $val");
  $val="\'" . chr(64+5+32) . "\'";
  $col="charnonuniquenotnull";
  $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
  &whereselect("$col $op $val");
  # CHARX
  $col="charxnonunique";
  $val="\'" . (chr(64+5)x2) . "\'";
  $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
  &whereselect("$col $op $val");
  $val="\'" . (chr(64+5+32)x2) . "\'";
  $col="charxnonuniquenotnull";
  $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
  &whereselect("$col $op $val");
  # VARCHAR
  $col="varcharnonunique";
  $val="\'" . (chr(64+5)x3) . "\'";
  $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
  &whereselect("$col $op $val");
  $val="\'" . (chr(64+5+32)x3) . "\'";
  $col="varcharnonuniquenotnull";
  $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
  &whereselect("$col $op $val");
  # BOOL
  if ($op eq "=" || $op eq "<>") {
    $col="boolnonunique";
    $val="true";
    $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
    &whereselect("$col $op $val");
    $col="boolnonuniquenotnull";
    $val="false";
    $SELECTQUERY="SELECT keycol, $col from nonuniquetable";
    &whereselect("$col $op $val");
  }
}

# update, replace testing
#foreach my $fieldtype ("int", "bool", "float", "char", "charx", "varchar") {
# comparisons with the same floating point value seem problematic
foreach my $fieldtype ("int", "bool", "char", "charx", "varchar") {
  foreach my $idxtype ("unique", "uniquenotnull", "nonunique", "nonuniquenotnull") {
    if ($fieldtype eq "bool" && ($idxtype eq "unique" || $idxtype eq
        "uniquenotnull")) {
      next;
    }
    if ($fieldtype eq "int") {
      $controlvalues="1, 1";
      $val1=50;
      $val2=60;
      $val3=70;
      $badval=55;
    } elsif ($fieldtype eq "bool") {
      $controlvalues="false, false";
      $val1="true";
      $val2="false";
      $val3="true";
      $badval="false";
    } elsif ($fieldtype eq "float") {
      $controlvalues="1.5, 1.5";
      $val1=50.5;
      $val2=60.5;
      $val3=70.5;
      $badval=55.5;
    } elsif ($fieldtype eq "char") {
      $controlvalues="\'A\', \'A\'";
      $val1="\'B\'";
      $val2="\'C\'";
      $val3="\'D\'";
      $badval="\'s\'";
    } elsif ($fieldtype eq "charx") {
      $controlvalues="\'AA\', \'AA\'";
      $val1="\'BB\'";
      $val2="\'CC\'";
      $val3="\'DD\'";
      $badval="\'ss\'";
    } elsif ($fieldtype eq "varchar") {
      $controlvalues="\'AAA\', \'AAA\'";
      $val1="\'BBB\'";
      $val2="\'CCC\'";
      $val3="\'DDD\'";
      $badval="\'sss\'";
    }

    $SELECTQUERY=sprintf("SELECT * FROM %s%stable", $fieldtype, $idxtype);
    &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
        $idxtype, $controlvalues));
    &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
        $idxtype, sprintf("%s, %s", $val1, $val1)));
    if ($idxtype eq "nonunique" || $idxtype eq "nonuniquenotnull") {
      &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
          $idxtype, sprintf("%s, %s", $val1, $val1)));
    }
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("DELETE FROM %s%stable WHERE %s%s1 = %s", $fieldtype,
        $idxtype, $fieldtype, $idxtype, $val1));
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
        $idxtype, sprintf("%s, %s", $val1, $val1)));
    if ($idxtype eq "nonunique" || $idxtype eq "nonuniquenotnull") {
      &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
          $idxtype, sprintf("%s, %s", $val1, $val1)));
    }
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("UPDATE %s%stable set %s%s2=%s WHERE %s%s2=%s",
        $fieldtype, $idxtype, $fieldtype, $idxtype, $val2, $fieldtype,
        $idxtype, $val1));
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("DELETE FROM %s%stable WHERE %s%s2 = %s", $fieldtype,
        $idxtype, $fieldtype, $idxtype, $val2));
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
        $idxtype, sprintf("%s, %s", $val1, $val1)));
    if ($idxtype eq "nonunique" || $idxtype eq "nonuniquenotnull") {
      &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
          $idxtype, sprintf("%s, %s", $val1, $val1)));
    }
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("UPDATE %s%stable set %s%s1=%s WHERE %s%s1=%s",
        $fieldtype, $idxtype, $fieldtype, $idxtype, $val2, $fieldtype,
        $idxtype, $val1));
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("DELETE FROM %s%stable WHERE %s%s1 = %s", $fieldtype,
        $idxtype, $fieldtype, $idxtype, $val2));
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
        $idxtype, sprintf("%s, %s", $val1, $val1)));
    if ($idxtype eq "nonunique" || $idxtype eq "nonuniquenotnull") {
      &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
          $idxtype, sprintf("%s, %s", $val1, $val1)));
    }
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("UPDATE %s%stable set %s%s2=%s WHERE %s%s2=%s",
        $fieldtype, $idxtype, $fieldtype, $idxtype, $val2, $fieldtype,
        $idxtype, $val1));
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("UPDATE %s%stable set %s%s1=%s WHERE %s%s2=%s",
        $fieldtype, $idxtype, $fieldtype, $idxtype, $val3, $fieldtype,
        $idxtype, $val2));
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("DELETE FROM %s%stable WHERE %s%s1 = %s", $fieldtype,
        $idxtype, $fieldtype, $idxtype, $val3));
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
        $idxtype, sprintf("%s, %s", $val1, $val1)));
    if ($idxtype eq "nonunique" || $idxtype eq "nonuniquenotnull") {
      &printanddo(sprintf("INSERT INTO %s%stable VALUES (%s)", $fieldtype,
          $idxtype, sprintf("%s, %s", $val1, $val1)));
    }
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("UPDATE %s%stable set %s%s1=%s WHERE %s%s1=%s",
        $fieldtype, $idxtype, $fieldtype, $idxtype, $val2, $fieldtype,
        $idxtype, $val1));
    &outputselect($SELECTQUERY);

    &printanddo(sprintf("UPDATE %s%stable set %s%s2=%s WHERE %s%s1=%s",
        $fieldtype, $idxtype, $fieldtype, $idxtype, $val3, $fieldtype,
        $idxtype, $val2));
    &outputselect($SELECTQUERY);

     &printanddo(sprintf("DELETE FROM %s%stable WHERE %s%s2 = %s", $fieldtype,
        $idxtype, $fieldtype, $idxtype, $val3));
    &outputselect($SELECTQUERY);
 }
}

print "stored procedure";
for ($n=1; $n <= 10; $n++) {
  $dbh->do("INSERT INTO accountstable VALUES (" . $n*10 . ", " . $n*200 . ")");
}
$SELECTQUERY="SELECT * FROM accountstable";
&outputselect($SELECTQUERY);
print "taking 35 from accountid 20 and adding it to accountid 90\n";
&outputselect("SELECT SqlProc (20, 90, 35)");
&outputselect($SELECTQUERY);

# end of main()

sub outputselect {
  my $rowsref=$dbh->selectall_arrayref($_[0]);
  my @rows=@$rowsref;
  print "rows returned: " . ($#rows+1) . "\n";
  foreach my $row (@$rowsref) {
    print join("\t", @{$row}) . "\n";
  }
}

sub whereselect {
  my $query="$SELECTQUERY WHERE ${_[0]}";
  print "$query\n";
  &outputselect($query);
}

# table, column, correct value, incorrectvalue
sub verifyupdate {
  my $table=$_[0];
  my $column=$_[1];
  my $originalval=$_[2];
  my $badval=$_[3];

  # valid search
  $SELECTQUERY="SELECT * FROM $table";
  &whereselect("$column = $originalval");
  # invalid search
  &whereselect("$column = $badval");
  # selectall
  &outputselect($SELECTQUERY);
}

sub printanddo {
  print "${_[0]}\n";
  $dbh->do($_[0]);
}

