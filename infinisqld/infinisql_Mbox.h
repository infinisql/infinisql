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

#ifndef MBOX_HPP
#define MBOX_HPP

#include "infinisql_Message.h"
#include "infinisql_Topology.h"

using std::vector;
using std::string;

class Mbox
{
public:
  Mbox();
  virtual ~Mbox();

  class Message *receive(int);  //int is wait

  struct pointer_s
  {
    class Message *ptr;
    uint64_t count;
  };

  static __int128 getInt128FromPointer(class Message *, uint64_t);
  static class Message *getPtr(__int128);
  static uint64_t getCount(__int128);

  friend class MboxProducer;

private:
  pthread_mutex_t mutexLast;

  class Message *firstMsg;
  class Message *currentMsg;
  class Message *lastMsg;
  class Message *myLastMsg; // not to be modified by producer

  __int128 head;
  __int128 tail;
  __int128 current;
  __int128 mytail;

  uint64_t counter;
};

class MboxProducer
{
public:
  MboxProducer();
  MboxProducer(class Mbox *);
  virtual ~MboxProducer();
  void sendMsg(class Message &);

  class Mbox *mbox;

  friend class Mboxes;
};

/** This class lets each actor keep track of destinations in a consistent way */
class Mboxes
{
public:
  struct location_s
  {
    Topology::addressStruct address;
    class MboxProducer *destmbox;
  };

  Mboxes();
  Mboxes(int64_t);
  virtual ~Mboxes();

  void update(class Topology &);
  void update(class Topology &, int64_t);
  void toActor(const Topology::addressStruct &,
               const Topology::addressStruct &, class Message &);
  void toUserSchemaMgr(const Topology::addressStruct &, class Message &);
  void toDeadlockMgr(const Topology::addressStruct &, class Message &);
  void toPartition(const Topology::addressStruct &, int64_t, class Message &);
  int64_t toAllOfType(actortypes_e, const Topology::addressStruct &,
                      class Message &);
  int64_t toAllOfTypeThisReplica(actortypes_e, const Topology::addressStruct &,
                                 class Message &);

  int64_t nodeid;

  class MboxProducer topologyMgr;
  class MboxProducer userSchemaMgr;
  class MboxProducer deadlockMgr;
  class MboxProducer listener;
  class MboxProducer obGateway;
  class MboxProducer ibGateway;
  vector<class MboxProducer> transactionAgents;
  vector<class MboxProducer> engines;

  // new
  vector<class MboxProducer *> actoridToProducers;
  vector<class MboxProducer *> transactionAgentPtrs;
  vector<class MboxProducer *> enginePtrs;
  class MboxProducer *topologyMgrPtr;
  location_s userSchemaMgrLocation;
  location_s deadlockMgrLocation;
  class MboxProducer *listenerPtr;
  class MboxProducer *obGatewayPtr;

  vector<location_s> partitionToProducers;
  // allActors[nodeid][actorid] = actortype
  vector< vector<int> > allActors;
  boost::unordered_map< int64_t, vector<int> > allActorsThisReplica;
};

#endif  /* MBOX_HPP */
