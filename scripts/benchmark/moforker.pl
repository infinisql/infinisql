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

# forks more benchmark clients
use POSIX;
use Getopt::Long;

GetOptions("cmd=s" => \@CMDS,
           "dir=s" => \$DIR);

if (fork() != 0) {
  exit(0);
}
POSIX::setsid();
close(STDIN);
close(STDOUT);
close(STDERR);

chdir($DIR);

# fork, 0=child, != 0, parent
for ($n=0; $n <= $#CMDS; $n++) {
  if (fork() == 0) {
    exec($CMDS[$n]);
    sleep(2);
  }
}

