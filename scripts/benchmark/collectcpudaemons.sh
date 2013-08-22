#!/bin/sh

for HOST in `cut -f 1 daemons | sort | uniq`
do
  if `echo $HOST | grep -q '^#'`
  then
    continue
  fi
  echo $HOST
  ssh $HOST "cd perfstats; ./collect.sh infinisqld"
done

