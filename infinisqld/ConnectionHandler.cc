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

#include "infinisql_ConnectionHandler.h"
#line 28 "ConnectionHandler.cc"

#define EPOLLEVENTS 1024

ConnectionHandler::ConnectionHandler(Topology::partitionAddress *myIdentityArg)
  : myIdentity(*myIdentityArg)
{
  delete myIdentityArg;
  mboxes.nodeid = myIdentity.address.nodeid;
  
  pthread_mutexattr_t attr = {};
  attr.__align = PTHREAD_MUTEX_ADAPTIVE_NP;
  pthread_mutex_init(&connectionsMutex, &attr);
  epollfd = epoll_create(1);
  
  socketAffinity.resize(NUMSOCKETS+1, 0);
  listenerTypes.resize(NUMSOCKETS+1, LISTENER_NONE);
  
  pthread_mutex_lock(&nodeTopologyMutex);
  if (myIdentity.instance+1 > (int64_t)nodeTopology.connectionHandlers.size())
  {
    nodeTopology.connectionHandlers.resize(myIdentity.instance+1, NULL);
  }
  nodeTopology.connectionHandlers[myIdentity.instance]=this;
  nodeTopology.numconnectionhandlers=nodeTopology.connectionHandlers.size();
  pthread_mutex_unlock(&nodeTopologyMutex);
  mboxes.update(myTopology);
  
  struct epoll_event events[EPOLLEVENTS];
  class MboxProducer *producer=NULL;
  listenertype_e listenertype=LISTENER_NONE;

  while (1)
  {
    int eventcount = epoll_wait(epollfd, events, EPOLLEVENTS, -1);

    for (int n=0; n < eventcount; n++)
    {
      int fd = events[n].data.fd;
      int event = events[n].events;
      if (fd > NUMSOCKETS)
      {
        fprintf(logfile, "%s %i fd %i > %i\n", __FILE__, __LINE__, fd, NUMSOCKETS);
        continue;
      }
      
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
              myIdentity.instance)));
    }
  }
}

ConnectionHandler::~ConnectionHandler()
{
}

void *connectionHandler(void *identity)
{
  ConnectionHandler((Topology::partitionAddress *)identity);
  return NULL;
}
