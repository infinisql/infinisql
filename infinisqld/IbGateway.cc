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

#include "infinisql_IbGateway.h"
#line 28 "IbGateway.cc"

#define EPOLLEVENTS 1024

IbGateway::IbGateway(Topology::partitionAddress *myIdentityArg) :
  myIdentity(*myIdentityArg), fds(NULL), nfds(0)
{
  delete myIdentityArg;

  mboxes.nodeid = myIdentity.address.nodeid;
  mboxes.update(myTopology);
  
  size_t found = myIdentity.argstring.find(':');
  string node = myIdentity.argstring.substr(0, found);
  string service = myIdentity.argstring.substr(found+1,
                   myIdentity.argstring.size()-(found+1));

  struct addrinfo hints = {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;
  struct addrinfo *servinfo;

  int rv = getaddrinfo(node.c_str(), service.c_str(), &hints, &servinfo);

  if (rv)
  {
    fprintf(logfile, "%s %i getaddrinfo %s %i\n", __FILE__, __LINE__,
            gai_strerror(rv), rv);
    exit(1);
  }

  int sockfd = 0;
  int yes = 1;
  struct addrinfo *p=NULL;
  int so_rcvbuf=16777216;
  socklen_t optlen=sizeof(so_rcvbuf);
  inbuf=new (std::nothrow) char[so_rcvbuf];
  if (inbuf==NULL)
  {
    fprintf(logfile, "%s %i malloc inbuf failed\n", __FILE__, __LINE__);
    exit(1);
  }

  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

    if (sockfd == -1)
    {
      fprintf(logfile, "%s %i socket errno %i\n", __FILE__, __LINE__, errno);
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))==-1)
    {
      fprintf(logfile, "%s %i setsockopt errno %i\n", __FILE__, __LINE__,
              errno);
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf, optlen)==-1)
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
    return; /**< TODO: handle listener failure */
  }

  fcntl(sockfd, F_SETFL, O_NONBLOCK==-1);

  if (listen(sockfd, 1000) == -1)
  {
    fprintf(logfile, "%s %i listen errno %i\n", __FILE__, __LINE__, errno);
    exit(1);
  }

  struct sockaddr_in their_addr; // connector's address information

  socklen_t sin_size = sizeof(their_addr);

  addtofds(sockfd);

  while (1)
  {
    int eventcount = poll(fds, nfds, -1);

    if (eventcount < 0)
    {
      continue;
    }

    int m = 0;

    for (nfds_t n=0; n < nfds; n++)
    {
      if (m==eventcount)
      {
        break;
      }

      if (!fds[n].revents)
      {
        continue;
      }

      m++;
      short event = fds[n].revents;

      // if it's the listening socket, then accept(), otherwise
      if (fds[n].fd==sockfd)
      {
        if ((event & EPOLLERR) || (event & EPOLLHUP))
        {
          continue;
        }
        else if (event & POLLIN)
        {
          int newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

          if (newfd == -1)
          {
            printf("%s %i accept errno %i\n", __FILE__, __LINE__, errno);
            continue;
          }
          if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf, optlen)==-1)
          {
            fprintf(logfile, "%s %i setsockopt errno %i\n", __FILE__, __LINE__,
                    errno);
            continue;
          }
          fcntl(newfd, F_SETFL, O_NONBLOCK);
          addtofds(newfd);
          continue;
        }
      }

      if ((event & EPOLLERR) || (event & EPOLLHUP))
      {
        close(fds[n].fd);
        fdremoveset.insert(fds[n].fd);
        fprintf(logfile, "%s %i instance %li n %lu\n", __FILE__, __LINE__, myIdentity.instance, n);
        continue;
      }
      else if (event & POLLIN)
      {
        ssize_t readed;

        do
        {
          readed = read(fds[n].fd, inbuf, so_rcvbuf);

          if (readed == -1)
          {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
              close(fds[n].fd);
              fdremoveset.insert(fds[n].fd);
              pendingReads.erase(fds[n].fd);
              break;
            }

            continue;
          }

          if (readed && pendingReads.count(fds[n].fd))
          {
            string instr = pendingReads[fds[n].fd];
            pendingReads.erase(fds[n].fd);
            instr.append(inbuf, readed);
            inbufhandler(fds[n].fd, instr.c_str(), instr.size());
          }
          else
          {
            inbufhandler(fds[n].fd, inbuf, readed);
          }
        }
        while (readed > 0);
      }
    }

    removefds();
  }
}

IbGateway::~IbGateway()
{
  delete inbuf;
}

void IbGateway::inbufhandler(int fd, const char *buf, ssize_t bufsize)
{
  ssize_t pos = 0;

  while (pos < bufsize)
  {
    if (bufsize - pos < (ssize_t)sizeof(ssize_t))
    {
      pendingReads[fd].assign(buf+pos, bufsize-pos);
      return;
    }

    ssize_t len;
    memcpy(&len, buf+pos, sizeof(ssize_t));

    if (len > bufsize-(pos+(ssize_t)sizeof(ssize_t)))
    {
      pendingReads[fd].assign(buf+pos, bufsize-pos);
      return;
    }

    msgpack::sbuffer sbuf;
    sbuf.write(buf+pos+sizeof(ssize_t), len);
    msgpack::unpacker unpack;
    unpack.reserve_buffer(sbuf.size());
    memcpy(unpack.buffer(), sbuf.data(), sbuf.size());
    unpack.buffer_consumed(sbuf.size());

    class Message *msg;

    while ((msg = Message::deser(unpack)) != NULL)
    {
      mboxes.toActor(msg->sourceAddr, msg->destAddr, *msg);
    }

    pos += sizeof(ssize_t) + len;
  }
}

void IbGateway::addtofds(int newfd)
{
  struct pollfd *newfds = new struct pollfd[++nfds];

  for (nfds_t n=0; n < nfds-1; n++)
  {
    newfds[n] = fds[n];
  }

  newfds[nfds-1] = {newfd, EPOLLIN | EPOLLERR | EPOLLHUP, 0};

  if (fds != NULL)
  {
    delete fds;
  }

  fds = newfds;
}

void IbGateway::removefds()
{
  if (!fdremoveset.size())
  {
    return;
  }

  struct pollfd *newfds = new struct pollfd[nfds - fdremoveset.size()];

  nfds_t m=0;

  for (nfds_t n=0; n < nfds - fdremoveset.size(); n++)
  {
    if (!fdremoveset.count(fds[n].fd))
    {
      newfds[m++] = fds[n];
    }
  }

  delete fds;
  fds = newfds;
  nfds -= fdremoveset.size();
  fdremoveset.clear();
  fprintf(logfile, "%s %i instance %li removefds nfds %lu\n", __FILE__, __LINE__, myIdentity.instance, nfds);
}

// launcher, regular function
void *ibGateway(void *identity)
{
  IbGateway((Topology::partitionAddress *)identity);
  return NULL;
}
