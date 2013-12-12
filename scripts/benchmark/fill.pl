#! /usr/bin/env perl

# Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
# All rights reserved. No warranty, explicit or implicit, provided.
#
# This file is part of InfiniSQL(tm).
 
# InfiniSQL is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3
# as published by the Free Software Foundation.
#
# InfiniSQL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with InfiniSQL. If not, see <http://www.gnu.org/licenses/>.

# http://www.postgresql.org/docs/9.2/static/pgbench.html
# insert in batches of 100,000. same number of bids, tids & aids

use DBI;
use Getopt::Std;
require '../infinisql.plib';

$opt_p="15432";
getopt('hpsb');
$HOSTNAME=$opt_h;
$PORT=$opt_p;
$STARTBATCH=$opt_s;
$BATCHES=$opt_b;

$dbh=DBI->connect("dbi:Pg:dbname=benchmark;host=$HOSTNAME;port=$PORT;sslmode=disable;", "benchmark", "benchmark", {pg_server_prepare => 0});

$startpoint=($STARTBATCH-1)*10000 + 1;
$endpoint=$startpoint-1 + $BATCHES*10000;

#$dbh->do("BEGIN");
for (my $aid=$startpoint; $aid <= $endpoint; $aid++) {
  $QUERY="INSERT INTO pgbench_branches VALUES ($aid, 0, '')";
  $dbh->do($QUERY);
  $QUERY="INSERT INTO pgbench_tellers VALUES ($aid, 1, 0, '')";
  $dbh->do($QUERY);
  $QUERY="INSERT INTO pgbench_accounts VALUES ($aid, 1, 0, '')";
  $dbh->do($QUERY);
}
#$dbh->do("COMMIT");

