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

/**
 * @file   main.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:30:10 2013
 * 
 * @brief  Contains main and various global function definitions.
 */

#include <sys/un.h>
#include "version.h"
#include "gch.h"
#include "Topology.h"
#line 33 "main.cc"

void *topologyMgr(void *);

FILE *logfile;
cfg_s cfgs;
std::string zmqsocket;
class Topology nodeTopology;
pthread_mutex_t nodeTopologyMutex;
pthread_mutex_t connectionsMutex;
void *zmqcontext;
std::string storedprocprefix = "InfiniSQL_";
std::vector<class MboxProducer *> socketAffinity;
std::vector<listenertype_e> listenerTypes;

int main(int argc, char **argv)
{
    zmqcontext = zmq_ctx_new();
    zmq_ctx_set(zmqcontext, ZMQ_IO_THREADS, 2);
    setlinebuf(stdout);

    string logfilename;
    int c;

    while ((c = getopt(argc, argv, "l:m:n:hv")) != -1)
    {
        switch (c)
        {
        case 'm':
            zmqsocket.assign("tcp://");
            zmqsocket.append(optarg, strlen(optarg));
            break;

        case 'l':
            logfilename.assign(optarg, strlen(optarg));
            break;

        case 'n':
            nodeTopology.nodeid = atol(optarg);
            break;

        case 'h':
            printf("-m <management ip:port> -n <nodeid> -l <log path/file> -v\n");
            exit(0);
            break;

        case 'v':
            printf("%s\n", version);
            exit(0);

        default:
            ;
        }
    }

    if (!logfilename.size())
    {
        logfilename.assign("/tmp/infinisqld.log");
    }

    logfile = fopen(logfilename.c_str(), "a");

    if (logfile==NULL)
    {
        printf("%s %i cannot open logfile %s errno %i\n", __FILE__, __LINE__,
               logfilename.c_str(), errno);
        exit(1);
    }

    setlinebuf(logfile);

    pthread_mutexattr_t attr;
    attr.__align = PTHREAD_MUTEX_ADAPTIVE_NP;
    pthread_mutex_init(&nodeTopologyMutex, &attr);
    pthread_mutex_init(&connectionsMutex, &attr);
    pthread_t topologyMgrThread;
    Topology::actorIdentity *arg = new Topology::actorIdentity();
    arg->type = ACTOR_TOPOLOGYMGR;
    arg->mbox = new class Mbox;
    arg->address.nodeid = 1;
    arg->address.actorid = 1;
    arg->instance = -1;
  
    cfgs.compressgw=true;

    int rv=pthread_create(&topologyMgrThread, NULL, topologyMgr, arg);
    if (rv)
    {
        printf("%s %i pthread_create rv %i\n", __FILE__, __LINE__, rv);
        exit(1);
    }

    while (1)
    {
        sleep(10);
    }

    return 0;
}

// global functions
void msgpack2Vector(vector<string> *resultvector, char *payload, int64_t length)
{
    msgpack::unpacked msg;
    msgpack::unpack(&msg, payload, length);
    msgpack::object obj = msg.get();
    obj.convert(resultvector);
}

void debug(char *description, int line, char *file)
{
    fprintf(logfile, "DEBUG %i %s %s\n", line, file, description);
}

char setdeleteflag(char *c)
{
    return *c |= 1 << DELETEFLAG;
}

bool getdeleteflag(char c)
{
    return c & 1 << DELETEFLAG;
}

char cleardeleteflag(char *c)
{
    return *c &= ~(1 << DELETEFLAG);
}

char setinsertflag(char *c)
{
    return *c |= 1 << INSERTFLAG;
}

bool getinsertflag(char c)
{
    return c & 1 << INSERTFLAG;
}

char clearinsertflag(char *c)
{
    return *c &= ~(1 << INSERTFLAG);
}

bool setwritelock(char *c)
{
    if (*c & 1 << LOCKEDFLAG)   // already locked
    {
        return false;
    }

    *c |= 1 << LOCKEDFLAG;
    *c |= 1 << LOCKTYPEFLAG;
    return true;
}

bool setreadlock(char *c)
{
    if (*c & 1 << LOCKEDFLAG)   // already locked
    {
        return false;
    }

    *c |= 1 << LOCKEDFLAG;
    *c &= ~(1 << LOCKTYPEFLAG);
    return true;
}

char clearlockedflag(char *c)
{
    return *c &= ~(1 << LOCKEDFLAG);
}

locktype_e getlocktype(char c)
{
    if (!(c & 1 << LOCKEDFLAG))
    {
        return NOLOCK;
    }

    if (c & 1 << LOCKTYPEFLAG)
    {
        return WRITELOCK;
    }
    else
    {
        return READLOCK;
    }
}

char clearreplacedeleteflag(char *c)
{
    return *c &= ~(1 << REPLACEDELETEFLAG);
}

bool getreplacedeleteflag(char c)
{
    return c & 1 << REPLACEDELETEFLAG;
}

