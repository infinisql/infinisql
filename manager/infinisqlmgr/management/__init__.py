__author__ = 'Christopher Nelson'

import fcntl
import logging
import select
import signal
import socket
import struct
import time

import msgpack
import zmq

from infinisqlmgr.management import msg
from infinisqlmgr.management import election
from infinisqlmgr.management import health


class Controller(object):
    def __init__(self, cluster_name, data_dir="/tmp", mcast_group="224.0.0.1", mcast_port=21001, cmd_port=21000):
        """
        Creates a new management node controller.

        :param cluster_name: The cluster name that this node should belong to.
        :param data_dir:  The directory where data related to management status should be stored.
        :param mcast_group: The multicast group address.
        :param mcast_port: The port for the multicast group.
        :param cmd_port: The TCP command port for the management node.
        """
        self.ctx = zmq.Context.instance()
        self.poller = zmq.Poller()
        self.presence_poller = select.poll()

        self.cmd_socket = self.ctx.socket(zmq.PUB)
        self.sub_sockets = {}

        self.node_id = (self._get_ip(), cmd_port)
        self.nodes = set([self.node_id])
        self.leader_node_id = None
        self.current_election = None
        self.current_cluster_time = 0
        self.current_node_time = 0
        self.settle_time = 10
        self.heartbeat_period = 10
        self.node_partition_threshold = 50

        self.presence_recv_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.last_presence_announcement = 0
        self.presence_announcement_period = 1

        self.cluster_name = cluster_name
        self.mcast_group = mcast_group
        self.mcast_port = mcast_port
        self.cmd_port = cmd_port

        self.health = health.Health(self.node_id, data_dir)
        self.heartbeats = {}

        self.message_handlers = {}

        self._configure_presence_socket()
        self._configure_pub_socket()
        self._configure_message_handlers()

        # Flag is set to False when it's time to stop.
        self.keep_going = True

    def _configure_presence_socket(self):
        """
        Configures the presence socket.
        :return: None
        """
        self.presence_recv_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.presence_recv_socket.bind((self.mcast_group, self.mcast_port))
        multicast_req = struct.pack("=4sl", socket.inet_aton(self.mcast_group), socket.INADDR_ANY)
        self.presence_recv_socket.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, multicast_req)
        self.presence_poller.register(self.presence_recv_socket, select.POLLIN)

    def _configure_pub_socket(self):
        """
        Configures the command publisher socket.
        :return: None
        """
        self.cmd_socket.bind("tcp://*:%d" % self.cmd_port)

    def _configure_message_handlers(self):
        """
        Configures message handlers for all messages in the system.
        :return: None
        """
        for item in dir(msg):
            handler_name = "on_" + item.lower()
            if not hasattr(self, handler_name):
                continue
            self._register_handler(getattr(msg, item), getattr(self, handler_name))


    def _get_ip(self, interface="eth0"):
        """
        :param interface: The interface to get an ip address for.
        :return: A string containing the ip address.
        """
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sockfd = sock.fileno()
        SIOCGIFADDR = 0x8915
        ifreq = struct.pack('16sH14s', bytes(interface, "ascii"), socket.AF_INET, b'\x00'*14)
        try:
             res = fcntl.ioctl(sockfd, SIOCGIFADDR, ifreq)
        except:
            return None
        finally:
            sock.close()

        ip = struct.unpack('16sH2x4s8x', res)[2]
        return socket.inet_ntoa(ip)

    def _register_handler(self, msg_id, handler):
        """
        Register a message handler.

        :param msg_id: The message id to set a handler for.
        :param handler:  The handler to set.
        :return: None
        """
        self.message_handlers[msg_id] = handler

    def _unregister_handler(self, msg_id):
        """
        Unregister a message handler.
        :param msg_id: The message id to drop.
        :return: None
        """
        del self.message_handlers[msg_id]

    def _process_publication(self, sock):
        """
        Processes a publication from another management node.

        :param sock: The ZMQ socket to handle data from.
        :return: None
        """
        data = sock.recv()
        msg_id, payload = msgpack.unpackb(data)
        handler = self.message_handlers.get(msg_id)
        if handler is not None:
            handler(payload)

    def _process_presence(self, sock):
        data = sock.recv(4096)
        cluster_name, remote_ip, remote_port = msgpack.unpackb(data, encoding="utf8")
        self.add_node(cluster_name, remote_ip, remote_port)

    def _stop_signal_handler(self, signum, frame):
        """
        Handles SIGTERM to stop the process.

        :param signum: The signal (should be SIGTERM.)
        :param frame: The frame from the signal.
        :return: None
        """
        logging.debug("caught termination signal: signal %d", signum)
        self.stop()

    def _elect_leader(self):
        """
        Processes the leader election. If no election is in progress then a new election is started. If an election is
        in progress we check to see if the election is ready to proceed. If so, we election a new leader and terminate
        the election.

        :return: None
        """
        if self.current_election is None:
            logging.info("No leader, node=%s forcing election.", self.node_id)
            self.current_election = election.Election(self.nodes, self.current_node_time)
            best_candidate = self.current_election.get_best_candidate()

            logging.info("Node %s voting for %s", self.node_id, best_candidate)
            self.current_election.tally(best_candidate, self.node_id)
            self._send(msg.ELECT_LEADER, msgpack.packb((best_candidate, self.node_id)))
            return

        if not self.current_election.ready(self.current_node_time):
            if self.current_election.undecideable(self.current_node_time):
                logging.info("Election is undecideable, forcing re-election.")
                self.current_election = None
                self.leader_node_id = None
            return

        self.leader_node_id = self.current_election.get_winner()
        self.current_election = None

        logging.info("Elected new leader, node=%s", self.leader_node_id)

    def _beat_heart(self):
        """
        Broadcast a heartbeat, update current node time.
        :return: None
        """
        self.current_node_time+=1
        if self.current_node_time % self.heartbeat_period == 0:
            self._send(msg.HEARTBEAT, msgpack.packb((self.cluster_name, self.node_id), encoding="utf8"))

    def _evaluate_cluster_health(self):
        """
        Check to see if the other nodes in the cluster are still alive. If we have not received a heartbeat from
        them in self.node_partition_threshold ticks of the node clock then we remove the nodes
        :return: None
        """
        partitioned = []
        for node_id, last_heartbeat_time in self.heartbeats.items():
            if self.current_node_time - last_heartbeat_time > self.node_partition_threshold:
                partitioned.append(node_id)

        if not partitioned:
            return

        partition_list = ",".join([str(n) for n in partitioned])
        logging.error("Partition involving nodes '%s' experienced.", partition_list)

        if self.leader_node_id in partitioned:
            logging.error("Leader node %s was involved in a partition event.", self.leader_node_id)
            self.leader_node_id = None

        # Remove the remote node from our registries
        for node_id in partitioned:
            sock = self.sub_sockets[node_id]
            self.poller.unregister(sock)
            sock.close()

            del self.heartbeats[node_id]
            del self.sub_sockets[node_id]

            self.nodes.remove(node_id)

    def _check_topology_stability(self):
        """
        Performs some stability checks to make sure that the topology is in an optimal state. If an optimal state
         can't be achieved, at least check to see if we are in a functional degraded state.
        :return: None
        """
        self.health.capture()
        self._evaluate_cluster_health()
        if self.leader_node_id is None and self.current_node_time > self.settle_time:
            self._elect_leader()


    def _send(self, msg_id, payload):
        """
        Sends a message on the command socket.
        :param msg_id: The message id.
        :param payload: The payload associated with the message.
        :return: None
        """
        self.cmd_socket.send(msgpack.packb((msg_id, payload)))

    def add_node(self, cluster_name, remote_ip, remote_port):
        """
        Processes a presence announcement on the multicast socket. This will register
        a new sub socket in the poller if the announcement contains the expected
        signature.

        :param cluster_name: The name of the cluster the node says it is in.
        :param remote_ip: The remote ip to connect to.
        :param remote_port: The remote port to connect to.
        """

        logging.debug("received add_node command for cluster=%s, ip=%s:%d", cluster_name, remote_ip, remote_port)

        # Drop the announcement if the cluster name is not the one we're looking for.
        if cluster_name != self.cluster_name:
            logging.debug("dropping add_node because my cluster is '%s' and the new cluster is '%s'",
                          self.cluster_name, cluster_name)
            return

        # Drop the announcement if we are receiving our own broadcast
        if remote_ip == self._get_ip() and \
            remote_port == self.cmd_port:
            logging.debug("dropping add_node because I have caught my own announcement")
            return

        node_id = (remote_ip, remote_port)
        if node_id in self.nodes:
            logging.debug("dropping add_node because we already know this node")

        # Subscribe to the remote management node.
        remote_pub = "tcp://%s:%d" % (remote_ip, remote_port)
        sub_sock = self.ctx.socket(zmq.SUB)
        sub_sock.connect(remote_pub)
        sub_sock.setsockopt(zmq.SUBSCRIBE, bytes())
        self.sub_sockets[node_id] = sub_sock

        # Register the new subscription socket.
        self.poller.register(sub_sock, flags=zmq.POLLIN)
        self.nodes.add(node_id)

        # Initialize the heartbeat
        self.heartbeats[node_id] = self.current_node_time

    def on_elect_leader(self, m):
        """
        If this message is received while an election is in progress, then the message is considered as a
        vote from the node. If no election is in progress then this message forces an election. The received
        message is saved and tallied after a local election in the node is started.

        :param m: The message payload.
        :return: None
        """
        candidate, voter = msgpack.unpackb(m, encoding="utf8")

        if self.current_election is None:
            remote_node_id = (candidate, voter)
            logging.info("Election forced by %s, evicting old leader %s", remote_node_id, self.leader_node_id)

            self.leader_node_id = None
            self._elect_leader()

        self.current_election.tally(candidate, voter)

    def on_heartbeat(self, m):
        """
        Updates the heartbeat for some node.
        :param m: Contains the cluster name and remote node id.
        :return:
        """
        cluster_name, node_id = msgpack.unpackb(m, encoding="utf8")

        # Drop heartbeat if it's for a different cluster
        if self.cluster_name != cluster_name:
            return

        self.heartbeats[tuple(node_id)] = self.current_node_time

    def announce_presence(self, force=False):
        """
        Send a presence announcement to the local network segment.
        :param force: If set to true the announcement is forced regardless of the last time an announcement was made.
        """
        now = time.time()
        if  (now - self.last_presence_announcement < self.presence_announcement_period) and \
            force==False:
            return

        self.last_presence_announcement = now
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 8)
        sock.sendto(msgpack.packb([self.cluster_name, self._get_ip(), self.cmd_port]), (self.mcast_group, self.mcast_port))
        sock.close()

    def get_nodes(self):
        """
        Provides a set of node ids.
        :return: A set of node ids.
        """
        return self.nodes

    def is_node_known(self, node_id):
        """
        Indicates if the node_id is known to this manager.

        :param node_id: The node id to look for.
        :return: True if it's known, False otherwise.
        """
        return node_id in self.nodes

    def stop(self):
        """
        Sets the flag to stop the management process. This may take a second to react.
        :return: None
        """
        logging.info("Setting stop flag for management process.")
        self.keep_going = False

    def process_leader_tasks(self):
        """
        Performs processing of tasks delegated to the leader.
        :return:
        """
        self.current_cluster_time+=1

    def process(self):
        """
        Performs one loop of the poll process for network I/O. This may wait up to 1 second before returning if
        there is no pending activity.
        :return: None
        """
        self._beat_heart()

        # Perform local stability tasks
        self._check_topology_stability()

        # Poll presence socket
        events = self.presence_poller.poll(50)
        for sock, event in events:
            self._process_presence(self.presence_recv_socket)

        # Poll ZMQ sockets
        events = self.poller.poll(timeout=50)
        for sock, event in events:
            self._process_publication(sock)

        # Perform leader tasks if I am the leader.
        if self.leader_node_id == self.node_id:
            self.process_leader_tasks()

    def run(self):
        """
        The main run loop for the management process. Sets a signal handler so that the process can be
        stopped by sending SIGTERM. You can also call the "stop()" function from inside the same process
        to stop the management server.

        :return: 0 on success, nonzero for error conditions.
        """
        signal.signal(signal.SIGTERM, self._stop_signal_handler)
        logging.info("Started management process, announcing on %s:%s", self.mcast_group, self.mcast_port)
        while self.keep_going:
            self.announce_presence()
            self.process()
        logging.info("Stopped management process.")
        signal.signal(signal.SIGTERM, signal.SIG_DFL)
        return 0

    def shutdown(self):
        """
        Shuts down this management node, releases all resources.
        :return: None
        """
        self.stop()
        self.presence_poller.unregister(self.presence_recv_socket)
        for sock in self.sub_sockets.values():
            self.poller.unregister(sock)

        self.presence_recv_socket.close()
        self.cmd_socket.close()
        for sock in self.sub_sockets.values():
            sock.close()
