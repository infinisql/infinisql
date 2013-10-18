#!/bin/sh

TESTSEC=$1

for HOST in `cat daemonhosts`
do
  if `echo $HOST | grep -q '^#'`
  then
    continue
  fi
  LOGDIR=/home/infinisql/daemonlogs/${TESTSEC}_${HOST}
  echo $LOGDIR
  mkdir $LOGDIR
  rsync -a $HOST:/home/infinisql/infinisql_built/var/ $LOGDIR
  ssh $HOST "rm /home/infinisql/infinisql_built/var/*"
done

