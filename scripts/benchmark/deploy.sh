#!/bin/sh

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

if [ -z "$2" ]
then
  echo there must be 2 directories passed on the command line
  exit 1
fi

DEPLOYMENT_DIRECTORY1=$1
DEPLOYMENT_DIRECTORY2=$2

for HOST in `cat clients daemonhosts`
do
  if `echo $HOST | grep -q '^#'`
  then
    continue
  fi
  echo $HOST
  rsync -a --delete $DEPLOYMENT_DIRECTORY1/ $HOST:$DEPLOYMENT_DIRECTORY1/
  rsync -a --delete $DEPLOYMENT_DIRECTORY2/ $HOST:$DEPLOYMENT_DIRECTORY2/
done

