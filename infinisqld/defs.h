/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL(tm).
 
 * InfiniSQL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InfiniSQL. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INFINISQLDEFS_H
#define INFINISQLDEFS_H

/**
 * @file   defs.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 12:47:30 2013
 * 
 * @brief  Global struct & enum types, global function declarations,
 * global symbols in general.
 */

// statuses builtins
#define BUILTIN_STATUS_OK 0
#define BUILTIN_STATUS_NOTOK 1

#define PAYLOADSIZE 128
#define NUMSOCKETS 1048576
/* following should match rtprio setting in limits.conf
 * anything >0 and <99 should be good
 * this is for prioritizing ObGateway & IbGateway threads
 */
#define RTPRIO 30
#define MSGRECEIVEBATCHSIZE 500
#define OBGWMSGBATCHSIZE 5000

#include "infinisql.h"

enum __attribute__ ((__packed__)) listenertype_e
{
    LISTENER_NONE = 0,
        LISTENER_RAW,
        LISTENER_PG
        };

// to builtins
enum builtincmds_e
{
    NOCMD = 0,
    STARTCMD,
    OKCMD,
    NOTOKCMD,
    USMRESPONSECMD,
    TASENGINESRESPONSECMD,
    ABORTCMD
};

enum __attribute__ ((__packed__)) indextype_e
{
    NONE = 0,
        UNIQUE = 1,
        NONUNIQUE = 2,
        UNORDERED = 3,
        UNIQUENOTNULL = 4,
        NONUNIQUENOTNULL = 5,
        UNORDEREDNOTNULL = 6
        };

enum indexmaptype_e
{
    noindexmaptype = -1,
    uniqueint = 0,
    nonuniqueint = 1,
    unorderedint = 2,
    uniqueuint = 3,
    nonuniqueuint = 4,
    unordereduint = 5,
    uniquebool = 6,
    nonuniquebool = 7,
    unorderedbool = 8,
    uniquefloat = 9,
    nonuniquefloat = 10,
    unorderedfloat = 11,
    uniquechar = 12,
    nonuniquechar = 13,
    unorderedchar = 14,
    uniquecharx = 15,
    nonuniquecharx = 16,
    unorderedcharx = 17,
    uniquevarchar = 18,
    nonuniquevarchar = 19,
    unorderedvarchar = 20
};

enum maptype_e
{
    Nomaptype = -1,
    Unique = 0,
    Nonunique = 1,
    Unordered = 2
};

enum __attribute__ ((__packed__)) topic_e
{
    TOPIC_NONE = 0,
        TOPIC_MBOXES = 1,
        TOPIC_SOCKET = 2,
        TOPIC_LOGIN = 3,
        TOPIC_LOGINOK = 4,
        TOPIC_LOGINFAIL = 5,
        TOPIC_CHANGEPASSWORD = 6,
        TOPIC_CHANGEPASSWORDOK = 7,
        TOPIC_CHANGEPASSWORDFAIL = 8,
        TOPIC_CREATEDOMAIN = 9,
        TOPIC_CREATEDOMAINOK = 10,
        TOPIC_CREATEDOMAINFAIL = 11,
        TOPIC_CREATEUSER = 12,
        TOPIC_CREATEUSEROK = 13,
        TOPIC_CREATEUSERFAIL = 14,
        TOPIC_DELETEUSER = 15,
        TOPIC_DELETEUSEROK = 16,
        TOPIC_DELETEUSERFAIL = 17,
        TOPIC_DELETEDOMAIN = 18,
        TOPIC_DELETEDOMAINOK = 19,
        TOPIC_DELETEDOMAINFAIL = 20,
        TOPIC_SCHEMAREQUEST = 21,
        TOPIC_SCHEMAREPLY = 22,
        TOPIC_DEADLOCKNEW = 23,
        TOPIC_DEADLOCKCHANGE = 24,
        TOPIC_DEADLOCKREMOVE = 25,
        TOPIC_LOADPROCEDURE = 26,
        TOPIC_TRANSACTION = 27,
        TOPIC_DISPATCH = 28,
        TOPIC_ACKDISPATCH = 29,
        TOPIC_DEADLOCKABORT = 30,
        TOPIC_ENDSUBTRANSACTION = 31,
        TOPIC_TOPOLOGY = 33,
        TOPIC_NEWPROCEDURE = 34,
        TOPIC_PROCEDURE1 = 35,
        TOPIC_PROCEDURE2 = 36,
        TOPIC_APPLY = 37,
        TOPIC_ACKAPPLY = 38,
        TOPIC_OPERATION = 41,
        TOPIC_COMPILE = 42,
        TOPIC_TABLENAME = 43,
        TOPIC_FIELDNAME = 44,
        TOPIC_SERIALIZED = 45,
        TOPIC_BATCHSERIALIZED = 46,
        TOPIC_SOCKETCONNECTED = 47
        };

