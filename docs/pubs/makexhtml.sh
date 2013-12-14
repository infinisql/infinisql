#!/bin/bash

# get docbook schama, put in ~/DocBook
# To use with Emacs nXml mode, visit .xml file, then (in X-Window):
# XML->Set Schema->File
# then browse to ~/DocBook/docbookxi.rnc


filename=$(basename $1)
filename="${filename%.*}"
xsltproc --xinclude -o ~/tmp/html/$filename.html ~/DocBook/docbook-xsl-ns-1.78.1/xhtml/docbook.xsl $1

