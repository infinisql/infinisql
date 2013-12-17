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

#include "IbGateway.h"
#line 22 "IbGateway.cc"

#define EPOLLEVENTS 1024

IbGateway::IbGateway(Topology::partitionAddress *myIdentityArg) :
    myIdentity(*myIdentityArg), fds(NULL), nfds(0), ismultinode(false)
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

    dcstrsmall=new (std::nothrow) char[SERIALIZEDMAXSIZE];
  
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
                    if (ismultinode==false)
                    {
                        setprio();
                        ismultinode=true;
                    }
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
                int readfd=fds[n].fd;

                do
                {
                    readed=read(readfd, inbuf, so_rcvbuf);
                    switch (readed)
                    {
                    case -1:
                        if (errno==EAGAIN || errno==EWOULDBLOCK)
                        {
                            if (pendingReads.count(readfd))
                            {
                                string &strRef=pendingReads[readfd];
                                size_t pos=0;
                                while (pos < strRef.size())
                                {
                                    if (sizeof(size_t)>(size_t)(strRef.size()-pos))
                                    { // can't even read size of message group
                                        break;
                                    }
                                    size_t packagesize=*(size_t *)(strRef.c_str()+pos);
                                    if (packagesize > strRef.size()-pos)
                                    { // can't read next message group entirely
                                        break;
                                    }
                                    inbufhandler(strRef.c_str()+pos, packagesize);
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
                            close(readfd);
                            fdremoveset.insert(readfd);
                            pendingReads.erase(readfd);
                            break;
                        }
                        break;
              
                    case 0:
                        close(readfd);
                        fdremoveset.insert(readfd);
                        pendingReads.erase(readfd);
                        break;
              
                    default:
                    {
                        if (pendingReads.count(readfd))
                        {
                            pendingReads[readfd].append(inbuf, readed);
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
                                pendingReads[readfd].assign(inbuf+pos, readed-pos);
                            }
                        }
                    }
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
    delete dcstrsmall;
}

// have read everything before processing
void IbGateway::inbufhandler(const char *buf, size_t bufsize)
{
    char *inbuf;
    size_t inbufsize;
    char *dcstr;
    char *dcstrbig;
    bool isdcstrbig=false;
  
    if (cfgs.compressgw==true)
    { // SERIALIZEDMAXSIZE
        int bs=SERIALIZEDMAXSIZE;
        dcstr=dcstrsmall;
        while (1)
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
            inbuf=dcstr;
            inbufsize=dcsize;
            break;
        }
    }
    else
    {
        inbuf=(char *)buf;
        inbufsize=bufsize;
    }
  
    size_t pos=sizeof(size_t); // i already know the whole size, it's bufsize
    while (pos<inbufsize)
    {
        size_t s=*(size_t *)(inbuf+pos);
        pos += sizeof(s);
        string *serstr=new string(inbuf+pos, s);
        pos += s;
        class MessageSerialized *msgsnd=new class MessageSerialized(serstr);
        mboxes.toActor(msgsnd->messageStruct.sourceAddr,
                       msgsnd->messageStruct.destAddr, *msgsnd);
    }

    if (isdcstrbig==true)
    {
        delete[] dcstrbig;
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
    new IbGateway((Topology::partitionAddress *)identity);
    while (1)
    {
        sleep(500000);
    }
    return NULL;
}
