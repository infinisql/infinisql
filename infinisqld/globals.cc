#include <sys/un.h>
#include "version.h"
#include "gch.h"
#include "Topology.h"

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

