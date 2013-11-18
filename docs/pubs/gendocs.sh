#!/bin/sh

for file in [a-z]*.xml
do
  ./makexhtml.sh $file
done

