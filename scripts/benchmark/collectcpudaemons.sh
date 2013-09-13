#!/bin/sh

for HOST in `cat daemonhosts`
do
  if `echo $HOST | grep -q '^#'`
  then
    continue
  fi
  echo $HOST
  ssh $HOST "cd perfstats; ./collect.sh infinisqld"
done