enum __attribute__ ((__packed__)) payloadtype_e
{
    PAYLOADNONE = 0,
        PAYLOADMESSAGE,
        PAYLOADSOCKET,
        PAYLOADUSERSCHEMA,
        PAYLOADDEADLOCK,
        PAYLOADSUBTRANSACTION,
        PAYLOADCOMMITROLLBACK,
        PAYLOADDISPATCH,
        PAYLOADACKDISPATCH,
        PAYLOADAPPLY,
        PAYLOADACKAPPLY,
        PAYLOADSERIALIZED,
        PAYLOADBATCHSERIALIZED
        };

enum __attribute__ ((__packed__)) operationtype_e
{
    OPERATION_NONE = 0,
        OPERATION_LOGIN
        };

#define OPERAND_STRING  'a'
#define OPERAND_IDENTIFIER  'b'
#define OPERAND_PARAMETER   'c'
#define OPERAND_SUBQUERY    'd'
#define OPERAND_INTEGER 'e'
#define OPERAND_FLOAT    'f'
#define OPERAND_AGGREGATE   'g'
#define OPERAND_FIELDID 'h'
#define OPERAND_NULL 'i'
#define OPERAND_PREDICATERESULTS    'j'
#define OPERAND_SUBQUERYRESULTS 'k'
#define OPERAND_BOOLEAN 'l'

#define AGGREGATE_AVG   'a'
#define AGGREGATE_COUNT 'b'
#define AGGREGATE_MAX   'c'
#define AGGREGATE_MIN   'd'
#define AGGREGATE_SUM   'e'

// passing messages pertaining to specific builtins
#define BUILTINCREATESCHEMA 3
#define BUILTINCREATETABLE 4
#define BUILTINADDCOLUMN 5
#define BUILTINDELETEINDEX 6
#define BUILTINDELETETABLE 7
#define BUILTINDELETESCHEMA 8
#define BUILTINDUMPCONFIG 9

// states
#define ST_USM 1 // waiting for user schema manager
#define ST_TASENGINES 2 // waiting for tas and engines to reply

// operation type
#define OP_AUTH 1
#define OP_SCHEMA 2
#define OP_PGLOGIN 3

// namespaces & usings
using namespace std;
using namespace CryptoPP;
using namespace msgpack;
using std::string;
using std::vector;
using std::map;
using std::multimap;
using msgpack::sbuffer;
using boost::lexical_cast;

// enums
enum enginecmds_e
{
    enginecmdnewrow
};

enum __attribute__ ((__packed__)) pendingprimitive_e
{
    NOCOMMAND = 0,
        INSERT,
        UPDATE,
        DELETE,
        REPLACE,
        SELECT,
        FETCH,
        UNLOCK,
        COMMIT,
        ROLLBACK,
        PRIMITIVE_SQLPREDICATE,
        PRIMITIVE_SQLSELECTALL,
        PRIMITIVE_SQLSELECTALLFORDELETE,
        PRIMITIVE_SQLSELECTALLFORUPDATE,
        PRIMITIVE_SQLDELETE,
        PRIMITIVE_SQLINSERT,
        PRIMITIVE_SQLUPDATE,
        PRIMITIVE_SQLREPLACE
        };

enum enginecmd_e
{
    NOENGINECMD = 0,
    NEWROW,
    UNIQUEINDEX,
    UPDATEROW,
    DELETEROW,
    REPLACEDELETEROW,
    INDEXSEARCH,
    SELECTROWS,
    COMMITCMD,
    ROLLBACKCMD,
    REVERTCMD,
    UNLOCKCMD,
    SEARCHRETURN1
};

/** Global configs */
#define SERIALIZEDMAXSIZE   1048576
typedef struct
{
    int anonymousping;
    int badloginmessages;
    bool compressgw;
} cfg_s;
extern cfg_s cfgs;

extern FILE *logfile;
extern std::string zmqsocket;
extern void *zmqcontext;
extern std::string storedprocprefix;

typedef struct
{
    uint64_t instance;
    class Mbox *mbox;
    int epollfd;
} serventIdentity_s;

typedef struct __attribute__ ((__packed__))
{
    void *procedurecreator; //typedef ApiInterface*(*spclasscreate)();
    void *proceduredestroyer; //typedef void(*spclassdestroy)(ApiInterface*);
} procedures_s;

