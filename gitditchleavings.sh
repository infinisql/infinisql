#!/bin/sh

# run this before doing a pull request (unless you specifically want to
# modify one of these files). they are leavings from autotools

for FILE in INSTALL Makefile.in aclocal.m4 config.guess config.h.in config.sub configure depcomp infinisqld/Makefile.in install-sh ltmain.sh missing ylwrap
do
  git update-index --assume-unchanged $FILE
done

