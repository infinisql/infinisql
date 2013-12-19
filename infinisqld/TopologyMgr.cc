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
 * @file   TopologyMgr.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Dec 17 13:58:20 2013
 * 
 * @brief  Actor which receives configuration commands over 0mq and MessagePack
 * from infinisqlmgr.py.
 *
 * Launches all other actors, updates Topology, and distributes configuration
 * changes to all other actors on each node. Facilitates dynamic
 * reconfiguration.
 */

#include "TopologyMgr.h"
#line 35 "TopologyMgr.cc"

extern cfg_s cfgs;

void *userSchemaMgr(void *);
void *listener(void *);
void *transactionAgent(void *);
void *engine(void *);
void *deadlockMgr(void *);
void *ibGateway(void *);
void *obGateway(void *);

void replyToManager(void *, msgpack::sbuffer &);

TopologyMgr::TopologyMgr(Topology::partitionAddress *myIdentityArg) :
    myIdentity(*myIdentityArg)
{
    delete myIdentityArg;

    if (daemon(1, 1))
    {
        fprintf(logfile, "%s %i daemon errno %i\n", __FILE__, __LINE__, errno);
        exit(1);
    }

    // start 0mq socket
    //  void *zmqcontext = zmq_ctx_new();
    void *zmqresponder = zmq_socket(zmqcontext, ZMQ_REP);
    int rv = zmq_bind(zmqresponder, zmqsocket.c_str());

    if (rv == -1)
    {
        perror("zmq_bind");
        printf("%s %i zmq_bind errno %i\n", __FILE__, __LINE__, errno);
        exit(1);
    }

    int epollfd = epoll_create(1);

    while (1)
    {
    HECK:

        zmq_msg_t zmqrecvmsg;
        zmq_msg_init(&zmqrecvmsg);
        int retval = zmq_msg_recv(&zmqrecvmsg, zmqresponder, 0);

        if (retval == -1)
        {
            fprintf(logfile, "%s %i zmq_recv errno %i\n", __FILE__, __LINE__,
                    errno);
        }

        msgpack::sbuffer replysbuf;
        msgpack::packer<msgpack::sbuffer> replypk(&replysbuf);

        msgpack::sbuffer inbuf;
        inbuf.write((char *)zmq_msg_data(&zmqrecvmsg), zmq_msg_size(&zmqrecvmsg));
        msgpack::unpacker pac;
        pac.reserve_buffer(inbuf.size());
        memcpy(pac.buffer(), inbuf.data(), inbuf.size());
        pac.buffer_consumed(inbuf.size());
        msgpack::unpacked result;

        if (pac.next(&result)==false)
        {
            replypk.pack_int(CMDNOTOK);
            replyToManager(zmqresponder, replysbuf);
            zmq_msg_close(&zmqrecvmsg);
            goto HECK;
        }

        int cmd;
        msgpack::object obj = result.get();
        obj.convert(&cmd);

        switch (cmd)
        {
        case CMDSET:
        {
            if (pac.next(&result)==false)
            {
                replypk.pack_int(CMDNOTOK);
                replyToManager(zmqresponder, replysbuf);
                zmq_msg_close(&zmqrecvmsg);
                goto HECK;
            }

            int cmd2;
            msgpack::object obj2 = result.get();
            obj2.convert(&cmd2);

            switch (cmd2)
            {
            case CMDANONYMOUSPING:
            {
                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                int val;
                msgpack::object obj3 = result.get();
                obj3.convert(&val);

                // t or f
                __sync_bool_compare_and_swap(&cfgs.anonymousping,
                                             !(val==0 ? 0 : 1), val==0 ? 0 : 1);
                replypk.pack_int(CMDOK);
            }
            break;

            case CMDBADLOGINMESSAGES:
            {
                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                int val;
                msgpack::object obj3 = result.get();
                obj3.convert(&val);
                // t or f
                __sync_bool_compare_and_swap(&cfgs.badloginmessages,
                                             !(val==0 ? 0 : 1), val==0 ? 0 : 1);
                replypk.pack_int(CMDOK);
            }
            break;

            default:
                replypk.pack_int(CMDNOTOK);
                replyToManager(zmqresponder, replysbuf);
                zmq_msg_close(&zmqrecvmsg);
                goto HECK;
            }
        }
        break;

        case CMDGET:
        {
            if (pac.next(&result)==false)
            {
                replypk.pack_int(CMDNOTOK);
                replyToManager(zmqresponder, replysbuf);
                zmq_msg_close(&zmqrecvmsg);
                goto HECK;
            }

            int cmd2;
            msgpack::object obj2 = result.get();
            obj2.convert(&cmd2);

            switch (cmd2)
            {
            case CMDGETTOPOLOGYMGRMBOXPTR:
            {
                replypk.pack_int(CMDOK);
                replypk.pack_int64((int64_t)myIdentity.mbox);
            }
            break;

            default:
                replypk.pack_int(CMDNOTOK);
                replyToManager(zmqresponder, replysbuf);
                zmq_msg_close(&zmqrecvmsg);
                goto HECK;
            }
        }
        break;

        case CMDSTART:
        {
            pthread_t tid;

            if (pac.next(&result)==false)
            {
                replypk.pack_int(CMDNOTOK);
                replyToManager(zmqresponder, replysbuf);
                zmq_msg_close(&zmqrecvmsg);
                goto HECK;
            }

            int cmd2;
            msgpack::object obj2 = result.get();
            obj2.convert(&cmd2);

            if (pac.next(&result)==false)
            {
                replypk.pack_int(CMDNOTOK);
                replyToManager(zmqresponder, replysbuf);
                zmq_msg_close(&zmqrecvmsg);
                goto HECK;
            }

            int64_t actorid;
            msgpack::object obj3 = result.get();
            obj3.convert(&actorid);

            class Mbox *newmbox=NULL;

            switch (cmd2)
            {
            case CMDLISTENER:
            {
                vector<string> nodes;
                vector<string> services;
                string node;
                string service;
                msgpack::object obj4;

                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                obj4 = result.get();
                obj4.convert(&node);
                nodes.push_back(node);

                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                obj4 = result.get();
                obj4.convert(&service);
                services.push_back(service);

                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                obj4 = result.get();
                obj4.convert(&node);
                nodes.push_back(node);

                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                obj4 = result.get();
                obj4.convert(&service);
                services.push_back(service);

                newmbox = new class Mbox;

                if (pthread_create(&tid, NULL, listener,
                                   nodeTopology.newActor(ACTOR_LISTENER, newmbox,
                                                         epollfd, string(),
                                                         actorid, nodes,
                                                         services))==-1)
                {
                    fprintf(logfile, "%s %i pthread_create errno %i\n", __FILE__,
                            __LINE__, errno);
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }
                else
                {
                    replypk.pack_int(CMDOK);
                    replypk.pack_int64((int64_t)newmbox);
                }
            }
            break;
          
            case CMDUSERSCHEMAMGR:
            {
                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                string globaladminpassword;
                msgpack::object obj4 = result.get();
                obj4.convert(&globaladminpassword);
                newmbox = new class Mbox;

                if (pthread_create(&tid, NULL, userSchemaMgr,
                                   nodeTopology.newActor(ACTOR_USERSCHEMAMGR,
                                                         newmbox, epollfd,
                                                         globaladminpassword,
                                                         actorid,
                                                         vector<string>(),
                                                         vector<string>()))==-1)
                {
                    fprintf(logfile, "%s %i pthread_create errno %i\n", __FILE__,
                            __LINE__, errno);
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }
                else
                {
                    replypk.pack_int(CMDOK);
                    replypk.pack_int64((int64_t)newmbox);
                }
            }
            break;

            case CMDDEADLOCKMGR:
                newmbox = new class Mbox;

                if (pthread_create(&tid, NULL, deadlockMgr,
                                   nodeTopology.newActor(ACTOR_DEADLOCKMGR,
                                                         newmbox, epollfd,
                                                         string(), actorid,
                                                         vector<string>(),
                                                         vector<string>()))==-1)
                {
                    fprintf(logfile, "%s %i pthread_create errno %i\n", __FILE__,
                            __LINE__, errno);
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }
                else
                {
                    replypk.pack_int(CMDOK);
                    replypk.pack_int64((int64_t)newmbox);
                }

                break;

            case CMDTRANSACTIONAGENT:
            {
                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                int64_t instance;
                msgpack::object obj4 = result.get();
                obj4.convert(&instance);

                newmbox = new class Mbox;
                Topology::partitionAddress *paddr =
                    nodeTopology.newActor(ACTOR_TRANSACTIONAGENT,
                                          newmbox, epollfd, string(), actorid,
                                          vector<string>(),
                                          vector<string>());
                paddr->instance = instance;

                if (pthread_create(&tid, NULL, transactionAgent, paddr)==-1)
                {
                    fprintf(logfile, "%s %i pthread_create errno %i\n", __FILE__,
                            __LINE__, errno);
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }
                else
                {
                    replypk.pack_int(CMDOK);
                    replypk.pack_int64((int64_t)newmbox);
                }
            }
            break;

            case CMDENGINE:
            {
                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                int64_t instance;
                msgpack::object obj4 = result.get();
                obj4.convert(&instance);

                newmbox = new class Mbox;
                Topology::partitionAddress *paddr =
                    nodeTopology.newActor(ACTOR_ENGINE,
                                          newmbox, epollfd, string(), actorid,
                                          vector<string>(),
                                          vector<string>());
                paddr->instance = instance;

                if (pthread_create(&tid, NULL, engine, paddr)==-1)
                {
                    fprintf(logfile, "%s %i pthread_create errno %i\n", __FILE__,
                            __LINE__, errno);
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }
                else
                {
                    replypk.pack_int(CMDOK);
                    replypk.pack_int64((int64_t)newmbox);
                }
            }
            break;

            case CMDOBGATEWAY:
            {
                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                int64_t instance;
                msgpack::object obj4 = result.get();
                obj4.convert(&instance);

                newmbox = new class Mbox;
                Topology::partitionAddress *paddr =
                    nodeTopology.newActor(ACTOR_OBGATEWAY,
                                          newmbox, epollfd, string(), actorid,
                                          vector<string>(), vector<string>());
                paddr->instance = instance;

                if (pthread_create(&tid, NULL, obGateway, paddr)==-1)
                {
                    fprintf(logfile, "%s %i pthread_create errno %i\n", __FILE__,
                            __LINE__, errno);
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }
                else
                {
                    replypk.pack_int(CMDOK);
                    replypk.pack_int64((int64_t)newmbox);
                }
            }
            break;

            case CMDIBGATEWAY:
            {
                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                int64_t instance;
                msgpack::object obj4 = result.get();
                obj4.convert(&instance);

                if (pac.next(&result)==false)
                {
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }

                string hostport;
                msgpack::object obj5 = result.get();
                obj5.convert(&hostport);

                newmbox = new class Mbox;
                Topology::partitionAddress *paddr =
                    nodeTopology.newActor(ACTOR_IBGATEWAY,
                                          newmbox, epollfd, hostport, actorid,
                                          vector<string>(), vector<string>());
                paddr->instance = instance;

                if (pthread_create(&tid, NULL, ibGateway, paddr)==-1)
                {
                    fprintf(logfile, "%s %i pthread_create errno %i\n", __FILE__,
                            __LINE__, errno);
                    replypk.pack_int(CMDNOTOK);
                    replyToManager(zmqresponder, replysbuf);
                    zmq_msg_close(&zmqrecvmsg);
                    goto HECK;
                }
                else
                {
                    replypk.pack_int(CMDOK);
                    replypk.pack_int64((int64_t)newmbox);
                }
            }
            break;

            default:
                replypk.pack_int(CMDNOTOK);
                replyToManager(zmqresponder, replysbuf);
                zmq_msg_close(&zmqrecvmsg);
                goto HECK;
            }
        }
        break;

        case CMDLOCALCONFIG:
        {
            updateLocalConfig(pac, result);
            replypk.pack_int(CMDOK);
        }
        break;

        case CMDGLOBALCONFIG:
        {
            updateGlobalConfig(pac, result);
            replypk.pack_int(CMDOK);
        }
        break;

        default:
            replypk.pack_int(CMDNOTOK);
            replyToManager(zmqresponder, replysbuf);
            zmq_msg_close(&zmqrecvmsg);
            goto HECK;
        }

        replyToManager(zmqresponder, replysbuf);

        if (zmq_msg_close(&zmqrecvmsg)==-1)
        {
            perror("zmq_msg_close");
            printf("%s %i %i\n", __FILE__, __LINE__, errno);
        }
    }
}

