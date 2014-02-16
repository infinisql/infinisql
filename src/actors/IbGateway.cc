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
 * @file   TransactionAgent.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Sat Jan 25 09:00:22 2014
 * 
 * @brief  Inbound Gateway actor. Counterpart to ObGateway. Receives messages
 * over the network from remote senders and distributes them to their
 * destination actors on the current node.
 */

#include "IbGateway.h"

#define EPOLLEVENTS 1024

IbGateway::IbGateway(Actor::identity_s identity)
    : Actor(identity), inbuf(nullptr), dcstrsmall(nullptr)
{
}

IbGateway::~IbGateway()
{
    delete[] inbuf;
    delete[] dcstrsmall;
}

void IbGateway::operator()()
{
    struct epoll_event ev={};
    ev.events=EPOLLIN | EPOLLHUP | EPOLLET;
    ev.data.fd=identity.sockfd;
    if (epoll_ctl(identity.epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev)==-1)
    {
        LOG("epoll_ctl problem");
        return;
    }

    struct sockaddr_in their_addr={}; // connector's address information
    socklen_t sin_size=sizeof(their_addr);
    struct epoll_event events[EPOLLEVENTS];
    int so_rcvbuf=GWBUFSIZE;
    inbuf=new (std::nothrow) char[so_rcvbuf];
    if (inbuf==nullptr)
    {
        LOG("allocate for inbuf failed size " << so_rcvbuf);
        return; // should probably exit node
    }
    dcstrsmall=new (std::nothrow) char[GWBUFSIZE];
    if (dcstrsmall==nullptr)
    {
        LOG("can't allocate " << GWBUFSIZE);
        return; // should probably exit node
    }
    
    while(1)
    {
        int eventcount=epoll_wait(identity.epollfd, events, EPOLLEVENTS, 10);
        myTopology.update();
        if (eventcount==-1)
        {
            if (errno != EINTR)
            {
                LOG("epoll_wait problem");
            }
            continue;
        }
        
        for (int n=0; n < eventcount; ++n)
        {
            int fd=events[n].data.fd;
            int event=events[n].events;

            if (fd==identity.sockfd)
            {
                if (event & EPOLLERR || event & EPOLLHUP)
                {
                    continue;
                }
                while(1)
                {
                    int newfd=accept(fd, (struct sockaddr *)&their_addr,
                                     &sin_size);
                    if (newfd==-1)
                    {
                        if (errno != EAGAIN && errno != EWOULDBLOCK)
                        {
                            LOG("accept error");
                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }

                    fcntl(newfd, F_SETFL, O_NONBLOCK);
                    setsockopt(newfd, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf,
                               sizeof(so_rcvbuf));
                    ev.data.fd=newfd;
                    epoll_ctl(identity.epollfd, EPOLL_CTL_ADD, newfd, &ev);
                    setprio();
                }
            }
            else
            { // existing connection
                if ((event & EPOLLERR) || (event & EPOLLHUP))
                {
                    close(fd);
                    epoll_ctl(identity.epollfd, EPOLL_CTL_DEL, fd, nullptr);
                    continue;
                }
                // must be EPOLLIN, so read()
                ssize_t readed;

                do
                {
                    readed=read(fd, inbuf, so_rcvbuf);
                    switch (readed)
                    {
                    case -1:
                        if (errno==EAGAIN || errno==EWOULDBLOCK)
                        {
                            if (pendingReads.count(fd))
                            {
                                string &strRef=pendingReads[fd];
                                size_t pos=0;
                                while (pos < strRef.size())
                                {
                                    if (sizeof(size_t)>(size_t)(strRef.size()-
                                                                pos))
                                    { // can't even read size of message group
                                        break;
                                    }
                                    size_t packagesize=
                                        *(size_t *)(strRef.c_str()+pos);
                                    if (packagesize > strRef.size()-pos)
                                    { // can't read next message group entirely
                                        break;
                                    }
                                    inbufhandler(strRef.c_str()+pos,
                                                 packagesize);
                                    pos += packagesize;
                                }
                                if (pos<strRef.size())
                                { // background the remainder
                                    string newstr(strRef, pos, string::npos);
                                    strRef.swap(newstr);
                                }
                                else
                                {
                                    strRef.clear();
                                }
                            }
                        }
                        else
                        {
                            close(fd);
                            epoll_ctl(identity.epollfd, EPOLL_CTL_DEL, fd,
                                      nullptr);
                            break;
                        }
                        break;
              
                    case 0:
                        close(fd);
                        epoll_ctl(identity.epollfd, EPOLL_CTL_DEL, fd,
                                  nullptr);
                        break;
              
                    default:
                    {
                        if (pendingReads.count(fd))
                        {
                            pendingReads[fd].append(inbuf, readed);
                        }
                        else
                        {
                            size_t pos=0;
                            while (pos < (size_t)readed)
                            {
                                if (sizeof(size_t)>(size_t)(readed-pos))
                                { // can't even read size of message group
                                    break;
                                }
                                size_t packagesize=*(size_t *)(inbuf+pos);
                                if (packagesize > readed-pos)
                                { // can't read next message group entirely
                                    break;
                                }
                                inbufhandler(inbuf+pos, packagesize);
                                pos += packagesize;
                            }
                            if (pos<(size_t)readed)
                            { // background the remainder
                                pendingReads[fd].assign(inbuf+pos, readed-pos);
                            }
                        }
                    }
                    }
                }
                while (readed > 0);
            }
        }
    }
}

void IbGateway::inbufhandler(const char *buf, size_t bufsize)
{
    char *inbuffer;
    size_t inbuffersize;
    char *dcstr;
    char *dcstrbig;
    bool isdcstrbig=false;
    char dcstrsmall[GWBUFSIZE];
  
    int bs=GWBUFSIZE;
    dcstr=dcstrsmall;
    while(1)
    {
        ssize_t dcsize=LZ4_decompress_safe(buf+sizeof(bufsize), dcstr,
                                           bufsize-sizeof(bufsize), bs);
        if (dcsize < 0)
        {
            if (isdcstrbig==true)
            {
                delete dcstrbig;
            }
            else
            {
                isdcstrbig=true;
            }
            bs *= 2;
            dcstrbig=new (std::nothrow) char[bs];
            dcstr=dcstrbig;
            continue;
        }
        inbuffer=dcstr;
        inbuffersize=dcsize;
        break;
    }
  
    size_t pos=0;
    while (pos<inbuffersize)
    {
        size_t s=*(size_t *)(inbuffer+pos);
        pos += sizeof(s)+s;
        MessageSerialized *msgsnd=new MessageSerialized(Serdes(inbuffer+pos,
                                                               s));
        myTopology.actoridToMboxes[msgsnd->message.destinationAddress.actorid]->sendMsg(*msgsnd);
    }

    if (isdcstrbig==true)
    {
        delete[] dcstrbig;
    }
}
