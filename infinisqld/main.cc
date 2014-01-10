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

#include "globals.cc"

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


/** @mainpage InfiniSQL(tm)
 * This is the doxygen-generated documentation for InfiniSQL's C++ source code.
 * User documentation, reference, FAQ, how to obtain the source code, contacts
 * and other information is available at the project's main website:
 * http://www.infinisql.org
 */
