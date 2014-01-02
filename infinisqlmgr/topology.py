#! /usr/bin/env python2

# Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
# All rights reserved. No warranty, explicit or implicit, provided.
#
# This file is part of InfiniSQL(tm).
 
# InfiniSQL is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3
# as published by the Free Software Foundation.
#
# InfiniSQL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with InfiniSQL. If not, see <http://www.gnu.org/licenses/>.

class globaltopology:
  def __init__(self):
    self.partitions = []
    self.ibgateways = []
    self.userschemamgrnode = 0
    self.userschemamgrmboxptr = 0
    self.nextpartitionid = 0
    self.replicamembers = []
    self.tas = []

  def getnextpartitionid(self):
    pid = self.nextpartitionid
    self.nextpartitionid += 1
    return pid

  def addpartition(self, pid, replica, nodeid, actorid):
    for x in range((pid+1) - len(self.partitions)):
      self.partitions.append( [-1, 0, 0] )
    self.partitions[pid] = (replica, nodeid, actorid)

  def addibgateway(self, nodeid, instance, hostport):
    self.ibgateways.append( (nodeid, instance, hostport) )

  def addreplicamember(self, replica, member, nodeid):
    for x in range((replica+1) - len(self.replicamembers)):
      self.replicamembers.append( [[-1]] )
    for y in range((member+1) - len(self.replicamembers[replica])):
      self.replicamembers[replica].append([-1])
    self.replicamembers[replica][member] = nodeid

  def addta(self, nodeid, instance, actorid):
    for x in range((nodeid+1) - len(self.tas)):
      self.tas.append([ -1 ])
    for y in range((instance+1) - len(self.tas[nodeid])):
      self.tas[nodeid].append(-1)
    self.tas[nodeid][instance] = actorid