TopologyMgr::TopologyMgr(const TopologyMgr &orig)
{
}

TopologyMgr::~TopologyMgr()
{
}

/** Launcher function for TopologyMgr actor */
void *topologyMgr(void *identity)
{
    new TopologyMgr((Topology::partitionAddress *)identity);
    while (1)
    {
        sleep(1000000);
    }
    return NULL;
}

void replyToManager(void *zmqsocket, msgpack::sbuffer &sbuf)
{
    zmq_msg_t zmqmsg;
    zmq_msg_init_size(&zmqmsg, sbuf.size());
    memcpy(zmq_msg_data(&zmqmsg), sbuf.data(), sbuf.size());
    zmq_msg_send(&zmqmsg, zmqsocket, 0);
    zmq_msg_close(&zmqmsg);
}

void TopologyMgr::updateLocalConfig(msgpack::unpacker &pac,
                                    msgpack::unpacked &result)
{
    pac.next(&result);
    msgpack::object obj = result.get();
    vector<int64_t> t;
    obj.convert(&t);

    vector<int64_t> i;
    pac.next(&result);
    obj = result.get();
    obj.convert(&i);

    vector<int64_t> m;
    pac.next(&result);
    obj = result.get();
    obj.convert(&m);

    vector<Topology::actor_s> v(t.size());

    for (size_t n=0; n < t.size(); n++)
    {
        Topology::actor_s a = { (actortypes_e)t[n], i[n], (class Mbox *)m[n] };
        v[n] = a;
    }

    pthread_mutex_lock(&nodeTopologyMutex);
    nodeTopology.actorList.swap(v);
    nodeTopology.numtransactionagents = 0;
    nodeTopology.numengines = 0;

    Topology::partitionAddress addr = {};
    addr.address.nodeid = nodeTopology.nodeid;
    addr.epollfd = -1;
    addr.argstring = "";
    addr.address.actorid = 4;
    addr.mbox = (class Mbox *)nodeTopology.actorList[4].mbox;
    addr.type = ACTOR_LISTENER;

    //  if (nodeTopology.actorList[]
    nodeTopology.numtransactionagents = 0;
    nodeTopology.numengines = 0;
    nodeTopology.numobgateways = 0;

    for (size_t n=0; n < nodeTopology.actorList.size(); n++)
    {
        if (nodeTopology.actorList[n].type==ACTOR_TRANSACTIONAGENT)
        {
            nodeTopology.numtransactionagents++;
            addr.address.actorid = n;
            addr.mbox = (class Mbox *)nodeTopology.actorList[n].mbox;
            addr.type = nodeTopology.actorList[n].type;
            addr.instance = nodeTopology.actorList[n].instance;
        }
        else if (nodeTopology.actorList[n].type==ACTOR_ENGINE)
        {
            nodeTopology.numengines++;
            addr.address.actorid = n;
            addr.mbox = (class Mbox *)nodeTopology.actorList[n].mbox;
            addr.type = nodeTopology.actorList[n].type;
            addr.instance = nodeTopology.actorList[n].instance;
        }
        else if (nodeTopology.actorList[n].type==ACTOR_IBGATEWAY)
        {
            addr.address.actorid = n;
            addr.mbox = (class Mbox *)nodeTopology.actorList[n].mbox;
            addr.type = nodeTopology.actorList[n].type;
            addr.instance = nodeTopology.actorList[n].instance;
        }
        else if (nodeTopology.actorList[n].type==ACTOR_OBGATEWAY)
        {
            nodeTopology.numobgateways++;
            addr.address.actorid = n;
            addr.mbox = (class Mbox *)nodeTopology.actorList[n].mbox;
            addr.type = nodeTopology.actorList[n].type;
            addr.instance = nodeTopology.actorList[n].instance;
        }
    }

    pthread_mutex_unlock(&nodeTopologyMutex);

    broadcastConfig();
}

