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
 * @file   TopologyManager.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Wed Jan 22 09:17:20 2014
 * 
 * @brief  Actor which receives configuration commands over 0mq and MessagePack
 * from infinisqlmgr.py.
 * 
 * Launches all other actors, updates Topology, and distributes configuration
 * changes to all other actors on each node. Facilitates dynamic
 * reconfiguration.
 */

#include "TopologyManager.h"

/* this isn't that important since the AdminListener doesn't use Mbox, but
 * all actors have an id and this is the one for AdminListener
 */
#define ADMINLISTENERACTORID 17

TopologyManager::TopologyManager(Actor::identity_s identity)
    : Actor(identity)
{
    nodeTopologyVersion=1;
}

void TopologyManager::operator()()
{
    identity.mbox=new (std::nothrow) Mbox;
    if (identity.mbox==NULL)
    {
        LOG("can't create Mbox");
        exit(1);
    }
    startAdminListener(ADMINLISTENERACTORID, identity.zmqhostport);

    // this should be a Mbox event loop
    while(1)
    {
        sleep(10);
    }
}

int TopologyManager::startSocket(std::string &node, std::string &service,
    bool isibgw)
{
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *servinfo;
    char *nodeptr=nullptr;

    if (node.size())
    {
        nodeptr = (char *)node.c_str();
    }

    int rv=getaddrinfo((const char *)nodeptr, service.c_str(), &hints,
                       &servinfo);
    if (rv)
    {
        LOG("getaddrinfo error " << rv << ": " << gai_strerror(rv));
        return 0;
    }
    int sockfd=0;
    int yes=1;
    struct addrinfo *p=nullptr;

    for (p = servinfo; p != nullptr; p = p->ai_next)
    {
        sockfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (sockfd==-1)
        {
            LOG("socket problem");
            continue;
        }
        fcntl(sockfd, F_SETFL, O_NONBLOCK);
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))==-1)
        {
            LOG("setsockopt problem");
            continue;
        }
        if (isibgw)
        {
            int so_rcvbuf=IBGWRCVBUF;
            socklen_t optlen=sizeof(so_rcvbuf);
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf,
                           optlen)==-1)
            {
                LOG("setsockopt");
                continue;
            }
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen)==-1)
        {
            close(sockfd);
            LOG("bind problem");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);

    if (p==nullptr)
    {
        LOG("failed to bind");
        return 0;
    }
    if (listen(sockfd, 1000) == -1)
    {
        LOG("listen problem");
        return 0;
    }

    return sockfd;
}

void TopologyManager::openPartition(MDB_env *env, size_t mapsize,
                                    unsigned int readers, MDB_dbi dbs,
                                    std::string &path)
{
    int retval=mdb_env_create(&env);
    if (retval)
    {
        env=nullptr;
        LOG("mdb_env_create() problem " << retval);
        return;
    }
    retval=mdb_env_set_mapsize(env, mapsize);
    if (retval)
    {
        mdb_env_close(env);
        env=nullptr;
        LOG("mdb_env_set_mapsize() problem " << retval);
        return;
    }
    retval=mdb_env_set_maxreaders(env, readers);
    if (retval)
    {
        mdb_env_close(env);
        env=nullptr;
        LOG("mdb_env_set_maxreaders() problem " << retval);
        return;
    }
    retval=mdb_env_set_maxdbs(env, dbs);
    if (retval)
    {
        mdb_env_close(env);
        env=nullptr;
        LOG("mdb_env_set_maxdbs() problem " << retval);
        return;
    }
    retval=mdb_env_open(env, path.c_str(), MDB_WRITEMAP|MDB_NOTLS,
                        S_IRUSR|S_IWUSR);
    if (retval)
    {
        mdb_env_close(env);
        env=nullptr;
        LOG("mdb_env_open() problem " << retval);
        return;
    }
}

void TopologyManager::setIdentity(int16_t actorid, int16_t instance,
    identity_s &newidentity)
{
    newidentity={};
    newidentity.address={identity.address.nodeid, actorid};
    newidentity.instance=instance;
    newidentity.mbox=new (std::nothrow) Mbox;
    if (newidentity.mbox==nullptr)
    {
        LOG("can't create Mbox");
        return;
    }    
}

