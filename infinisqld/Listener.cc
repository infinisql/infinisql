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

#include "infinisql_Listener.h"
#line 22 "Listener.cc"

#define EPOLLEVENTS 1024

Listener::Listener(Topology::partitionAddress *myIdentityArg)
    : myIdentity(*myIdentityArg)
{
    delete myIdentityArg;
    mboxes.nodeid = myIdentity.address.nodeid;
    mboxes.update(myTopology);

    int listenersockfd = startsocket(myIdentity.nodes[0], myIdentity.services[0]);
    int pglistenersockfd = startsocket(myIdentity.nodes[1],
                                       myIdentity.services[1]);
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLET;
    ev.data.fd = listenersockfd;

    if (epoll_ctl(myIdentity.epollfd, EPOLL_CTL_ADD, listenersockfd, &ev) == -1)
    {
        fprintf(logfile, "%s %i epoll_ctl errno %i\n", __FILE__, __LINE__, errno);
    }

    ev.data.fd = pglistenersockfd;

    if (epoll_ctl(myIdentity.epollfd, EPOLL_CTL_ADD, pglistenersockfd, &ev) == -1)
    {
        fprintf(logfile, "%s %i epoll_ctl errno %i\n", __FILE__, __LINE__, errno);
    }

    struct sockaddr_in their_addr; // connector's address information

    socklen_t sin_size = sizeof(their_addr);

    int roundrobin = 0;

    struct epoll_event events[EPOLLEVENTS];

    socketAffinity.resize(NUMSOCKETS+1, 0);
    listenerTypes.resize(NUMSOCKETS+1, LISTENER_NONE);

    class MboxProducer *producer=NULL;
    listenertype_e listenertype=LISTENER_NONE;
  
    while (1)
    {
        int eventcount = epoll_wait(myIdentity.epollfd, events, EPOLLEVENTS, -1);

        for (int n=0; n < eventcount; n++)
        {
            int fd = events[n].data.fd;
            int event = events[n].events;

            if (fd==listenersockfd || fd==pglistenersockfd)
            {
                if (fd==listenersockfd)
                {
                    if (event & EPOLLIN)
                    {
                        int newfd = accept(fd, (struct sockaddr *)&their_addr, &sin_size);

                        if (newfd == -1)
                        {
                            printf("%s %i accept errno %i\n", __FILE__, __LINE__, errno);
                            break;
                        }
                        if (newfd > NUMSOCKETS)
                        {
                            fprintf(logfile, "%s %i fd %i > %i\n", __FILE__, __LINE__, newfd,
                                    NUMSOCKETS);
                            close(newfd);
                            continue;
                        }

                        fcntl(newfd, F_SETFL, O_NONBLOCK);
                        int optval = 1;
                        setsockopt(newfd, SOL_SOCKET, SO_KEEPALIVE, &optval,
                                   sizeof(optval));
                        ev.data.fd = newfd;
                        pthread_mutex_lock(&connectionsMutex);
                        socketAffinity[newfd]=
                            mboxes.transactionAgentPtrs[roundrobin++ % myTopology.numtransactionagents];
                        listenerTypes[newfd]=LISTENER_RAW;
                        pthread_mutex_unlock(&connectionsMutex);
                        epoll_ctl(myIdentity.epollfd, EPOLL_CTL_ADD, newfd, &ev);
                    }
                }
                else if (fd==pglistenersockfd)
                {
                    //          if (event & EPOLLIN)
                    while (1)
                    {
                        int newfd = accept(fd, (struct sockaddr *)&their_addr, &sin_size);

                        if (newfd == -1)
                        {
                            if (errno != EAGAIN && errno != EWOULDBLOCK)
                            {
                                printf("%s %i accept errno %i\n", __FILE__, __LINE__, errno);
                            }
                            else
                            {
                                break;
                            }
                        }
                        if (newfd > NUMSOCKETS)
                        {
                            fprintf(logfile, "%s %i fd %i > %i\n", __FILE__, __LINE__, newfd,
                                    NUMSOCKETS);
                            close(newfd);
                            continue;
                        }

                        fcntl(newfd, F_SETFL, O_NONBLOCK);
                        int optval = 1;
                        setsockopt(newfd, SOL_SOCKET, SO_KEEPALIVE, &optval,
                                   sizeof(optval));
                        ev.data.fd = newfd;
                        pthread_mutex_lock(&connectionsMutex);
                        socketAffinity[newfd]=
                            mboxes.transactionAgentPtrs[roundrobin++ % myTopology.numtransactionagents];
                        listenerTypes[newfd]=LISTENER_PG;
                        pthread_mutex_unlock(&connectionsMutex);
                        epoll_ctl(myIdentity.epollfd, EPOLL_CTL_ADD, newfd, &ev);
                        socketAffinity[newfd]->sendMsg(*(new class MessageSocket(newfd, 0,
                                                                                 listenerTypes[newfd], myIdentity.address.nodeid,
                                                                                 TOPIC_SOCKETCONNECTED)));
                    }
                }
            }
            else
            { // already established socket activity
                pthread_mutex_lock(&connectionsMutex);
                if (socketAffinity[fd])
                {
                    producer=socketAffinity[fd];
                    listenertype=listenerTypes[fd];
                }
                else
                {
                    pthread_mutex_unlock(&connectionsMutex);
                    fprintf(logfile, "%s %i event %i on spurious sockfd %i\n", __FILE__,
                            __LINE__, event, fd);
                    continue;
                }
                pthread_mutex_unlock(&connectionsMutex);
                producer->sendMsg(*(new class MessageSocket(fd, event, listenertype,
                                                            myIdentity.address.nodeid, TOPIC_SOCKET)));
            }
        }
    }
}

Listener::~Listener()
{
}

void *listener(void *identity)
{
    Listener((Topology::partitionAddress *)identity);
    return NULL;
}

int Listener::startsocket(string &node, string &service)
{
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *servinfo;

    char *nodeptr = NULL;

    if (node.size())
    {
        nodeptr = (char *)node.c_str();
    }

    int rv = getaddrinfo((const char *)nodeptr, service.c_str(), &hints,
                         &servinfo);

    if (rv)
    {
        fprintf(logfile, "%s %i getaddrinfo %s %i\n", __FILE__, __LINE__,
                gai_strerror(rv), rv);
        exit(1);
    }

    int sockfd = 0;
    int yes = 1;
    struct addrinfo *p=NULL;

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (sockfd == -1)
        {
            fprintf(logfile, "%s %i socket errno %i\n", __FILE__, __LINE__, errno);
            continue;
        }

        fcntl(sockfd, F_SETFL, O_NONBLOCK);

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))==-1)
        {
            fprintf(logfile, "%s %i setsockopt errno %i\n", __FILE__, __LINE__,
                    errno);
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen)==-1)
        {
            close(sockfd);
            fprintf(logfile, "%s %i bind errno %i\n", __FILE__, __LINE__, errno);
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p==NULL)
    {
        fprintf(logfile, "%s %i listener: failed to bind\n", __FILE__, __LINE__);
        return -1; /**< TODO: handle listener failure */
    }

    if (listen(sockfd, 1000) == -1)
    {
        fprintf(logfile, "%s %i listen errno %i\n", __FILE__, __LINE__, errno);
        exit(1);
    }

    return sockfd;
}
