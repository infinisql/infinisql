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
 * @file   Listener.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Sat Jan 25 09:13:24 2014
 * 
 * @brief  On each host, this actor accepts new connections and distributes
 * incoming network traffic from clients to Transaction Agents.
 *
 * There is only one listener per host. It takes a small amount of coding
 * effort to allow for multiple listeners per host. But no workload as yet
 * (benchmarked on 12-core Xeon) has been shown to warrant multiple listeners.
 * It's very possible that larger hosts may benefit from distributing
 * incoming TCP/IP traffic across multiple listeners, and the effort to allow
 * that won't be difficult to implement.
 */

#include "Listener.h"

#define EPOLLEVENTS 1024

Listener::Listener(Actor::identity_s identity)
    : Actor(identity)
{
}

void Listener::operator()()
{
    struct epoll_event ev={};
    ev.events=EPOLLIN | EPOLLHUP | EPOLLET;
    ev.data.fd=identity.sockfd;
    if (epoll_ctl(identity.epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev)==-1)
    {
        LOG("epoll_ctl problem");
        return;
    }

    struct rlimit rlim;
    if (getrlimit(RLIMIT_NOFILE, &rlim) != 0)
    {
        LOG("getrlimit RLIMIT_NOFILE problem");
        /** 
         * @todo this should probably be fatal for this node
         */
        return;
    }
    /**
     * assumes socket fd is always < number of available sockets
     */
    socketAffinity=new (std::nothrow) std::atomic<int64_t>[rlim.rlim_max]();
    if (socketAffinity==nullptr)
    {
        LOG("problem allocating for " << rlim.rlim_max);
        /**
         * @todo this should be fatal for the node
         */
        return;
    }
    rlim_t maxsocknum=rlim.rlim_max-1;
    
    struct sockaddr_in their_addr={}; // connector's address information
    socklen_t sin_size = sizeof(their_addr);
    int roundrobin=0;
    struct epoll_event events[EPOLLEVENTS];
    int optval=1;

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
                    if (newfd > (int)maxsocknum)
                    {
                        LOG("too high fd: " << newfd << " maxsocknum: " <<
                            maxsocknum);
                        close(newfd);
                        continue;
                    }

                    fcntl(newfd, F_SETFL, O_NONBLOCK);
                    setsockopt(newfd, SOL_SOCKET, SO_KEEPALIVE, &optval,
                               sizeof(optval));
                    ev.data.fd=newfd;
                    Mbox &mboxRef=*myTopology.localTransactionAgents[++roundrobin % myTopology.localTransactionAgents.size()];                    
                    socketAffinity[newfd]=(int64_t)&mboxRef;
                    epoll_ctl(identity.epollfd, EPOLL_CTL_ADD, newfd, &ev);
                    mboxRef.sendMsg(*(new MessageSocket(Message::TOPIC_SOCKETCONNECTED, myTopology.nodeid, newfd, event)));
                }
            }
            else
            { // already established socket
                int64_t mboxint=socketAffinity[fd];
                if (mboxint)
                {
                    ((Mbox *)mboxint)->sendMsg(*(new MessageSocket(Message::TOPIC_SOCKET, myTopology.nodeid, fd, event)));
                }
                else
                {
                    LOG("event " << event << " on spurious sockfd " << fd);
                }
            }
        }
    }
}
