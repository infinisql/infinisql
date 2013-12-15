__author__ = 'Christopher Nelson'

import logging

from io import BytesIO
from infinisqlmgr.engine import cfg, msg

import msgpack

class ConfigurationState(object):
    def __init__(self, controller):
        self.controller = controller
        self.actors = {}
        self.replica_members = {}

    def _recv(self, sock):
        """
        Reads data from a socket and feeds it into an unpacker.
        :param sock: The socket to read from.
        :return: An unpacker.
        """
        unpacker = msgpack.Unpacker()
        unpacker.feed(sock.recv(copy=False))
        return unpacker

    def _send(self, sock, data):
        """
        Writes data into a socket.
        :param sock: The socket to write into.
        :param data: The data to write (this should be an iterable.)
        :return: None
        """
        buf = BytesIO()
        for item in data:
           buf.write(msgpack.packb(item))
        sock.send(buf.getvalue(), copy=False)

    def add_actor(self, actor_id, actor_type, instance, mbox_ptr):
        """
        Adds actor information.

        :param actor_id: The actor id.
        :param actor_type: The actor type.
        :param instance: The instance of the actor.
        :param mbox_ptr: The mailbox pointer for this actor.
        :return:
        """
        # Update the specified id.
        self.actors[actor_id] = (actor_type, instance, mbox_ptr)

    def add_replica_member(self, replica, member, node_id):
        self.replica_members[(replica, member)] = node_id

    def update_node(self, sock):
        """
        Updates the database engine with configuration information about the actors.
        :return: None
        """
        types = []
        instances = []
        mbox_ptrs = []

        actor_ids = sorted(self.actors.keys())

        # Whenever an actor is added, we update the database engine with their configuration
        # information. This loop collects that into a few parallel structures and sends it
        # off to the database engine's topology manager.
        for actor_id in actor_ids:
            x = self.actors[actor_id]
            types.append(int(x[0]))
            instances.append(x[1])
            mbox_ptrs.append(x[2])

        self._send(sock, (cfg.CMD_LOCALCONFIG, types, instances, mbox_ptrs))
        for msg in self._recv(sock):
            if msg != cfg.CMD_OK:
                logging.warning("Expected CMD_OK updating node local config, but response=%s", msg)

    def get_topology_mgr_mbox_ptr(self, sock):
        """
        Issues a GET_TOPOLOGY_MANAGER_MAILBOX_POINTER command and sets the receive handler to process the response.
        :param sock: The socket to write to.
        :return: None
        """
        self._send(sock, (cfg.CMD_GET, cfg.CMD_GETTOPOLOGYMGRMBOXPTR))
        self.controller.set_next_handler(self.on_get_topology_mgr_mbox_ptr)

    def on_get_topology_mgr_mbox_ptr(self, sock):
        """
        Handles a GET_TOPOLOGY_MANAGER_MAILBOX_POINTER response.
        :param sock: The socket to read from.
        :return: None
        """
        stream = self._recv(sock)
        result = next(stream)
        if result != cfg.CMD_OK:
            logging.error("Expected CMD_OK, but received %s", result)
            return False
        mbox_ptr = next(stream)
        self.add_actor(1, cfg.ACTOR_TOPOLOGYMGR, -1, mbox_ptr)
        self.update_node(sock)

