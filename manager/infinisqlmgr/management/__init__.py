__author__ = 'Christopher Nelson'

import ipaddress
import logging
import select
import signal
import socket
import struct
import time

import msgpack
import tornado.ioloop
import tornado.web
import zmq
import zmq.eventloop.ioloop as zmq_ioloop

import infinisqlmgr
from infinisqlmgr.management import msg, election, health, api


class Controller(object):
    # Contains the current controller instance. This is only available
    # when the controller is run() in tornado I/O loop mode.
    instance = None

    def __init__(self, config):
        """
        Creates a new management node controller.

        :param cluster_name: The cluster name that this node should belong to.
        :param data_dir:  The directory where data related to management status should be stored.
        :param mcast_group: The multicast group address.
        :param mcast_port: The port for the multicast group.
        :param cmd_port: The TCP command port for the management node.
        """
        self.config = config
        mt = self.config.config["management"]

        self.ctx = zmq.Context.instance()
        self.poller = zmq.Poller()
        self.presence_poller = select.poll()

        self.cmd_socket = self.ctx.socket(zmq.PUB)
        self.sub_sockets = {}

        self.cluster_name = mt["cluster_name"]
        self.mcast_group = mt["announcement_mcast_group"]
        self.mcast_port = int(mt["announcement_mcast_port"])
        self.cmd_port = int(mt["management_port"])
        self.dist_dir = config.dist_dir

        self.node_id = (mt["management_ip"], self.cmd_port)
        if mt["management_ip"] == "*":
            self.nodes = {(interface[0].ip.compressed, self.cmd_port) for interface in self.config.interfaces()
            .values()}
        else:
            self.nodes = {self.node_id}

        print(self.nodes)

        self.leader_node_id = None
        self.current_election = None
        self.current_cluster_time = 0
        self.current_cluster_size = 1
        self.current_node_time = 0
        self.current_election_id = 0
        self.peak_cluster_size = 1

        self.settle_time = 10
        self.heartbeat_period = 10
        self.node_partition_threshold = 50

        self.presence_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.last_presence_announcement = 0
        self.presence_announcement_period = 1

        self.application = None

        self.health = health.Health(self.node_id, config)
        self.heartbeats = {}

        self.engines = {}

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
        self.presence_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.presence_socket.bind((self.mcast_group, self.mcast_port))
        multicast_req = struct.pack("=4sl", socket.inet_aton(self.mcast_group), socket.INADDR_ANY)
        self.presence_socket.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, multicast_req)
        self.presence_poller.register(self.presence_socket, select.POLLIN)

    def _configure_pub_socket(self):
        """
        Configures the command publisher socket.
        :return: None
        """
        address = "tcp://%s:%d" % (self.config.get("management", "management_ip"), self.cmd_port)
        logging.debug("binding management configuration to '%s'", address)
        self.cmd_socket.bind(address)

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

    def _process_publication(self, sock, events):
        """
        Processes a publication from another management node.

        :param sock: The ZMQ socket to handle data from.
        :param events: The events that triggered this handler.
        :return: None
        """
        data = sock.recv()
        msg_id, payload = msgpack.unpackb(data)
        handler = self.message_handlers.get(msg_id)
        if handler is not None:
            handler(payload)

    def _process_presence(self, sock, events):
        """
        Processes announcements received on the presence socket.

        :param sock: The socket to read.
        :param events: The events that triggered this handler.
        :return: None
        """
        data = self.presence_socket.recv(4096)
        cluster_name, remote_port, remote_interfaces = msgpack.unpackb(data, encoding="utf8")

        logging.debug("announcement received: %s", str(remote_interfaces))
        # Figure out which of the available IP addresses is reachable from our system.
        remote_interfaces = {ipaddress.ip_interface(interface) for interface in remote_interfaces}
        local_interfaces = {interface[0] for interface in self.config.interfaces().values()}

        if remote_interfaces == local_interfaces and remote_port == self.cmd_port:
            logging.debug("ignoring presence announcement because it is our own")
            return

        for local_interface in local_interfaces:
            for remote_interface in remote_interfaces:
                li = local_interface
                ri = remote_interface
                if li.network.compare_networks(ri.network) == 0:
                    self.add_node(cluster_name, ri.ip.compressed, remote_port)

    def _stop_signal_handler(self, signum, frame):
        """
        Handles SIGTERM to stop the process.

        :param signum: The signal (should be SIGTERM.)
        :param frame: The frame from the signal.
        :return: None
        """
        logging.debug("caught termination signal: signal %d", signum)
        self.stop()
        zmq_ioloop.ZMQIOLoop.instance().stop()

    def _start_transaction_engines(self):
        """
        Turns on all transaction engines for this node.
        :return:
        """
        logging.info("Starting transaction engines.")

    def _shutdown_transaction_engines(self):
        """
        Turns off all transaction engines.
        :return: None
        """
        logging.warning("Shutting down all transaction engines for management node %s", self.node_id)

    def _elect_leader(self):
        """
        Processes the leader election. If no election is in progress then a new election is started. If an election is
        in progress we check to see if the election is ready to proceed. If so, we election a new leader and terminate
        the election.

        The exception to the rule is when the cluster is part of a minority partition. In this case no elections
        are allowed to proceed.

        :return: None
        """
        if self.current_cluster_size < self.peak_cluster_size/2:
            return

        if self.current_election is None:
            logging.info("No leader, node=%s forcing election.", self.node_id)
            self.current_election = election.Election(self.nodes, self.current_node_time)
            best_candidate = self.current_election.get_best_candidate()

            logging.info("Node %s voting for %s", self.node_id, best_candidate)
            self.current_election.tally(best_candidate, self.node_id)
            self._send(msg.ELECT_LEADER, msgpack.packb((best_candidate, self.node_id, self.current_election_id)))
            return

        if not self.current_election.ready(self.current_node_time):
            if self.current_election.undecideable(self.current_node_time):
                logging.info("Election is undecideable, forcing re-election.")
                self.current_election = None
                self.leader_node_id = None
            return

        self.leader_node_id = self.current_election.get_winner()
        self.current_election = None
        self.current_election_id += 1

        logging.info("On node=%s, elected new leader=%s in election %s",
                     self.node_id, self.leader_node_id, self.current_election_id - 1)

    def _beat_heart(self):
        """
        Broadcast a heartbeat, update current node time.
        :return: None
        """
        self.current_node_time += 1
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

        # Re-adjust the current cluster size
        self.current_cluster_size = len(self.nodes)
        if self.current_cluster_size < self.peak_cluster_size/2:
            logging.warning("Node %s is part of a minority partition. "
                            "Elections will not proceed until a majority is established.",
                            self.node_id)
            self._shutdown_transaction_engines()

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
        zmq_ioloop.ZMQIOLoop.instance().add_handler(sub_sock, self._process_publication, zmq_ioloop.ZMQIOLoop.READ)
        self.poller.register(sub_sock, flags=zmq.POLLIN)
        self.nodes.add(node_id)
        self.current_cluster_size += 1
        self.peak_cluster_size = max(self.current_cluster_size, self.peak_cluster_size)

        # Initialize the heartbeat
        self.heartbeats[node_id] = self.current_node_time

        # Erase the leader
        self.leader_node_id = None

    def on_elect_leader(self, m):
        """
        If this message is received while an election is in progress, then the message is considered as a
        vote from the node. If no election is in progress then this message forces an election. The received
        message is saved and tallied after a local election in the node is started.

        :param m: The message payload.
        :return: None
        """
        candidate, voter, election_id = msgpack.unpackb(m, encoding="utf8")

        # If we receive an election request for an election newer than ours. Restart the election.
        if election_id > self.current_election_id:
            logging.info("Election %s is newer than %s, abandoning current election.",
                         election_id, self.current_election_id)
            self.current_election = None
            self.current_election_id = election_id

        if self.current_election is None:
            remote_node_id = (candidate, voter)
            logging.info("Election %s forced by %s, evicting old leader %s",
                         self.current_election_id, remote_node_id, self.leader_node_id)

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
        if (now - self.last_presence_announcement < self.presence_announcement_period) and force == False:
            return

        interfaces = self.config.interfaces()
        ips = sorted([interface[0].with_prefixlen for interface in interfaces.values()])

        self.last_presence_announcement = now
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 8)
        sock.sendto(msgpack.packb([self.cluster_name, self.cmd_port, ips]),
                    (self.mcast_group, self.mcast_port))
        sock.close()

    def get_nodes(self):
        """
        Provides a set of node ids.
        :return: A set of node ids.
        """
        return self.nodes

    def get_health(self):
        return self.health

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

    def process_node_tasks(self):
        """
        Performs local node processing tasks.

        :return:
        """
        self._beat_heart()

        # Perform local stability tasks
        self._check_topology_stability()

    def process_leader_tasks(self):
        """
        Performs processing of tasks delegated to the leader.
        :return: None
        """
        # Perform leader tasks if I am the leader.
        if self.leader_node_id != self.node_id:
            return

        self.current_cluster_time += 1

    def process(self):
        """
        Performs one loop of the poll process for network I/O. This may wait up to 1 second before returning if
        there is no pending activity.
        :return: None
        """
        self.process_node_tasks()

        # Poll presence socket
        while True:
            events = self.presence_poller.poll(50)
            for sock, event in events:
                self._process_presence(self.presence_socket, [event])
            if not events:
                break

        # Poll ZMQ sockets
        while True:
            events = self.poller.poll(timeout=50)
            for sock, event in events:
                self._process_publication(sock, [event])
            if not events:
                break

        self.process_leader_tasks()

    def start_engine(self, dbe_node_id):
        engine = infinisqlmgr.engine.Configuration(dbe_node_id, self.dis)
        self.engines[dbe_node_id] = engine

    def run_management_only(self):
        """
        The main run loop for the management process. Sets a signal handler so that the process can be
        stopped by sending SIGTERM. You can also call the "stop()" function from inside the same process
        to stop the management server.

        This version of the run loop does not provide the web configuration front-end.

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

    def run(self):
        """
        The main run loop for the management process. Sets a signal handler so that the process can be
        stopped by sending SIGTERM. You can also call the "stop()" function from inside the same process
        to stop the management server.

        This version of the run loop does provides the web configuration front-end. The web front-end is a
        RESTful API that provides status and configuration command and control interfaces. See api_client
        for use.

        :return: 0 on success, nonzero for error conditions.
        """
        Controller.instance = self

        zmq_ioloop.install()
        signal.signal(signal.SIGTERM, self._stop_signal_handler)

        # Setup the web application
        self.application = tornado.web.Application(api.handlers, gzip=True)
        self.application.listen(self.config.get("management", "configuration_port"),
                                address=self.config.get("management", "configuration_ip"))

        # Setup handlers to care for the management tasks.
        announce_timer = tornado.ioloop.PeriodicCallback(self.announce_presence, 1000)
        management_timer = tornado.ioloop.PeriodicCallback(self.process_node_tasks, 100)
        leader_timer = tornado.ioloop.PeriodicCallback(self.process_leader_tasks, 100)
        announce_timer.start()
        management_timer.start()
        leader_timer.start()

        instance = zmq_ioloop.ZMQIOLoop.instance()
        instance.add_handler(self.presence_socket.fileno(), self._process_presence, zmq_ioloop.ZMQIOLoop.READ)

        # Start the I/O loop
        logging.info("Started management process, announcing on %s:%s, configuration=%s:%s, command=%s:%s",
                     self.mcast_group, self.mcast_port,
                     self.config.get("management", "configuration_ip"),
                     self.config.get("management", "configuration_port"),
                     self.config.get("management", "management_ip"),
                     self.config.get("management", "management_port")
        )
        instance.start()
        announce_timer.stop()
        management_timer.stop()
        leader_timer.stop()

        # Unregister the signal handler and exit.
        signal.signal(signal.SIGTERM, signal.SIG_DFL)
        logging.info("Stopped management process.")
        return 0

    def shutdown(self):
        """
        Shuts down this management node, releases all resources.
        :return: None
        """
        self.stop()
        self.presence_poller.unregister(self.presence_socket)
        for sock in self.sub_sockets.values():
            self.poller.unregister(sock)

        self.presence_socket.close()
        self.cmd_socket.close()
        for sock in self.sub_sockets.values():
            sock.close()
