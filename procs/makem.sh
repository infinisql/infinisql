#!/bin/sh

PREFIX=$1

for src in *.cc
do
	make `echo $src|sed 's/\..*//'`.so PREFIX=$PREFIX
done
for so in *.so
do
	cp $so $PREFIX/procs/
done

