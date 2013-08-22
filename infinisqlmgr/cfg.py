#! /usr/bin/env python

# Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
# All rights reserved. No warranty, explicit or implicit, provided.
#
# This file is part of InfiniSQL (tm). It is available either under the
# GNU Affero Public License or under a commercial license. Contact the
# copyright holder for information about a commercial license if terms
# of the GNU Affero Public License do not suit you.
#
# This copy of InfiniSQL is free software: you can redistribute it and/or
# modify it under the terms of the GNU Affero Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# InfiniSQL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero Public License for more details.
#
# You should have received a copy of the GNU Affero Public License
# along with InfiniSQL. It should be in the top level of the source
# directory in a file entitled "COPYING".
# If not, see <http://www.gnu.org/licenses/>.

import argparse
import ConfigParser
import subprocess
import zmq
import msgpack
import sys
import io

import cfgenum
import topology

#nodes=[]
nodes=dict()

def parseargs():
  parser = argparse.ArgumentParser(description="InfiniSQL manager")
  parser.add_argument("-f", "--file", help="Config file", nargs=1, 
    default=['../etc/infinisqlmgr.conf'])
  args = parser.parse_args()
  return args.file[0]

def sendcmd(nodeobj, serializedmsg):
  nodeobj.zmqsocket.send(str(serializedmsg))
  unpacker = msgpack.Unpacker()
  unpacker.feed(nodeobj.zmqsocket.recv())

  return unpacker.__iter__()

def serialize(inlist):
  buf = io.BytesIO()
  for t in inlist:
    buf.write(msgpack.packb(t))
  return buf.getvalue()

class node:
  def __init__(self, id):
    self.id = int(id)
    self.enabled = 0
    self.nextactorid = cfgenum.firstactorid  # leave enough room for a bunch of 
                                     # singleton actors
                           # TopologyMgr in each node is always actorid 1
                           # DeadlockMgr: 2
                           # UserSchemaMgr: 3
                           # Listener: 4
                           # connectionHandler: 5
                           # pghandler: 6
    self.nexttainstance = -1
    self.nextengineinstance = -1
    self.nextibgatewayinstance = -1
    self.nextobgatewayinstance = -1
    self.actors = []

  def getnextactorid(self):
    self.nextactorid += 1
    return self.nextactorid

  def getnexttainstance(self):
    self.nexttainstance += 1
    return self.nexttainstance

  def getnextengineinstance(self):
    self.nextengineinstance += 1
    return self.nextengineinstance

  def getnextibgatewayinstance(self):
    self.nextibgatewayinstance += 1
    return self.nextibgatewayinstance

  def getnextobgatewayinstance(self):
    self.nextobgatewayinstance += 1
    return self.nextobgatewayinstance

  def enable(self):
    self.start()
    self.connect()
    self.enabled = 1
    topo.addreplicamember(self.replica, self.member, self.id)
    if self.setanonymousping():
      print 'node ' + str(self.id) + ' problem setanonymousping'
    if self.setbadloginmessages():
      print 'node ' + str(self.id) + ' problem setbadloginmessages'
#    if self.startlistener():
#      print 'node ' + str(self.id) + ' problem startlistener'
    if topo.deadlockmgrnode==self.id:
      if self.startdeadlockmgr():
        print 'node ' + str(self.id) + ' problem startdeadlockmgr'
    if topo.userschemamgrnode==self.id:
      if self.startuserschemamgr():
        print 'node ' + str(self.id) + ' problem startuserschemamgr'
    for obgateway in range(self.obgateways):
      if self.startobgateway():
        print 'node ' + str(self.id) + 'problem starting obgateway ' + \
        str(obgateway)
    for ta in range(self.transactionagents):
      if self.starttransactionagent():
        print 'node ' + str(self.id) + ' problem starttransactionagent '  + \
            str(ta)
    for engine in range(self.engines):
      if self.startengine():
        print 'node ' + str(self.id) + ' problem startengine ' + str(engine)
    # ch & ibgw & pghandler need to start last (for now, 1/20/31) because
    # can't know of new tas
    if self.startconnectionhandler():
      print 'node ' + str(self.id) + ' problem startconnectionhandler'
    for ibgateway in range(self.ibgateways):
      if self.startibgateway():
        print 'node ' + str(self.id) + 'problem startibgateway'
