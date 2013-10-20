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

#include "infinisql_ObGateway.h"
#line 28 "ObGateway.cc"

ObGateway::ObGateway(Topology::partitionAddress *myIdentityArg) :
  myIdentity(*myIdentityArg), so_sndbuf(16777216), ismultinode(false)
{
  delete myIdentityArg;

  mboxes.nodeid = myIdentity.address.nodeid;
  mboxes.update(myTopology);
  updateRemoteGateways();

  optlen=sizeof(so_sndbuf);
  int waitfor = 100;
  
//  bool buildup=true;
  
  // pendingMsgs[remotenodeid]=serialized messages
  char *sendstr=NULL;
  size_t sendsize=0;
  boost::unordered_map< int64_t, vector<string *> > pendingMsgs;
  char *serstr=NULL;
  serstrsmall=new (std::nothrow) char[SERIALIZEDMAXSIZE];
  char *serstrbig=NULL;
  bool isserstrbig=false;
  char *cstr=NULL;
  cstrsmall=new (std::nothrow) char[SERIALIZEDMAXSIZE];
  char *cstrbig=NULL;
  bool iscstrbig=false;

  /*
  uint32_t D_sends=0;
  uint64_t D_sendbytes=0;
   */

  while (1)
  {
    for (size_t inmsg=0; inmsg < 5000; inmsg++)
    {
      class Message *msgrcv = myIdentity.mbox->receive(waitfor);

      if (msgrcv==NULL)
      {
        waitfor = 100;
        break;
      }

      waitfor = 0;

      switch (msgrcv->messageStruct.topic)
      {
        case TOPIC_TOPOLOGY:
          mboxes.update(myTopology);
          updateRemoteGateways();
          break;

        case TOPIC_SERIALIZED: // destined for remote host
          pendingMsgs[msgrcv->messageStruct.destAddr.nodeid].push_back(
              ((class MessageSerialized *)msgrcv)->data);
          break;
          
        case TOPIC_BATCHSERIALIZED:
        {
          /*
          boost::unordered_multimap<int16_t, string *> &msgsRef=
                  ((class MessageBatchSerialized *)msgrcv)->msgs;
          boost::unordered_multimap<int16_t, string *>::const_iterator it;
           */
          /*
          boost::unordered_map< int16_t, vector<string *> > &msgsRef=
                  ((class MessageBatchSerialized *)msgrcv)->msgs;
          boost::unordered_map< int16_t, vector<string *> >::const_iterator it;
          for (it=msgsRef.begin(); it != msgsRef.end(); it++)
          {
//            pendingMsgs[it->first].push_back(it->second);
            vector<string *> &stringsRef=pendingMsgs[it->first];
            stringsRef.insert(stringsRef.end(), it->second.begin(),
                    it->second.end());
          }
           */
          class MessageBatchSerialized &msgRef=
              *(class MessageBatchSerialized *)msgrcv;
          for (short n=0; n<msgRef.nmsgs; n++)
          {
            pendingMsgs[msgRef.msgbatch[n].nodeid].push_back(msgRef.msgbatch[n].serializedmsg);
          }
        }
        break;
        
        default:
          printf("%s %i anomaly %i\n", __FILE__, __LINE__,
                  msgrcv->messageStruct.topic);
      }
    }

    /*
    // simple flow control
    if (buildup==true)
    {
      usleep(100);
      buildup=false;
      continue;
    }
    buildup=true;
     */

    ssize_t sended;

    // send all pendings
    boost::unordered_map< int64_t, vector<string *> >::iterator it;
    for (it=pendingMsgs.begin(); it != pendingMsgs.end(); it++)
    {
      vector<string *> &msgsRef = it->second;
      size_t s=sizeof(size_t);
      for (size_t n=0; n < msgsRef.size(); n++)
      {
        s += sizeof(size_t) + msgsRef[n]->size();
      }
      if (!s)
      {
        continue;
      }
      // format is: total size, [msgsize, msg]...
      if (s>SERIALIZEDMAXSIZE)
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
      memcpy(serstr, &s, sizeof(s));
      size_t pos=sizeof(s);
      for (size_t n=0; n < msgsRef.size(); n++)
      {
        string &msgRef=*msgsRef[n];
        size_t ms=msgRef.size();
        memcpy(serstr+pos, &ms, sizeof(ms));
        pos += sizeof(size_t);
        memcpy(serstr+pos, msgRef.c_str(), ms);
        pos += ms;
        delete &msgRef;
      }
      
      // compress
      if (cfgs.compressgw==true)
      {
        int cbound=LZ4_compressBound(s);
        if (cbound+sizeof(size_t) > SERIALIZEDMAXSIZE)
        {
          cstrbig=new (std::nothrow) char[cbound+sizeof(size_t)];
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
      }
      else
      {
        sendstr=serstr;
        sendsize=s;
      }

      if ((sended=send(remoteGateways[it->first], sendstr, sendsize, 0))==-1)
      {
        printf("%s %i send errno %i nodeid %i instance %li it->first %li socket %i\n",
               __FILE__, __LINE__, errno, myTopology.nodeid, myIdentity.instance,
               it->first, remoteGateways[it->first]);
      }
      pendingMsgs.erase(it->first);

      if (isserstrbig==true)
      {
        delete serstrbig;
      }
      if (iscstrbig==true)
      {
        delete cstrbig;
      }

      /*
      D_sendbytes+=sended;
      if (!(++D_sends % 1000))
      {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        fprintf(logfile, "X\t%s\t%i\t%li\t%li\t%u\t%lu\n", __FILE__, __LINE__, myIdentity.instance, tv.tv_sec*1000000+tv.tv_usec, D_sends, D_sendbytes);
      }
       */
    }
  }
}

ObGateway::~ObGateway()
{
  delete serstrsmall;
  delete cstrsmall;
}

void ObGateway::updateRemoteGateways()
{
  map< int64_t, vector<string> >::iterator it;

  if (!myTopology.ibGateways.empty() &&
          (int64_t)remoteGateways.size() < myTopology.ibGateways.rbegin()->first+1)
  {
    remoteGateways.resize(myTopology.ibGateways.rbegin()->first+1, 0);
  }

  for (it = myTopology.ibGateways.begin(); it != myTopology.ibGateways.end();
       it++)
  {
    vector<string> &vecstringRef = it->second;
    
    if (it->first==myTopology.nodeid)
    {
      continue;
    }
    if ((int64_t)myTopology.ibGateways[it->first].size() < myIdentity.instance+1)
    {
      continue;
    }
    
    if (remoteGateways[it->first]==0)
    {
      // connect to server
      if (ismultinode==false)
      {
        setprio();
        ismultinode=true;
      }
      int sockfd;
      size_t found = vecstringRef[myIdentity.instance].find(':');
      string node = vecstringRef[myIdentity.instance].substr(0, found);
      string service = vecstringRef[myIdentity.instance].substr(found+1,
              vecstringRef[myIdentity.instance].size()-(found+1));
      struct addrinfo hints;
      memset(&hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;
      struct addrinfo *servinfo;

      if (getaddrinfo(node.c_str(), service.c_str(), &hints,
              &servinfo))
      {
        fprintf(logfile, "%s %i getaddrinfo\n", __FILE__, __LINE__);
        return;
      }

      sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
              servinfo->ai_protocol);
      if (sockfd == -1)
      {
        fprintf(logfile, "%s %i socket\n", __FILE__, __LINE__);
        return;
      }

      if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen))
      {
        fprintf(logfile, "%s %i connect errno %i '%s:%s'\n", __FILE__, __LINE__,
                errno, node.c_str(), service.c_str());
        return;
      }
    
      if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &so_sndbuf, sizeof(so_sndbuf))==-1)
      {
        fprintf(logfile, "%s %i setsockopt errno %i\n", __FILE__, __LINE__,
                errno);
        continue;
      }

      freeaddrinfo(servinfo);

      remoteGateways[it->first] = sockfd;
    }
  }
}

// launcher, regular function
void *obGateway(void *identity)
{
  ObGateway((Topology::partitionAddress *)identity);
  return NULL;
}
