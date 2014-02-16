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
 * @file   ObGateway.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Sat Jan 25 09:10:25 2014
 * 
 * @brief  Outbound Gateway actor. Counterpart to IbGateway. Receives messages
 * from actors on current node bound for remote nodes. Sends them over the
 * network to IbGateway.
 */

#include "ObGateway.h"

ObGateway::ObGateway(Actor::identity_s identity)
    : Actor(identity), so_sndbuf(GWBUFSIZE), serstrsmall(nullptr),
      cstrsmall(nullptr)
{
}

void ObGateway::operator()()
{
    int waitfor=100;
    std::unordered_map< int16_t, vector<Serdes *> > pendingMsgs;
    char *serstrbig=nullptr;
    char *serstr=nullptr;
    bool isserstrbig=false;
    serstrsmall=new (std::nothrow) char[GWBUFSIZE];
    if (serstrsmall==nullptr)
    {
        LOG("can't allocate for serstrsmall " << GWBUFSIZE);
        return; // probably should die here
    }
    char *cstrbig=nullptr;
    char *cstr=nullptr;
    bool iscstrbig=false;
    char *sendstr=nullptr;
    cstrsmall=new (std::nothrow) char[GWBUFSIZE];
    if (cstrsmall==nullptr)
    {
        LOG("can't allocate for cstrsmall " << GWBUFSIZE);
        return; // probably should die here
    }
    size_t sendsize=0;

    while(1)
    {
        if (myTopology.update()==true)
        {
            updateRemoteGateways();
        }
        for (size_t inmsg=0; inmsg < 5000; ++inmsg)
        {
            msgrcv=identity.mbox->receive(waitfor);
            if (msgrcv==nullptr)
            {
                waitfor=100;
                break;
            }
            waitfor=0;

            switch(msgrcv->message.topic)
            {
            case Message::TOPIC_BATCH:
            {
                MessageBatch &msgref=*(MessageBatch *)msgrcv;
                for (int16_t n=0; n < msgref.nmsgs; ++n)
                {
                    MessageBatch::messagebatch_s &messagebatchref=
                        msgref.messagebatch[n];
                    pendingMsgs[messagebatchref.nodeid].push_back(messagebatchref.serializedMessage);
                }
            }
            break;

            default:
                LOG("bad topic " << msgrcv->message.topic);
            }
        }

        // send batches
        std::unordered_map< int16_t, vector<Serdes *> >::iterator it;
        for (it=pendingMsgs.begin(); it != pendingMsgs.end(); ++it)
        {
            vector<Serdes *> &msgsRef=it->second;
            size_t s=0;
            for (size_t n=0; n < msgsRef.size(); n++)
            {
                s += sizeof(size_t) + msgsRef[n]->val.mv_size;
            }
            if (!s)
            {
                continue;
            }
            // format is: total size, [msgsize, msg]...
            if (s>GWBUFSIZE)
            {
                serstrbig=new (std::nothrow) char[s];
                serstr=serstrbig;
                isserstrbig=true;
            }
            else
            {
                serstr=serstrsmall;
                isserstrbig=false;
            }
            size_t pos=0;
            for (size_t n=0; n < msgsRef.size(); n++)
            {
                Serdes &msgRef=*msgsRef[n];
                size_t ms=msgRef.val.mv_size;
                memcpy(serstr+pos, &ms, sizeof(ms));
                pos += sizeof(size_t);
                memcpy(serstr+pos, msgRef.val.mv_data, ms);
                pos += ms;
                delete &msgRef;
            }
      
            // compress
            int cbound=LZ4_compressBound(s);
            if (cbound+sizeof(size_t) > GWBUFSIZE)
            {
                cstrbig=new (std::nothrow) char[cbound+sizeof(size_t)];
                if (cstrbig==nullptr)
                {
                    LOG("can't allocate for cstrbig " << cbound+sizeof(size_t));
                    continue;
                    // probably should die here
                }
                cstr=cstrbig;
                iscstrbig=true;
            }
            else
            {
                cstr=cstrsmall;
                iscstrbig=false;
            }
            size_t csize=LZ4_compress(serstr, cstr+sizeof(csize), s);
            csize += sizeof(csize);
            memcpy(cstr, &csize, sizeof(csize));
            sendstr=cstr;
            sendsize=csize;

            if (!nodeidToSocket[it->first])
            {
                if (myTopology.update()==true)
                {
                    updateRemoteGateways();
                }                
            }
            if (send(nodeidToSocket[it->first], sendstr, sendsize, 0)==-1)
            {
                LOG("send() problem");
            }

            if (isserstrbig==true)
            {
                delete[] serstrbig;
                serstrbig=nullptr;
            }
            if (iscstrbig==true)
            {
                delete[] cstrbig;
                cstrbig=nullptr;
            }
        }
        pendingMsgs.clear();
    }
}

void ObGateway::updateRemoteGateways()
{
    if (myTopology.nodeidToIbGateway.empty())
    {
        return;
    }
    if (nodeidToSocket.size()+1 <
        (size_t)myTopology.nodeidToIbGateway.rbegin()->first)
    {
        nodeidToSocket.resize(myTopology.nodeidToIbGateway.rbegin()->first+1, 0);
        setprio();
    }
    for (size_t n=1; n <= nodeidToSocket.size(); ++n)
    {
        if (nodeidToSocket[n]) // already connected
        {
            continue;
        }
        if ((size_t)identity.instance >= myTopology.nodeidToIbGateway.size())
        {
            continue;
        }

        string &node=myTopology.nodeidToIbGateway[n].first;
        string &service=myTopology.nodeidToIbGateway[n].second;
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family=AF_INET;
        hints.ai_socktype=SOCK_STREAM;
        hints.ai_flags=AI_PASSIVE;
        struct addrinfo *servinfo;
        if (getaddrinfo(node.c_str(), service.c_str(), &hints,
                        &servinfo))
        {
            LOG("getaddrinfo problem for " << node << ":" << service);
            continue;
        }
        int newfd=socket(servinfo->ai_family, servinfo->ai_socktype,
                            servinfo->ai_protocol);
        if (newfd==-1)
        {
            LOG("socket problem for " << node << ":" << service);
            continue;
        }
        if (connect(newfd, servinfo->ai_addr, servinfo->ai_addrlen))
        {
            LOG("connect problem for " << node << ":" << service);
            continue;
        }
        if (setsockopt(newfd, SOL_SOCKET, SO_SNDBUF, &so_sndbuf,
                       sizeof(so_sndbuf))==-1)
        {
            LOG("setsockopt problem for " << node << ":" << service);
            continue;
        }
        freeaddrinfo(servinfo);

        nodeidToSocket[n]=newfd;
    }
}

ObGateway::~ObGateway()
{
    delete[] serstrsmall;
}
