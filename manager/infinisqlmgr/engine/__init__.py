__author__ = 'Christopher Nelson'

import logging
import os
import signal

import psutil
import zmq

from infinisqlmgr.engine import state

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

        self.state = state.ConfigurationState(self)
        self.receive_handler = None

    def _connect(self):
        """
        Connects to the database engine's management port.
        :return: None
        """
        ip = "127.0.0.1" if self.management_ip=="*" else self.management_ip

        logging.debug("connecting to database engine management port (%s:%s)", self.management_ip, self.management_port)
        self.socket = self.ctx.socket(zmq.REQ)
        self.socket.connect("tcp://%s:%s" % (ip, self.management_port))
        self.poller.register(self.socket, zmq.POLLIN)
        self.state.get_topology_mgr_mbox_ptr(self.socket)

    def set_next_handler(self, handler):
        self.receive_handler = handler

    def process(self):
        """
        Processes engine events.
        :return: None
        """
        events = self.poller.poll(timeout=100)
        for sock, event in events:
            if self.receive_handler is None:
                continue
            handler = self.receive_handler
            self.receive_handler = None
            handler(sock)

    def start(self):
        """
        Starts a database engine.
        :return: None
        """
        if self.pid is not None:
            logging.debug("database engine already started")
            return

        log_dir = os.path.dirname(self.log_file)
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)

        # -m <management ip:port> -n <nodeid> -l <log path/file>
        self.pid = os.spawnl(os.P_NOWAIT, self.infinisql, self.infinisql,
                              '-m', '%s:%s' % (self.management_ip, self.management_port),
                              '-n', str(self.node_id),
                              '-l', self.log_file)

        logging.info("Started database engine pid=%s", self.pid)
        self._connect()

    def stop(self):
        """
        Stops a database engine.
        :return: None
        """
        if self.pid is None:
            logging.debug("database engine already stopped")
            return

        logging.debug("disconnecting from database engine management command port")
        self.poller.unregister(self.socket)
        self.socket.close()
        self.socket = None

        logging.info("Stopping database engine pid=%s", self.pid)
        os.kill(self.pid, signal.SIGTERM)
        os.waitpid(self.pid, 0)

        # Now search for any remaining database processes in order to ensure that we have closed everything down.
        for p in psutil.process_iter():
            if "infinisqld" in p.name:
                logging.info("Terminating '%s' (pid=%s)", p.name, p.pid)
                p.terminate()
        logging.info("Stopped database engine.")
        self.pid = None


