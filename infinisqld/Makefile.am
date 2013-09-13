sbin_PROGRAMS = infinisqld
infinisqld_SOURCES = Index.cc Operation.cc Table.cc DeadlockMgr.cc main.cc Schema.cc TransactionAgent.cc Engine.cc Mbox.cc spooky.cc Transaction.cc Field.cc Message.cc SubTransaction.cc UserSchemaMgr.cc Topology.cc TopologyMgr.cc IbGateway.cc ObGateway.cc Applier.cc Pg.cc Listener.cc lexer.ll parser.yy Larxer.cc Asts.cc
infinisqld_LDADD = libinfinisql.la /usr/local/lib/libmsgpack.a
lib_LTLIBRARIES = libinfinisql.la
libinfinisql_la_SOURCES = api.cc
include_HEADERS = infinisql_api.h infinisql_Message.h infinisql_Operation.h infinisql_DeadlockMgr.h infinisql_Schema.h infinisql_defs.h infinisql_spooky.h infinisql_Engine.h infinisql_SubTransaction.h infinisql_Field.h infinisql_Table.h infinisql_gch.h infinisql_Index.h infinisql_TransactionAgent.h infinisql_Transaction.h infinisql_UserSchemaMgr.h infinisql_Mbox.h infinisql_Topology.h infinisql_TopologyMgr.h infinisql_cfgenum.h infinisql_IbGateway.h infinisql_ObGateway.h infinisql_Applier.h infinisql_Pg.h infinisql_Listener.h infinisql_Larxer.h infinisql_Asts.h larx.h parser.h
AM_CXXFLAGS = -ggdb -O3 -finline-functions -Wall -Wno-deprecated \
    -Wno-write-strings -std=c++11 -mcx16
AM_YFLAGS = -v -d
EXTRA_DIST = ../scripts/* ../procs/*
CLEANFILES = parser.cc parser.h parser.output lexer.cc infinisql_cfgenum.h \
    ../infinisqlmgr/cfgenum.py ../infinisqlmgr/*.pyc ../procs/*.so
infinisql_cfgenum.h: ../infinisqlmgr/gencfgenum.py
	../infinisqlmgr/gencfgenum.py
../infinisqlmgr/infinisql_cfgenum.py: ../infinisqlmgr/gencfgenum.py
	../infinisqlmgr/gencfgenum.py
BUILT_SOURCES = infinisql_cfgenum.h parser.h
install-data-local:
	$(MKDIR_P) $(prefix)/var $(prefix)/run $(prefix)/procs
parser.h: parser.yy
	../ylwrap parser.yy y.tab.c parser.cc y.tab.h parser.h y.output \
    parser.output -- bison -y -v -d
distclean-local:
	- rm -fr .deps
