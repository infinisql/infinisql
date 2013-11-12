#!/bin/bash

# should have something such that for each file (unless chunked) it does
# aS FOLLOWS:
# xsltproc -o <infinisql.github.io/file.html /Users/mtravis/DocBook/docbook-xsl-ns-1.78.1/xhtml/docbook.xsl infile.xml

filename=$(basename $1)
filename="${filename%.*}"
xsltproc --xinclude -o ./html/$filename.html /Users/mtravis/DocBook/docbook-xsl-ns-1.78.1/xhtml/docbook.xsl $1

