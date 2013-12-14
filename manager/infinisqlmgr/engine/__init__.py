__author__ = 'Christopher Nelson'

import logging
import os
import signal

import zmq

from infinisqlmgr.engine import msg

class Configuration(object):
    def __init__(self, node_id, dist_dir, management_ip, management_port):
        self.node_id = node_id
        self.dist_dir = dist_dir
        self.infinisql = os.path.join(dist_dir, "sbin", "infinisqld")
        self.log_file = os.path.join(dist_dir, "var", "log", "infinisql.%s.log" % node_id)
        self.management_ip = management_ip
        self.management_port = management_port
        self.pid = None

        self.ctx = zmq.Context.instance()
        self.socket = None
        self.poller = zmq.Poller()

    def _connect(self):
        logging.debug("connecting to database engine management port (%s:%s)", self.management_ip, self.management_port)
        self.socket = self.ctx.socket(zmq.REQ)
        self.socket.connect("tcp://%s:%s" % self.management_ip, self.management_port)

    def _send(self, msg):
        self.socket.send(msg)

    def process(self):
        pass

    def start(self):
        """
        Starts a database engine.
        :return: None
        """
        if self.pid is not None:
            logging.debug("database engine already started")
            return

        # -m <management ip:port> -n <nodeid> -l <log path/file>
        self.pid = os.spawnlp(os.P_NOWAIT, self.infinisql,
                              "-m", "%s:%s" % (self.management_ip, self.management_port),
                              "-n", str(self.node_id),
                              "-l", self.log_file)

        logging.info("Started database engine pid=%s", self.pid)

    def stop(self):
        """
        Stops a database engine.
        :return: None
        """
        if self.pid is None:
            logging.debug("database engine already stopped")
            return

        logging.info("Stopping database engine pid=%s", self.pid)
        os.kill(self.pid, signal.SIGTERM)
        os.waitpid(self.pid, 0)
        logging.info("Stopped database engine.")
        self.pid = None


