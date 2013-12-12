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

require '../infinisql.plib';
$CLIENTS="$SCRIPTDIR/benchmark/clients";

use Getopt::Std;
getopt('ts');
$TESTNAME=$opt_t;
$SECOND=$opt_s;

$secstring=localtime($SECOND);

open(CLIENTS, $CLIENTS);
while (<CLIENTS>) {
  next if /^#/;
  chomp($_);
  push(@CLIENTS, $_);
}
close(CLIENTS);
$numclienthosts=$#CLIENTS+1;

$lockerrors=0;
foreach $clienthost (@CLIENTS) {
  $cmd=sprintf("ssh %s cat bmresults/%s_%s_*.err|", $clienthost, $TESTNAME,
      $SECOND);
  open(ERRS, $cmd);
  while(<ERRS>) {
    $lockerrors++ if /lock not available$/;
  }
  close(ERRS);

  my @OUTFILES;
  $cmd=sprintf("ssh %s ls bmresults/%s_%s_*.out|", $clienthost, $TESTNAME,
      $SECOND);
  open(GETOUTFILES, $cmd);
  while(<GETOUTFILES>) {
    chomp($_);
    push(@OUTFILES, $_);
  }
  close(GETOUTFILES);
  $numtests+=$#OUTFILES+1;

  foreach $outfile (@OUTFILES) {
    $cmd=sprintf("ssh %s tac %s|", $clienthost, $outfile);
    open(READOUTFILE, $cmd);
    my $firstline=1;
    while(<READOUTFILE>) {
      if ($firstline) {
        if ($_ !~ /^[0-9]/) {
          printf("WARNING: outfile %s not finished, is pgbench still running?\n", $outfile);
          break;
        }
        $firstline=0;
      }
      chomp($_);
      if (/^[0-9]/) {
        ($sec, @rates)=split(/,/, $_);
        foreach $threadrate (@rates) {
          $RATE{$sec}+=$threadrate;
        }
      } elsif (/^number of clients: ([0-9]+)/) {
        $numclients+=$1;
      }
    }
    close(READOUTFILE);
  }
}

printf("test %i: %s\n", $SECOND, $secstring);
printf("lockerrors %i numtests %i numclienthosts %i numclients %i\n", $lockerrors, $numtests, $numclienthosts, $numclients);

# get rid of first and last seconds
@ORDEREDSECONDS=sort { $RATE{$a} <=> $RATE{$b} } keys(%RATE);
delete $RATE{$ORDEREDSECONDS[0]};
delete $RATE{$ORDEREDSECONDS[$#ORDEREDSECONDS]};

my @PERSECOND;
my $ttl=0;
foreach $sec (keys(%RATE)) {
#  printf("%i: %i\n", $sec, $RATE{$sec});
  push(@PERSECOND, $RATE{$sec});
  $ttl+=$RATE{$sec};
}
my @SORTEDPERSECOND=sort {$a <=> $b} @PERSECOND;
my $seconds=$#PERSECOND+1;
$min=$SORTEDPERSECOND[0];
$max=$SORTEDPERSECOND[$#SORTEDPERSECOND];
my $median;
if ($seconds % 2 == 1) {
  $median=$SORTEDPERSECOND[$#SORTEDPERSECOND/2];
} else {
  my $higherval=$SORTEDPERSECOND[$seconds/2];
  my $lowerval=$SORTEDPERSECOND[$seconds/2-1];
  $median=($higherval+$lowerval)/2;
}
$avg=$ttl/$seconds;
$errorrate=$lockerrors/$ttl;

printf("seconds %i min %i max %i median %.0f average %.0f errorrate %.2f%\n", $seconds, $min, $max, $median, $avg, $errorrate*100);

foreach $sec (sort {$a <=> $b} keys(%RATE)) {
  printf("%i: %i\n", $sec, $RATE{$sec});
}