void TopologyMgr::updateGlobalConfig(msgpack::unpacker &pac,
                                     msgpack::unpacked &result)
{
    pac.next(&result);
    msgpack::object obj = result.get();
    int activereplica;
    obj.convert(&activereplica);

    pac.next(&result);
    obj = result.get();
    vector<int64_t> replicaids;
    obj.convert(&replicaids);

    pac.next(&result);
    obj = result.get();
    vector<int64_t> nodeids;
    obj.convert(&nodeids);

    pac.next(&result);
    obj = result.get();
    vector<int64_t> actorids;
    obj.convert(&actorids);

    pac.next(&result);
    obj = result.get();
    vector<int64_t> ibgatewaynodes;
    obj.convert(&ibgatewaynodes);

    pac.next(&result);
    obj = result.get();
    vector<int64_t> ibgatewayinstances;
    obj.convert(&ibgatewayinstances);

    pac.next(&result);
    obj = result.get();
    vector<string> ibgatewayhostports;
    obj.convert(&ibgatewayhostports);

    pac.next(&result);
    obj = result.get();
    int64_t dmgrnode;
    obj.convert(&dmgrnode);

    pac.next(&result);
    obj = result.get();
    int64_t dmgrmboxptr;
    obj.convert(&dmgrmboxptr);

    pac.next(&result);
    obj = result.get();
    int64_t usmgrnode;
    obj.convert(&usmgrnode);

    pac.next(&result);
    obj = result.get();
    int64_t usmgrmboxptr;
    obj.convert(&usmgrmboxptr);

    pac.next(&result);
    obj = result.get();
    size_t numreplicas;
    obj.convert(&numreplicas);

    vector< vector<int16_t> > replicaMembers;

    for (size_t n=0; n < numreplicas; n++)
    {
        pac.next(&result);
        obj = result.get();
        vector<int16_t> v;
        obj.convert(&v);
        replicaMembers.push_back(v);
    }

    pac.next(&result);
    obj = result.get();
    size_t lentas;
    obj.convert(&lentas);

    vector< vector<int16_t> > tas;
    tas.push_back(vector<int16_t>());

    for (size_t n=1; n <= lentas; n++)
    {
        pac.next(&result);
        obj = result.get();
        vector<int16_t> v;
        obj.convert(&v);
        tas.push_back(v);
    }

    map< int64_t, vector<int> > allActorsMap;
    vector< vector<int> > allActors;

    while (pac.next(&result))
    {
        obj = result.get();
        int64_t nid;
        obj.convert(&nid);
        pac.next(&result);
        obj = result.get();
        vector<int> aids;
        obj.convert(&aids);
        allActorsMap[nid] = aids;
    }

    allActors.resize(allActorsMap.rbegin()->first + 1, vector<int>());
    map< int64_t, vector<int> >::iterator it;

    for (it = allActorsMap.begin(); it != allActorsMap.end(); ++it)
    {
        allActors[it->first] = it->second;
    }

    boost::unordered_map< int16_t, vector<int> > allActorsThisReplica;
    boost::unordered_set<int64_t> nodesThisReplica;
    int64_t myreplica = -1;

    for (size_t n=0; n < replicaMembers.size(); n++)
    {
        for (size_t m=0; m < replicaMembers[n].size(); m++)
        {
            if (replicaMembers[n][m]==myTopology.nodeid)
            {
                myreplica = n;
            }
        }
    }

    if (myreplica >= 0)
    {
        for (size_t n=0; n < replicaMembers[myreplica].size(); n++)
        {
            nodesThisReplica.insert(replicaMembers[myreplica][n]);
        }

        for (size_t n=0; n < allActors.size(); n++)
        {
            if (nodesThisReplica.count(n))
            {
                allActorsThisReplica[n] = allActors[n];
            }
        }
    }

    size_t numpartitions=0;

    for (size_t n=0; n < replicaids.size(); n++)
    {
        if (replicaids[n]==0)
        {
            numpartitions++;
        }
    }

    vector< vector<Topology::partitionAddress> > pl(numreplicas);
    /*
      Topology::partitionAddress pa = {};
      vector<Topology::partitionAddress> pl2(numpartitions, pa);
      vector<Topology::partitionAddress> pl2;
      for (size_t n=0; n < numreplicas; n++)
      {
      pl[n] = pl2;
      }
    */
    pthread_mutex_lock(&nodeTopologyMutex);

    nodeTopology.activereplica = activereplica;

    for (size_t n=0; n < replicaids.size(); n++)
    {
        Topology::partitionAddress addr;
        addr.address.nodeid = nodeids[n];
        addr.address.actorid = actorids[n];
        addr.epollfd = -1;
        addr.argstring = "";
        addr.instance = -1;

        if (nodeTopology.nodeid==addr.address.nodeid)
        {
            addr.mbox = nodeTopology.actorList[addr.address.actorid].mbox;
        }
        else
        {
            addr.mbox = NULL;
        }

        addr.type = ACTOR_ENGINE;
        pl[replicaids[n]].push_back(addr);
    }

    vector<Topology::partitionAddress> pltr;

    if (myreplica >= 0)
    {
        pltr = pl[myreplica];
    }

    nodeTopology.partitionList.swap(pl);
    nodeTopology.partitionListThisReplica.swap(pltr);

    map< int64_t, vector<string> > ibgws;

    for (size_t n=0; n < ibgatewaynodes.size(); n++)
    {
        int64_t node = ibgatewaynodes[n];
        int64_t instance = ibgatewayinstances[n];
        string hostport = ibgatewayhostports[n];

        if (ibgws[node].size() < (size_t)(instance+1))
        {
            ibgws[node].resize(instance+1, string());
            ibgws[node][instance] = hostport;
        }
    }

    nodeTopology.ibGateways.swap(ibgws);

    nodeTopology.userSchemaMgrNode = usmgrnode;
    nodeTopology.userSchemaMgrMbox = (class Mbox *)usmgrmboxptr;
    nodeTopology.deadlockMgrNode = dmgrnode;
    nodeTopology.deadlockMgrMbox = (class Mbox *)dmgrmboxptr;
    //  nodeTopology.numpartitions = nodeTopology.partitionListThisReplica.size();
    nodeTopology.numpartitions = numpartitions;
    nodeTopology.replicaMembers.swap(replicaMembers);
    nodeTopology.tas.swap(tas);
    nodeTopology.allActors.swap(allActors);
    nodeTopology.allActorsThisReplica.swap(allActorsThisReplica);
    nodeTopology.numreplicas = numreplicas;
    pthread_mutex_unlock(&nodeTopologyMutex);

    broadcastConfig();
}

void TopologyMgr::broadcastConfig()
{
    mboxes.update(myTopology);
    // next, go through each and send mails
    class Message *msg;

    for (size_t n=0; n < myTopology.actorList.size(); n++)
    {
        if (myTopology.actorList[n].type != ACTOR_NONE)
        {
            msg = new class Message;
            msg->messageStruct.topic = TOPIC_TOPOLOGY;
            msg->messageStruct.payloadtype = PAYLOADMESSAGE;
            msg->messageStruct.sourceAddr.nodeid = myTopology.nodeid;
            msg->messageStruct.sourceAddr.actorid = 1;
            msg->messageStruct.destAddr.nodeid = myTopology.nodeid;
            msg->messageStruct.destAddr.actorid = n;
            mboxes.actoridToProducers[n]->sendMsg(*msg);
        }
    }
}
