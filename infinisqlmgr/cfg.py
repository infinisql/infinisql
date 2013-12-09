#! /usr/bin/env python2

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

import ConfigParser

import argparse
import io
import logging
import msgpack
import os
import subprocess
import types
import zmq

import cfgenum
import topology


ACTOR_NONE = 'ACTOR_NONE'
ACTOR_OBGATEWAY = 'ACTOR_OBGATEWAY'
ACTOR_TOPOLOGYMGR = 'ACTOR_TOPOLOGYMGR'
ACTOR_IBGATEWAY = 'ACTOR_IBGATEWAY'
ACTOR_ENGINE = 'ACTOR_ENGINE'
ACTOR_TRANSACTIONAGENT = 'ACTOR_TRANSACTIONAGENT'
ACTOR_DEADLOCKMGR = 'ACTOR_DEADLOCKMGR'
ACTOR_USERSCHEMAMGR = 'ACTOR_USERSCHEMAMGR'
ACTOR_LISTENER = 'ACTOR_LISTENER'

CMD_SET = 'CMDSET'
CMD_GET = 'CMDGET'
CMD_START = 'CMDSTART'
CMD_OK = 'CMDOK'

CMD_OBGATEWAY = 'CMDOBGATEWAY'
CMD_IBGATEWAY = 'CMDIBGATEWAY'
CMD_ENGINE = 'CMDENGINE'
CMD_TRANSACTIONAGENT = 'CMDTRANSACTIONAGENT'
CMD_DEADLOCKMGR = 'CMDDEADLOCKMGR'
CMD_USERSCHEMAMGR = 'CMDUSERSCHEMAMGR'
CMD_PGHANDLER = 'CMDPGHANDLER'
CMD_LISTENER = 'CMDLISTENER'
CMD_BADLOGINMESSAGES = 'CMDBADLOGINMESSAGES'
CMD_ANONYMOUSPING = 'CMDANONYMOUSPING'
CMD_GETTOPOLOGYMGRMBOXPTR = 'CMDGETTOPOLOGYMGRMBOXPTR'
CMD_LOCALCONFIG = 'CMDLOCALCONFIG'
CMD_GLOBALCONFIG = 'CMDGLOBALCONFIG'


nodes = {}

def parseargs():
    parser = argparse.ArgumentParser(description="InfiniSQL manager")
    parser.add_argument("-f", "--file", help="Config file", nargs=1,
                        default=[os.path.join(os.path.dirname(__file__), '..', 'etc', 'infinisqlmgr.conf')])
    parser.add_argument("--debug", dest="debug",
                        default=False, action="store_true",
                        help="Turn on debug logging.")
    args = parser.parse_args()

    format = "%(asctime)-15s %(levelname)s (%(module)s:%(lineno)s) %(message)s"
    logging.basicConfig(level=logging.INFO if not args.debug else logging.DEBUG, format=format)
    return args.file[0]


