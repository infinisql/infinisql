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

$dbh=DBI->connect("dbi:Pg:dbname=benchmark;host=$HOSTNAME;port=$PORT;", "benchmark", "benchmark", {pg_server_prepare => 0});

$startpoint=($STARTBATCH-1)*10000 + 1;
$endpoint=$startpoint-1 + $BATCHES*10000;

for (my $aid=$startpoint; $aid <= $endpoint; $aid++) {
  $QUERY="INSERT INTO keyvaltable VALUES ($aid, 'default value for this this is the default value for this is the default value value default the is this')";
  $dbh->do($QUERY);
}