typedef struct
{
    boost::unordered_set<std::string> locked;
    boost::unordered_set<std::string> waiting;
} newDeadLockLists_s;

typedef struct
{
    bool isrow;
    int16_t tableid;
    int64_t rowid; // used both for row's rowid, and index value
    int16_t fieldid;
    int16_t engineid; // used with rowid for index value if
    bool deleteindexentry; // true=add, false=remove, for delete unique entry,
    // add/delete nonunique, add/delete null
    bool isnotaddunique; //simple flag for index entries that don't get prestaged
    bool isreplace;
    int64_t newrowid;
    int16_t newengineid;

    fieldValue_s fieldVal;
} rowOrField_s;

typedef struct
{
    operatortypes_e op;
    std::vector <fieldValue_s> values;
    std::string regexString;
} searchParams_s;

typedef struct __attribute__ ((__packed__)) 
{
    int64_t rowid;
    int16_t engineid;
} nonLockingIndexEntry_s;

typedef nonLockingIndexEntry_s indexEntry_s;

typedef struct
{
    int64_t status;
    bool isrow;
    int64_t rowid;
    int64_t tableid;
    std::string row;
    locktype_e locktype;
    int64_t forward_rowid;
    int64_t forward_engineid;
    int64_t fieldid;
    int64_t engineid; // index also uses rowid

    fieldValue_s fieldVal;
    std::vector<nonLockingIndexEntry_s> indexHits;
    searchParams_s searchParameters;
    std::vector<int64_t> rowids;
    std::vector<returnRow_s> returnRows;
} subtransactionCmd_s;

typedef struct
{
    std::string node;
    std::string service;
    int epollfd;
} listenerStruct_s;

typedef struct
{
    locktype_e locktype;
    bool isrow; // false, is index
    int64_t rowid;
    int64_t tableid;
    int64_t engineid;

    // index entries are field value -> rowid,subtransactionid}
    int64_t fieldid;
    fieldtype_e fieldType;
    fieldValue_s fieldVal;
} locked_s;

typedef struct
{
    locktype_e locktype;
    int64_t engineid;
    fieldValue_s fieldVal;
} lockFieldValue_s;

typedef struct
{
    pendingprimitive_e cmd;

    std::string originalRow;
    int64_t originalrowid;
    int64_t originalengineid;
    int64_t previoussubtransactionid;
    std::string newRow;
    int64_t newrowid;
    int64_t newengineid;
    locktype_e locktype;
    // uniqueIndices[fieldid] = lockFieldValue
    boost::unordered_map< int64_t, lockFieldValue_s > uniqueIndices;
} stagedRow_s;

typedef boost::unordered_map<int64_t, class Schema *> domainidToSchemaMap;

void debug(char *, int, char *);

void trimspace(string &);

// flags for row & unique index entry manipulation
#define DELETEFLAG 0
#define INSERTFLAG 1
#define LOCKEDFLAG 2
#define LOCKTYPEFLAG 3 // 0 is READLOCK 1 is WRITELOCK
#define REPLACEDELETEFLAG 4

enum deadlockchange_e
{
    ADDLOCKEDENTRY = 1,
    ADDLOCKPENDINGENTRY,
    REMOVELOCKEDENTRY,
    REMOVELOCKPENDINGENTRY,
    TRANSITIONPENDINGTOLOCKEDENTRY
};

class Topology;
extern class Topology nodeTopology;
extern pthread_mutex_t nodeTopologyMutex;
extern pthread_mutex_t connectionsMutex;
extern std::vector<class MboxProducer *> socketAffinity;
extern std::vector<listenertype_e> listenerTypes;

void msgpack2Vector(vector<string> *resultvector, char *payload,
                    int64_t length);
char setdeleteflag(char *c);
bool getdeleteflag(char c);
char cleardeleteflag(char *c);
char setinsertflag(char *c);
bool getinsertflag(char c);
char clearinsertflag(char *c);
char clearlockedflag(char *c);
locktype_e getlocktype(char c);
// return false if a problem
bool setwritelock(char *c);
bool setreadlock(char *c);
char setreplacedeleteflag(char c*);
bool getreplacedeleteflag(char c);
char clearreplacedeleteflag(char *c);
// end of flags stuff

int16_t getPartitionid(fieldValue_s &fieldVal, fieldtype_e type,
                       int16_t numpartitions);
void like2Regex(string &likeStr);
bool compareFields(fieldtype_e type, const fieldValue_s &val1,
                   const fieldValue_s &val2);
void stagedRow2ReturnRow(const stagedRow_s &stagedRow, returnRow_s &returnRow);
void setprio();

#endif // INFINISQLDEFS_H
