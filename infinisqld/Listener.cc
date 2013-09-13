/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL (tm). It is available either under the
 * GNU Affero Public License or under a commercial license. Contact the
 * copyright holder for information about a commercial license if terms
 * of the GNU Affero Public License do not suit you.
 *
 * This copy of InfiniSQL is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with InfiniSQL. It should be in the top level of the source
 * directory in a file entitled "COPYING".
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "infinisql_Listener.h"
#include "infinisql_ConnectionHandler.h"
#line 29 "Listener.cc"

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
  //  ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLET;
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

  // accept() on unix domain socket
  struct sockaddr_un remotesock;
  socklen_t remotesocklen = sizeof(remotesock);
  int udsockfd = accept(listenerudsockfd, (struct sockaddr *)&remotesock,
                        &remotesocklen);
  if (udsockfd==-1)
  {
    printf("%s %i can't accept on listenerudsockfile %s errno %i\n",
           __FILE__, __LINE__, listenerudsockfile.c_str(), errno);
    exit(1);
  }

  fcntl(udsockfd, F_SETFL, O_NONBLOCK);
  ev.data.fd = udsockfd;

  if (epoll_ctl(myIdentity.epollfd, EPOLL_CTL_ADD, udsockfd, &ev) == -1)
  {
    fprintf(logfile, "%s %i epoll_ctl errno %i\n", __FILE__, __LINE__, errno);
  }

  struct sockaddr_in their_addr; // connector's address information

  socklen_t sin_size = sizeof(their_addr);

  int roundrobin = 0;
  int64_t roundrobinconnectionhandler = 0;

  struct epoll_event events[EPOLLEVENTS];

  //  char peekbuf[64];
  //  ssize_t peeked=0;

  while (1)
  {
    //    memset(events, 0, EPOLLEVENTS * sizeof(epoll_event));
    int eventcount = epoll_wait(myIdentity.epollfd, events, EPOLLEVENTS, -1);

    for (int n=0; n < eventcount; n++)
    {
      int fd = events[n].data.fd;
      int event = events[n].events;

      if (fd==listenersockfd || fd==pglistenersockfd || fd==udsockfd)
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
            /*
            epoll_ctl(myIdentity.epollfd, EPOLL_CTL_ADD, newfd, &ev);
            listenerTypeMap[newfd] = LISTENER_RAW;
            socketAffinity[newfd] = mboxes.transactionAgentPtrs[roundrobin++ %
                                    myTopology.numtransactionagents];
             */
            class ConnectionHandler &chPtrRef=
                *myTopology.connectionHandlers[roundrobinconnectionhandler++ %
                myTopology.numconnectionhandlers];
            pthread_mutex_lock(&chPtrRef.connectionsMutex);
            chPtrRef.socketAffinity[newfd]=
                mboxes.transactionAgentPtrs[roundrobin++ % myTopology.numtransactionagents];
            chPtrRef.listenerTypes[newfd]=LISTENER_RAW;
            pthread_mutex_unlock(&chPtrRef.connectionsMutex);
            epoll_ctl(chPtrRef.epollfd, EPOLL_CTL_ADD, newfd, &ev);
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
            /*
            epoll_ctl(myIdentity.epollfd, EPOLL_CTL_ADD, newfd, &ev);
            listenerTypeMap[newfd] = LISTENER_PG;
            socketAffinity[newfd] = mboxes.transactionAgentPtrs[roundrobin++ %
                                    myTopology.numtransactionagents];
             */
            class ConnectionHandler &chPtrRef=
                *myTopology.connectionHandlers[roundrobinconnectionhandler++ %
                myTopology.numconnectionhandlers];
            pthread_mutex_lock(&chPtrRef.connectionsMutex);
            chPtrRef.socketAffinity[newfd]=
                mboxes.transactionAgentPtrs[roundrobin++ % myTopology.numtransactionagents];
            chPtrRef.listenerTypes[newfd]=LISTENER_PG;
            pthread_mutex_unlock(&chPtrRef.connectionsMutex);
            epoll_ctl(chPtrRef.epollfd, EPOLL_CTL_ADD, newfd, &ev);
          }
        }
        else //udsockfd
        {
          class MessageSocket *m = NULL;

          while (recv(fd, &m, sizeof(class MessageSocket *), 0)==
                 sizeof(class MessageSocket *))
          {
            closesocket(m->socket);
            delete m;
          }
        }
      }
      else
      {
        if (socketAffinity.count(fd))
        {
          //          peeked=recv(fd, peekbuf, 64, MSG_PEEK);
          //          if (peeked==0 || peeked==-1)
          //          {
          //            closesocket(fd);
          //          }
          //          else
          //          {
          socketAffinity[fd]->sendMsg(*(new class MessageSocket(fd, event,
                                        listenerTypeMap[fd], 0)));
          //          }
        }
        else
        {
          printf("%s %i event %i on spurious sockfd %i\n", __FILE__, __LINE__, event, fd);
        }
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

void Listener::closesocket(int socket)
{
  epoll_ctl(myIdentity.epollfd, EPOLL_CTL_DEL, socket, NULL);
  socketAffinity.erase(socket);
  listenerTypeMap.erase(socket);
  close(socket);
}