#    if self.startpghandler():
#      print 'node ' + str(self.id) + ' problem startpghandler'

    self.globalupdate()

  def start(self):
    if len(self.mgmthost)==0:
      # just start daemon
      retval = subprocess.call([self.infinisqld, "-n" + str(self.id), "-l" +
          self.logfile, "-m" + self.cfghostport, "-p" + self.pgsockfile])
      if retval:
        print "problem starting node " + str(self.id)
        return retval
    else:
      # ssh in and start daemon
      subprocess.call([self.ssh, "-i" + self.sshkey, self.mgmthost, 
          " LD_PRELOAD=/usr/local/lib/libllalloc.so.1.4 " +
          self.infinisqld, "-n" + str(self.id), "-l" + self.logfile,
          "-m" + self.cfghostport, "-p" + self.pgsockfile, " >" + 
          self.logfile + ".out 2>" + self.logfile + ".err "])

  def connect(self):
    self.context = zmq.Context()
    self.zmqsocket = self.context.socket(zmq.REQ)
    self.zmqsocket.connect("tcp://" + self.cfgremotehostport)
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDGET'],
        cfgenum.cfgforwarddict['CMDGETTOPOLOGYMGRMBOXPTR']] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    mboxptr = returnit.next()
    self.addactor(1, cfgenum.actortypesforwarddict['ACTOR_TOPOLOGYMGR'],
        -1, mboxptr)
    self.nodeupdate()
    return 0

  def setanonymousping(self):
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSET'],
        cfgenum.cfgforwarddict['CMDANONYMOUSPING'], self.anonymousping] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    return 0

  def setbadloginmessages(self):
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSET'],
      cfgenum.cfgforwarddict['CMDBADLOGINMESSAGES'], self.badloginmessages] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    return 0

  def startlistener(self):
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSTART'],
      cfgenum.cfgforwarddict['CMDLISTENER'], 4, self.listenhost,
      self.listenport] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    # has no mbox 
    return 0

  def startpghandler(self):
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSTART'],
      cfgenum.cfgforwarddict['CMDPGHANDLER'], 6, self.pghost,
      self.pgport] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    # has no mbox 
    return 0

  def startconnectionhandler(self):
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSTART'],
        cfgenum.cfgforwarddict['CMDCONNECTIONHANDLER'], 5, self.listenhost,
        self.listenport, self.pghost, self.pgport] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    self.addactor(5, cfgenum.actortypesforwarddict['ACTOR_CONNECTIONHANDLER'],
        -1, returnit.next())
    self.nodeupdate()
    return 0

  def startuserschemamgr(self):
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSTART'],
      cfgenum.cfgforwarddict['CMDUSERSCHEMAMGR'], 3,
      self.globaladminpassword] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    mboxptr = returnit.next()
    self.addactor(3, cfgenum.actortypesforwarddict['ACTOR_USERSCHEMAMGR'],
        -1, mboxptr)
    self.nodeupdate()
    topo.userschemamgrmboxptr = mboxptr
    self.globalupdate()
    return 0

  def startdeadlockmgr(self):
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSTART'],
      cfgenum.cfgforwarddict['CMDDEADLOCKMGR'], 2] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    mboxptr = returnit.next()
    self.addactor(2, cfgenum.actortypesforwarddict['ACTOR_DEADLOCKMGR'],
        -1, mboxptr)
    self.nodeupdate()
    topo.deadlockmgrmboxptr = mboxptr
    self.globalupdate()
    return 0

  def starttransactionagent(self):
    actorid = self.getnextactorid()
    instance = self.getnexttainstance()
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSTART'],
      cfgenum.cfgforwarddict['CMDTRANSACTIONAGENT'], actorid, instance] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    self.addactor(actorid,
        cfgenum.actortypesforwarddict['ACTOR_TRANSACTIONAGENT'],
        instance, returnit.next())
    self.nodeupdate()
    topo.addta(self.id, instance, actorid)
    self.globalupdate()
    return 0

  def startengine(self):
    actorid = self.getnextactorid()
    instance = self.getnextengineinstance()
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSTART'],
        cfgenum.cfgforwarddict['CMDENGINE'], actorid, instance] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    self.addactor(actorid, cfgenum.actortypesforwarddict['ACTOR_ENGINE'],
        instance, returnit.next())
    topo.addpartition(topo.getnextpartitionid(), self.replica, self.id, actorid)
    self.nodeupdate()
    self.globalupdate()
    return 0

  def startibgateway(self):
    actorid = self.getnextactorid()
    instance = self.getnextibgatewayinstance()
    hp = self.ibgatewayhostport.split(':')
    ibgatewayhostport = hp[0] + ':' + str(int(hp[1]) + instance)
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSTART'],
        cfgenum.cfgforwarddict['CMDIBGATEWAY'], actorid, instance,
        ibgatewayhostport] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    self.addactor(actorid, cfgenum.actortypesforwarddict['ACTOR_IBGATEWAY'],
        instance, returnit.next())
    topo.addibgateway(self.id, instance, ibgatewayhostport)
    self.nodeupdate()
    self.globalupdate()
    return 0

  def startobgateway(self):
    actorid = self.getnextactorid()
    instance = self.getnextobgatewayinstance()
    returnit = sendcmd(self, serialize( [cfgenum.cfgforwarddict['CMDSTART'],
        cfgenum.cfgforwarddict['CMDOBGATEWAY'], actorid, instance] ))
    if cfgenum.cfgreversedict[returnit.next()] != 'CMDOK':
      return 1
    self.addactor(actorid, cfgenum.actortypesforwarddict['ACTOR_OBGATEWAY'],
        instance, returnit.next())
    self.nodeupdate()
    self.globalupdate()
    return 0

  def addactor(self, actorid, type, instance, mboxptr):
    for x in range((actorid+1) - len(self.actors)):
      self.actors.append( [cfgenum.actortypesforwarddict['ACTOR_NONE'] , 0, 0] )
    self.actors[actorid] = [type, instance, mboxptr]

  def globalupdate(self):
    replicaids = []
    nodeids = []
    actorids = []
    ibgatewaynodes = []
    ibgatewayinstances = []
    ibgatewayhostports = []
    for x in topo.partitions:
      replicaids.append(x[0])
      nodeids.append(x[1])
      actorids.append(x[2])
    for x in topo.ibgateways:
      ibgatewaynodes.append(x[0])
      ibgatewayinstances.append(x[1])
      ibgatewayhostports.append(x[2])

    allactors = []
    for n in nodes:
      ts = []
      allactors.append(int(n))