bool TopologyManager::startTransactionAgent(int16_t actorid, int16_t instance)
{
    identity_s newidentity;
    setIdentity(actorid, instance, newidentity);
    if (newidentity.mbox==nullptr)
    {
        return false;
    }
    if (startThread(TransactionAgent(newidentity))==true)
    {
        return true;
    }
    delete newidentity.mbox;
    return false;
}

bool TopologyManager::startPartitionWriter(int16_t actorid, int16_t instance,
                              size_t mapsize, unsigned int readers,
                              MDB_dbi dbs, std::string &path)
{
    identity_s newidentity;
    setIdentity(actorid, instance, newidentity);
    if (newidentity.mbox==nullptr)
    {
        return false;
    }
    openPartition(newidentity.env, mapsize, readers, dbs, path);
    if (newidentity.env==nullptr)
    {
        delete newidentity.mbox;
        newidentity.mbox=nullptr;
        return false;
    }
    if (startThread(PartitionWriter(newidentity))==true)
    {
        return true;
    }
    delete newidentity.mbox;
    return false;
}

bool TopologyManager::startTransactionLogger(int16_t actorid, int16_t instance,
                                             std::string &path)
{
    identity_s newidentity;
    setIdentity(actorid, instance, newidentity);
    newidentity.transactionlogpath=path;
    if (newidentity.mbox==nullptr)
    {
        return false;
    }
    if (startThread(TransactionLogger(newidentity))==true)
    {
        return true;
    }
    delete newidentity.mbox;
    return false;
}


bool TopologyManager::startUserSchemaManager(int16_t actorid,
                                             std::string &path)
{
    identity_s newidentity;
    setIdentity(actorid, 0, newidentity);
    if (newidentity.mbox==nullptr)
    {
        return false;
    }
    // 100MB ought to be plenty for USM LMDB environment
    openPartition(newidentity.env, 104857600, 126, 1024, path);
    if (newidentity.env==nullptr)
    {
        delete newidentity.mbox;
        newidentity.mbox=nullptr;
        return false;
    }
    if (startThread(UserSchemaManager(newidentity))==true)
    {
        return true;
    }
    delete newidentity.mbox;
    return false;
}

bool TopologyManager::startObGateway(int16_t actorid, int16_t instance)
{
    identity_s newidentity;
    setIdentity(actorid, instance, newidentity);
    if (newidentity.mbox==nullptr)
    {
        return false;
    }
    if (startThread(ObGateway(newidentity))==true)
    {
        return true;
    }
    delete newidentity.mbox;
    return false;
}

bool TopologyManager::startIbGateway(int16_t actorid, int16_t instance,
                                     std::string &node, std::string &service)
{
    identity_s newidentity;
    setIdentity(actorid, instance, newidentity);
    if (newidentity.mbox==nullptr)
    {
        return false;
    }
    newidentity.epollfd=epoll_create(1);
    if (newidentity.epollfd==-1)
    {
        LOG("epoll_create() problem");
        delete newidentity.mbox;
        return false;
    }
    newidentity.sockfd=startSocket(node, service, true);
    if (startThread(IbGateway(newidentity))==true)
    {
        return true;
    }
    delete newidentity.mbox;
    return false;
}

bool TopologyManager::startListener(int16_t actorid, std::string &node,
                                    std::string &service)
{
    identity_s newidentity;
    setIdentity(actorid, 0, newidentity);
    if (newidentity.mbox==nullptr)
    {
        return false;
    }
    newidentity.epollfd=epoll_create(1);
    if (newidentity.epollfd==-1)
    {
        LOG("epoll_create() problem");
        delete newidentity.mbox;
        return false;
    }
    newidentity.sockfd=startSocket(node, service, false);
    if (startThread(Listener(newidentity))==true)
    {
        return true;
    }
    delete newidentity.mbox;
    return false;
}

bool TopologyManager::startAdminListener(int16_t actorid,
                                         std::string &zmqhostport)
{
    identity_s newidentity;
    setIdentity(actorid, 0, newidentity);
    if (newidentity.mbox==nullptr)
    {
        return false;
    }
    newidentity.zmqhostport=zmqhostport;
    if (startThread(AdminListener(newidentity))==true)
    {
        return true;
    }
    delete newidentity.mbox;
    return false;
}
