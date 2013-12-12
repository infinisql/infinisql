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

# ssh into a list of hosts, and call "moforker.pl" with a list of commands
# to execute

require '../infinisql.plib';

$CLIENTS="$SCRIPTDIR/benchmark/clients";
$DAEMONS="$SCRIPTDIR/benchmark/daemons";
$MOFORKERDIR="$SCRIPTDIR/benchmark";
$PGBENCH="nohup /usr/local/bin/pgbench -s 1 -c 200 -j 1 -n -U benchmark -I -P benchmark --per-second=100 --urandom";

use Getopt::Long;
# for prepare test: rows equal in accounts, tellers, branches, in 10000 row
# batches
GetOptions("test=s" => \$TEST,
           "accountids=i" => \$ACCOUNTIDS,
           "duration=i" => \$DURATION,
           "nclients=i" => \$NCLIENTS,
           "bmresultsdir=s" => \$BMRESULTSDIR);

open(CLIENTS, $CLIENTS);
while (<CLIENTS>) {
  next if /^#/;
  chomp($_);
  push(@CLIENTS, $_);
}
close(CLIENTS);
$numclients=$#CLIENTS+1;

open(DAEMONS, $DAEMONS);
while (<DAEMONS>) {
  next if /^#/;
  chomp($_);
  ($server, $port)=split(/\t/, $_);
  push(@SERVERS, $server);
  push(@PORTS, $port);
}
$numdaemons=$#SERVERS+1;

foreach $client (@CLIENTS) {
  $CMDS{$client}=" --dir=$MOFORKERDIR ";
}

$nowsec=time();

# a different fill.pl cmd per batch
if ($TEST eq "preparepgbench") {

  $batchsize=10000;
  if ($ACCOUNTIDS % $batchsize || !$ACCOUNTIDS) {
    die "accountids must be in multiples of $batchsize";
  }

  my $batches=$ACCOUNTIDS/$batchsize;
  for (my $n=1; $n <= $batches; $n++) {
    my $client=$CLIENTS[($n-1) % $numclients];
    my $server=$SERVERS[($n-1) % $numdaemons];
    my $port=$PORTS[($n-1) % $numdaemons];
    $CMDS{$client} .= "--cmd=\\\"./fill.pl -h $server -p $port -s $n -b 1\\\" ";
  }

} elsif ($TEST eq "preparekeyval") {

  $batchsize=10000;
  if ($ACCOUNTIDS % $batchsize || !$ACCOUNTIDS) {
    die "accountids must be in multiples of $batchsize";
  }

  my $batches=$ACCOUNTIDS/$batchsize;
  for (my $n=1; $n <= $batches; $n++) {
    my $client=$CLIENTS[($n-1) % $numclients];
    my $server=$SERVERS[($n-1) % $numdaemons];
    my $port=$PORTS[($n-1) % $numdaemons];
    $CMDS{$client} .= "--cmd=\\\"./fillkeyval.pl -h $server -p $port -s $n -b 1\\\" ";
  }

} elsif ($TEST eq "procedure") {
  &pgbenchcmd("procedure");
} elsif ($TEST eq "procedurenoinsert") {
  &pgbenchcmd("procedurenoinsert");
} elsif ($TEST eq "multistatements") {
  &pgbenchcmd("multistatements");
} elsif ($TEST eq "setkey") {
  &pgbenchcmd("setkey");
} elsif ($TEST eq "getkey") {
  &pgbenchcmd("getkey");
} else {
  die "no such test: $TEST";
}

foreach $client (@CLIENTS) {
  my $cmdline="ssh $client \"$MOFORKERDIR/moforker.pl ${CMDS{$client}}\"";
  print "$cmdline\n";
  system($cmdline);
}

$nowstr=localtime($nowsec);
print "\n$nowstr: $nowsec\n";

# end of main()

sub pgbenchcmd {
  my $testtype=$_[0];
  die "duration must be a positive integer" if $DURATION<=0;
  die "bmresultsdir must be defined" if !length($BMRESULTSDIR);

  for (my $n=0; $n < $NCLIENTS; $n++) {
    my $client=$CLIENTS[$n % $numclients];
    my $server=$SERVERS[$n % $numdaemons];
    my $port=$PORTS[$n % $numdaemons];
    $CMDS{$client} .= "--cmd=\\\"$PGBENCH -f $testtype.pgb -h $server -p $port -T $DURATION >$BMRESULTSDIR/${testtype}_${nowsec}_$n.out 2>$BMRESULTSDIR/${testtype}_${nowsec}_$n.err\\\" ";
  }
}

