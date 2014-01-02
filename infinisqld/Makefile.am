sbin_PROGRAMS = infinisqld
infinisqld_SOURCES = Index.cc Operation.cc Table.cc main.cc Schema.cc TransactionAgent.cc Engine.cc Mbox.cc spooky.cc Transaction.cc Field.cc Message.cc SubTransaction.cc UserSchemaMgr.cc Topology.cc TopologyMgr.cc IbGateway.cc ObGateway.cc Applier.cc Pg.cc Listener.cc lexer.ll parser.yy Larxer.cc Asts.cc Actor.cc
infinisqld_LDADD = libinfinisql.la
lib_LTLIBRARIES = libinfinisql.la
libinfinisql_la_SOURCES = api.cc
include_HEADERS = infinisql.h
AM_CXXFLAGS = -ggdb -O3 -finline-functions -Wall -Wno-deprecated \
    -Wno-write-strings -std=c++11 -mcx16
AM_YFLAGS = -v -d
EXTRA_DIST = ../scripts/* ../procs/*
CLEANFILES = parser.cc parser.h parser.output lexer.cc cfgenum.h \
    ../infinisqlmgr/cfgenum.py ../infinisqlmgr/*.pyc ../procs/*.so
cfgenum.h: ../infinisqlmgr/gencfgenum.py
	../infinisqlmgr/gencfgenum.py
../infinisqlmgr/infinisql_cfgenum.py: ../infinisqlmgr/gencfgenum.py
	../infinisqlmgr/gencfgenum.py
BUILT_SOURCES = cfgenum.h parser.h
install-data-local:
	$(MKDIR_P) $(DESTDIR)$(prefix)/var $(DESTDIR)$(prefix)/run $(DESTDIR)$(prefix)/procs
parser.h: parser.yy
	../ylwrap parser.yy y.tab.c parser.cc y.tab.h parser.h y.output \
    parser.output -- bison -y -v -d
distclean-local:
	- rm -fr .deps