char setreplacedeleteflag(char *c)
{
    return *c |= 1 << REPLACEDELETEFLAG;
}

int16_t getPartitionid(fieldValue_s &fieldVal, fieldtype_e type,
                       int16_t numpartitions)
{
    switch (type)
    {
    case INT:
        return SpookyHash::Hash64((void *) &fieldVal.value.integer,
                                  sizeof(fieldVal.value.integer), 0) %
                                  numpartitions;
        break;

    case UINT:
        return SpookyHash::Hash64((void *) &fieldVal.value.uinteger,
                                  sizeof(fieldVal.value.uinteger), 0) %
            numpartitions;
        break;

    case BOOL:
        return SpookyHash::Hash64((void *) &fieldVal.value.boolean,
                                  sizeof(fieldVal.value.boolean), 0) %
            numpartitions;
        break;

    case FLOAT:
        return SpookyHash::Hash64((void *) &fieldVal.value.floating,
                                  sizeof(fieldVal.value.floating), 0) %
            numpartitions;
        break;

    case CHAR:
        return SpookyHash::Hash64((void *) &fieldVal.value.character,
                                  sizeof(fieldVal.value.character), 0) %
            numpartitions;
        break;

    case CHARX:
        return SpookyHash::Hash64((void *) fieldVal.str.c_str(),
                                  fieldVal.str.length(), 0) % numpartitions;
        break;

    case VARCHAR:
        return SpookyHash::Hash64((void *) fieldVal.str.c_str(),
                                  fieldVal.str.length(), 0) % numpartitions;
        break;

    default:
        printf("%s %i anomaly fieldtype %i\n", __FILE__, __LINE__, type);
        return -1;
    }
}

// no escape chars specified as yet
void like2Regex(string &likeStr)
{
    size_t pos;

    while ((pos = likeStr.find('_', 0)) != string::npos)
    {
        likeStr[pos]='.';
    }

    while ((pos = likeStr.find('%', 0)) != string::npos)
    {
        likeStr[pos]='*';
        likeStr.insert(pos, 1, '.');
    }
}

bool compareFields(fieldtype_e type, const fieldValue_s &val1,
                   const fieldValue_s &val2)
{
    if (val1.isnull==true && val2.isnull==true)
    {
        return true;
    }
    else if (val1.isnull==true || val2.isnull==true)
    {
        return false;
    }

    switch (type)
    {
    case INT:
        if (val1.value.integer==val2.value.integer)
        {
            return true;
        }

        break;

    case UINT:
        if (val1.value.uinteger==val2.value.uinteger)
        {
            return true;
        }

        break;

    case BOOL:
        if (val1.value.boolean==val2.value.boolean)
        {
            return true;
        }

        break;

    case FLOAT:
        if (val1.value.floating==val2.value.floating)
        {
            return true;
        }

        break;

    case CHAR:
        if (val1.value.character==val2.value.character)
        {
            return true;
        }

        break;

    case CHARX:
        if (val1.str.compare(val2.str)==0)
        {
            return true;
        }

        break;

    case VARCHAR:
        if (val1.str.compare(val2.str)==0)
        {
            return true;
        }

        break;

    default:
        printf("%s %i anomaly %i\n", __FILE__, __LINE__, type);
    }

    return false;
}

void trimspace(string &input)
{
    size_t last=input.find_last_not_of(' ');

    if (last != string::npos)
    {
        input.erase(last+1);
    }
    else
    {
        input.clear();
    }
}

void stagedRow2ReturnRow(const stagedRow_s &stagedRow, returnRow_s &returnRow)
{
    returnRow.previoussubtransactionid=stagedRow.previoussubtransactionid;
    returnRow.locktype=stagedRow.locktype;

    switch (stagedRow.cmd)
    {
    case NOCOMMAND:
        returnRow.rowid=stagedRow.originalrowid;
        returnRow.row=stagedRow.originalRow;
        break;

    case INSERT:
        returnRow.rowid=stagedRow.newrowid;
        returnRow.row=stagedRow.newRow;
        break;

    case UPDATE:
        returnRow.rowid=stagedRow.newrowid;
        returnRow.row=stagedRow.newRow;
        break;

    default:
        printf("%s %i anomaly %i\n", __FILE__, __LINE__, stagedRow.cmd);
        returnRow=returnRow_s();
    }
}

void setprio()
{
    struct sched_param params;
    params.sched_priority=RTPRIO;
    int rv=pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);
    if (rv != 0)
    {
        fprintf(logfile, "%s %i some problem setting priority %i for tid %li error %i\n", __FILE__, __LINE__, RTPRIO, pthread_self(), rv);
    }
}

/** @mainpage InfiniSQL(tm)
 * This is the doxygen-generated documentation for InfiniSQL's C++ source code.
 * User documentation, reference, FAQ, how to obtain the source code, contacts
 * and other information is available at the project's main website:
 * http://www.infinisql.org
 */
