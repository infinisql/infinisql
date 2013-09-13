#!/bin/sh

TESTSEC=$1

for HOST in `cat daemonhosts`
do
  if `echo $HOST | grep -q '^#'`
  then
    continue
  fi
  LOGDIR=/home/mtravis/daemonlogs/${TESTSEC}_${HOST}
  echo $LOGDIR
  mkdir $LOGDIR
  rsync -a $HOST:/home/mtravis/infinisql_built/var/ $LOGDIR
  ssh $HOST "rm /home/mtravis/infinisql_built/var/*"
done