#      allactors.append(int(n.id))
      for x in nodes[n].actors:
#      for x in n.actors:
        ts.append(int(x[0]))
      allactors.append(ts)

    for n in nodes:
      if nodes[n].enabled:
#      if n.enabled:
        vector = [ cfgenum.cfgforwarddict['CMDGLOBALCONFIG'],
            topo.activereplica, replicaids, nodeids, actorids,
            ibgatewaynodes, ibgatewayinstances, ibgatewayhostports,
            topo.deadlockmgrnode, topo.deadlockmgrmboxptr,
            topo.userschemamgrnode, topo.userschemamgrmboxptr,
            len(topo.replicamembers) ]
        for x in topo.replicamembers:
          members = []
          for y in x:
            members.append(y)
          vector.append(members)
        vector.append(len(topo.tas))
        for x in topo.tas:
          instance = []
          for y in x:
            instance.append(y)
          vector.append(instance)
        for x in allactors:
          vector.append(x)
        returnit = sendcmd(nodes[n], serialize(vector))
#        returnit = sendcmd(n, serialize(vector))

  def nodeupdate(self):
    types = []
    instances = []
    mboxptrs = []
    for x in self.actors:
      types.append(int(x[0]))
      instances.append(x[1])
      mboxptrs.append(x[2])
    returnit = sendcmd(self, serialize(
        [cfgenum.cfgforwarddict['CMDLOCALCONFIG'], types, instances,
        mboxptrs] ))

# end of class node

def cfg(cfgfile):
  config = ConfigParser.SafeConfigParser()
  config.read(cfgfile)
  global nodes

  topo.deadlockmgrnode = config.getint('global', 'deadlockmgrnode')
  topo.userschemamgrnode = config.getint('global', 'userschemamgrnode')
  topo.activereplica = config.getint('global', 'activereplica')

  for s in config.sections():
    if s.find('node_')==0:
      n = node(s.split('_')[1])

      n.username = config.get(s, 'username')
      n.sshkey = config.get(s, 'sshkey')
      n.ssh = config.get(s, 'ssh')
      n.infinisqld = config.get(s, 'infinisqld')
      n.logfile = config.get(s, 'logfile')
      n.globaladminpassword = config.get(s, 'globaladminpassword')
      n.cfghostport = config.get(s, 'cfghostport')
      n.cfgremotehostport = config.get(s, 'cfgremotehostport')
      n.mgmthost = config.get(s, 'mgmthost')
      n.listenhost = config.get(s, 'listenhost')
      n.listenport = config.get(s, 'listenport')
      n.ibgatewayhostport = config.get(s, 'ibgatewayhostport')
      n.transactionagents = config.getint(s, 'transactionagents')
      n.engines = config.getint(s, 'engines')
      n.ibgateways = config.getint(s, 'ibgateways')
      n.obgateways = config.getint(s, 'obgateways')
      n.anonymousping = config.getint(s, 'anonymousping')
      n.badloginmessages = config.getint(s, 'badloginmessages')
      n.replica = config.getint(s, 'replica')
      n.member = config.getint(s, 'member')
      n.pghost = config.get(s, 'pghost')
      n.pgport = config.get(s, 'pgport')
      n.pgsockfile = config.get(s, 'pgsockfile')

      nodes[n.id]=n
#      nodes.append(n)

# main of this module
topo = topology.globaltopology()
cfg(parseargs())
#for n in nodes:
#  topo.addreplicamember(n.replica, n.member, n.id)
#  print 'addreplicamember ' + str(n.id)

