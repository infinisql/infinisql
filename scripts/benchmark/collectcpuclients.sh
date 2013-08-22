#!/bin/sh

for HOST in `cat clients`
do
  if `echo $HOST | grep -q '^#'`
  then
    continue
  fi
  echo $HOST
  ssh $HOST "cd perfstats; ./collect.sh pgbench"
done

