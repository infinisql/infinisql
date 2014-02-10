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

    struct sockaddr_in their_addr={}; // connector's address information
    socklen_t sin_size = sizeof(their_addr);
    int roundrobin=0;
    struct epoll_event events[EPOLLEVENTS];

    while(1)
    {
        int eventcount=epoll_wait(identity.epollfd, events, EPOLLEVENTS, -1);
        if (eventcount==-1)
        {
            LOG("epoll_wait problem");
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
                    if (newfd > NUMSOCKETS)
                    {
                        LOG("too high fd: " << newfd << " NUMSOCKETS: " <<
                            NUMSOCKETS);
                        close(newfd);
                        continue;
                    }

                    fcntl(newfd, F_SETFL, O_NONBLOCK);
                    int optval=1;
                    setsockopt(newfd, SOL_SOCKET, SO_KEEPALIVE, &optval,
                               sizeof(optval));
                    ev.data.fd=newfd;
                    // @todo socketAffinity[newfd]=mboxes transactionagent
                    epoll_ctl(identity.epollfd, EPOLL_CTL_ADD, newfd, &ev);
//                    socketAffinity[newfd]->sendMsg()
                }
            }
        }
    }
}