def sendcmd(nodeobj, serializedmsg):
    nodeobj.zmq_socket.send(str(serializedmsg))
    unpacker = msgpack.Unpacker()
    unpacker.feed(nodeobj.zmq_socket.recv())

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
        self.next_actor_id = cfgenum.firstactorid  # leave enough room for a bunch of
        # singleton actors
        # TopologyMgr in each node is always actorid 1
        # DeadlockMgr: 2
        # UserSchemaMgr: 3
        # Listener: 4
        # pghandler: 6
        self.next_ta_instance = -1
        self.next_engine_instance = -1
        self.next_ib_gateway_instance = -1
        self.next_ob_gateway_instance = -1
        self.actors = []
        self.context = None
        self.zmq_socket = None
        self.username = None
        self.ssh_key = None
        self.ssh = None
        self.infinisqld = None
        self.logfile = None
        self.global_admin_password = None
        self.cfg_host_port = None
        self.cfg_remote_host_port = None
        self.mgmt_host = None
        self.listen_host = None
        self.listen_port = None
        self.ib_gateway_host_port = None
        self.transaction_agents = None
        self.engines = None
        self.ib_gateways = None
        self.ob_gateways = None
        self.anonymous_ping = None
        self.bad_login_messages = None
        self.replica = None
        self.member = None
        self.pg_host = None
        self.pg_port = None

    def configure(self, config, section):
        self.username = config.get(section, 'username')
        self.ssh_key = config.get(section, 'sshkey')
        self.ssh = config.get(section, 'ssh')
        self.infinisqld = config.get(section, 'infinisqld')
        self.logfile = config.get(section, 'logfile')
        self.global_admin_password = config.get(section, 'globaladminpassword')
        self.cfg_host_port = config.get(section, 'cfghostport')
        self.cfg_remote_host_port = config.get(section, 'cfgremotehostport')
        self.mgmt_host = config.get(section, 'mgmthost')
        self.listen_host = config.get(section, 'listenhost')
        self.listen_port = config.get(section, 'listenport')
        self.ib_gateway_host_port = config.get(section, 'ibgatewayhostport')
        self.transaction_agents = config.getint(section, 'transactionagents')
        self.engines = config.getint(section, 'engines')
        self.ib_gateways = config.getint(section, 'ibgateways')
        self.ob_gateways = config.getint(section, 'obgateways')
        self.anonymous_ping = config.getint(section, 'anonymousping')
        self.bad_login_messages = config.getint(section, 'badloginmessages')
        self.replica = config.getint(section, 'replica')
        self.member = config.getint(section, 'member')
        self.pg_host = config.get(section, 'pghost')
        self.pg_port = config.get(section, 'pgport')

        if self.ssh_key is not None and not os.path.exists(self.ssh_key):
            logging.error("The specified ssh key '%s' does not exist.", self.ssh_key)

        ignore_types = (types.FunctionType, types.InstanceType, types.MethodType)
        for item in sorted(dir(self)):
            if item.startswith("_") or type(getattr(self, item)) in ignore_types:
                continue

            logging.debug("%s=%s", item, getattr(self, item))

    def getnextactorid(self):
        self.next_actor_id += 1
        return self.next_actor_id

    def getnexttainstance(self):
        self.next_ta_instance += 1
        return self.next_ta_instance

    def getnextengineinstance(self):
        self.next_engine_instance += 1
        return self.next_engine_instance

    def getnextibgatewayinstance(self):
        self.next_ib_gateway_instance += 1
        return self.next_ib_gateway_instance

    def getnextobgatewayinstance(self):
        self.next_ob_gateway_instance += 1
        return self.next_ob_gateway_instance

    def enable(self):
        self.start()
        self.connect()
        self.enabled = 1
        topo.addreplicamember(self.replica, self.member, self.id)
        if self.setanonymousping():
            logging.error('node %s problem setanonymousping', str(self.id))
        if self.setbadloginmessages():
            logging.error('node %s problem setbadloginmessages', str(self.id))
        if topo.deadlockmgrnode == self.id:
            if self.startdeadlockmgr():
                logging.error('node %s problem startdeadlockmgr', str(self.id))
        if topo.userschemamgrnode == self.id:
            if self.startuserschemamgr():
                logging.error('node %s problem startuserschemamgr', str(self.id))
        for obgateway in range(self.ob_gateways):
            if self.startobgateway():
                logging.error('node %s problem starting obgateway %s' , str(self.id), str(obgateway))
        for ta in range(self.transaction_agents):
            if self.starttransactionagent():
                logging.error('node %s problem starttransactionagent %s' , str(self.id), str(ta))
        for engine in range(self.engines):
            if self.startengine():
                logging.error('node %s problem startengine %s', str(self.id), str(engine))
            # listener & ibgw need to start last
        # (for now, 1/20/31) because can't know of new tas
        if self.startlistener():
            logging.error('node %s problem startlistener', str(self.id))
        for ibgateway in range(self.ib_gateways):
            if self.startibgateway():
                logging.error('node %s problem startibgateway', str(self.id))

        self.globalupdate()

    def start(self):
        if len(self.mgmt_host) == 0:
            args = [self.infinisqld,
                    "-n" + str(self.id),
                    "-l" + self.logfile,
                    "-m" + self.cfg_host_port]
            logging.debug("Starting local infinisql daemon: %s", str(args))
            # just start daemon
            retval = subprocess.call(args)
            if retval:
                logging.error("Problem starting node %d", str(self.id))
                return retval
        else:
            args = [self.ssh,
                    "-i" + self.ssh_key,
                    self.mgmt_host,
                    " LD_PRELOAD=/usr/local/lib/libllalloc.so.1.4 " + self.infinisqld,
                    "-n" + str(self.id),
                    "-l" + self.logfile,
                    "-m" + self.cfg_host_port,
                    " >" + self.logfile + ".out 2>" + self.logfile + ".err "]
            logging.debug("Starting remote infinisql daemon: %s", str(args))
            # ssh in and start daemon
            subprocess.call(args)

    def connect(self):
        self.context = zmq.Context()
        self.zmq_socket = self.context.socket(zmq.REQ)
        self.zmq_socket.connect("tcp://" + self.cfg_remote_host_port)
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_GET],
                                            cfgenum.cfgforwarddict[CMD_GETTOPOLOGYMGRMBOXPTR]]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
        mboxptr = returnit.next()
        self.addactor(1, cfgenum.actortypesforwarddict[ACTOR_TOPOLOGYMGR],
                      -1, mboxptr)
        self.nodeupdate()
        return 0

    def setanonymousping(self):
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_SET],
                                            cfgenum.cfgforwarddict[CMD_ANONYMOUSPING], self.anonymous_ping]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
        return 0

    def setbadloginmessages(self):
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_SET],
                                            cfgenum.cfgforwarddict[CMD_BADLOGINMESSAGES], self.bad_login_messages]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
        return 0

    def startlistener(self):
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_START],
                                            cfgenum.cfgforwarddict[CMD_LISTENER], 4, self.listen_host,
                                            self.listen_port, self.pg_host, self.pg_port]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
        self.addactor(4, cfgenum.actortypesforwarddict[ACTOR_LISTENER],
                      -1, returnit.next())
        # has no mbox
        self.nodeupdate()
        return 0

    def startpghandler(self):
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_START],
                                            cfgenum.cfgforwarddict[CMD_PGHANDLER], 6, self.pg_host,
                                            self.pg_port]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
            # has no mbox
        return 0

    def startuserschemamgr(self):
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_START],
                                            cfgenum.cfgforwarddict[CMD_USERSCHEMAMGR], 3,
                                            self.global_admin_password]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
        mboxptr = returnit.next()
        self.addactor(3, cfgenum.actortypesforwarddict[ACTOR_USERSCHEMAMGR],
                      -1, mboxptr)
        self.nodeupdate()
        topo.userschemamgrmboxptr = mboxptr
        self.globalupdate()
        return 0

    def startdeadlockmgr(self):
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_START],
                                            cfgenum.cfgforwarddict[CMD_DEADLOCKMGR], 2]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
        mboxptr = returnit.next()
        self.addactor(2, cfgenum.actortypesforwarddict[ACTOR_DEADLOCKMGR],
                      -1, mboxptr)
        self.nodeupdate()
        topo.deadlockmgrmboxptr = mboxptr
        self.globalupdate()
        return 0

    def starttransactionagent(self):
        actorid = self.getnextactorid()
        instance = self.getnexttainstance()
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_START],
                                            cfgenum.cfgforwarddict[CMD_TRANSACTIONAGENT], actorid, instance]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
        self.addactor(actorid,
                      cfgenum.actortypesforwarddict[ACTOR_TRANSACTIONAGENT],
                      instance, returnit.next())
        self.nodeupdate()
        topo.addta(self.id, instance, actorid)
        self.globalupdate()
        return 0

    def startengine(self):
        actorid = self.getnextactorid()
        instance = self.getnextengineinstance()
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_START],
                                            cfgenum.cfgforwarddict[CMD_ENGINE], actorid, instance]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
        self.addactor(actorid, cfgenum.actortypesforwarddict[ACTOR_ENGINE],
                      instance, returnit.next())
        topo.addpartition(topo.getnextpartitionid(), self.replica, self.id, actorid)
        self.nodeupdate()
        self.globalupdate()
        return 0

    def startibgateway(self):
        actorid = self.getnextactorid()
        instance = self.getnextibgatewayinstance()
        hp = self.ib_gateway_host_port.split(':')
        ibgatewayhostport = hp[0] + ':' + str(int(hp[1]) + instance)
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_START],
                                            cfgenum.cfgforwarddict[CMD_IBGATEWAY], actorid, instance,
                                            ibgatewayhostport]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
        self.addactor(actorid, cfgenum.actortypesforwarddict[ACTOR_IBGATEWAY],
                      instance, returnit.next())
        topo.addibgateway(self.id, instance, ibgatewayhostport)
        self.nodeupdate()
        self.globalupdate()
        return 0

    def startobgateway(self):
        actorid = self.getnextactorid()
        instance = self.getnextobgatewayinstance()
        returnit = sendcmd(self, serialize([cfgenum.cfgforwarddict[CMD_START],
                                            cfgenum.cfgforwarddict[CMD_OBGATEWAY], actorid, instance]))
        if cfgenum.cfgreversedict[returnit.next()] != CMD_OK:
            return 1
        self.addactor(actorid, cfgenum.actortypesforwarddict[ACTOR_OBGATEWAY],
                      instance, returnit.next())
        self.nodeupdate()
        self.globalupdate()
        return 0

    def addactor(self, actorid, type, instance, mboxptr):
        for x in range((actorid + 1) - len(self.actors)):
            self.actors.append([cfgenum.actortypesforwarddict[ACTOR_NONE], 0, 0])
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
            for x in nodes[n].actors:
                ts.append(int(x[0]))
            allactors.append(ts)

        for n in nodes:
            if nodes[n].enabled:
            #      if n.enabled:
                vector = [cfgenum.cfgforwarddict[CMD_GLOBALCONFIG],
                          topo.activereplica, replicaids, nodeids, actorids,
                          ibgatewaynodes, ibgatewayinstances, ibgatewayhostports,
                          topo.deadlockmgrnode, topo.deadlockmgrmboxptr,
                          topo.userschemamgrnode, topo.userschemamgrmboxptr,
                          len(topo.replicamembers)]
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

    def nodeupdate(self):
        types = []
        instances = []
        mboxptrs = []
        for x in self.actors:
            types.append(int(x[0]))
            instances.append(x[1])
            mboxptrs.append(x[2])
        returnit = sendcmd(self, serialize(
            [cfgenum.cfgforwarddict[CMD_LOCALCONFIG], types, instances,
             mboxptrs]))

# end of class node

def cfg(cfgfile):
    logging.debug("using configuration file '%s'", cfgfile)
    config = ConfigParser.SafeConfigParser()
    config.read(cfgfile)
    global nodes

    topo.deadlockmgrnode = config.getint('global', 'deadlockmgrnode')
    topo.userschemamgrnode = config.getint('global', 'userschemamgrnode')
    topo.activereplica = config.getint('global', 'activereplica')

    for s in config.sections():
        if s.find('node_') == 0:
            logging.debug("configuring node '%s'", s)
            n = node(s.split('_')[1])
            n.configure(config, s)
            nodes[n.id] = n

# main of this module
topo = topology.globaltopology()
cfg(parseargs())
