__author__ = 'Christopher Nelson'

import asyncore
import fcntl
import logging
import signal
import socket
import struct


import msgpack
import zmq

class PresenceHandler(asyncore.dispatcher):
    def __init__(self, sock, controller):
        asyncore.dispatcher.__init__(sock)
        self.controller = controller

    def handle_read(self):
        data = self.recv(8192)
        remote_ip, cluster_name = msgpack.unpackb(data)
        self.controller.add_node(cluster_name, remote_ip)


class PresenceServer(asyncore.dispatcher):
    def __init__(self, host, port, controller):
        asyncore.dispatcher.__init__(self)
        self.controller = controller
        self.create_socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.set_reuse_addr()

        self.bind((host, port))
        multicast_req = struct.pack("=4sl", socket.inet_aton(host), socket.INADDR_ANY)
        self.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, multicast_req)

        #self.listen(5)

    def handle_accepted(self, sock, addr):
        handler = PresenceHandler(sock, self.controller)


class Controller(object):
    def __init__(self, cluster_name, mcast_group="224.0.0.1", mcast_port=21001, cmd_port=21000):
        self.ctx = zmq.Context.instance()
        self.poller = zmq.Poller()

        self.cmd_socket = self.ctx.socket(zmq.PUB)
        self.sub_sockets = []

        self.presence_recv_socket = PresenceServer(mcast_group, mcast_port, self)

        self.cluster_name = cluster_name
        self.mcast_group = mcast_group
        self.mcast_port = mcast_port
        self.cmd_port = cmd_port

        # Flag is set to False when it's time to stop.
        self.keep_going = True

    def _configure_pub_socket(self):
        self.cmd_socket.bind("tcp://*:%d" % self.cmd_port)

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
            sock.close()
            return None
        ip = struct.unpack('16sH2x4s8x', res)[2]
        return socket.inet_ntoa(ip)

    def add_node(self, cluster_name, remote_ip):
        """
        Processes a presence announcement on the multicast socket. This will register
        a new sub socket in the poller if the announcement contains the expected
        signature.

        :param cluster_name: The name of the cluster the node says it is in.
        :param remote_ip: The remote ip to connect to.
        """
        # Drop the announcement if the cluster name is not the one we're looking for.
        if cluster_name != self.cluster_name:
            return

        # Subscribe to the remote management node.
        remote_pub = "tcp://%s:%d" % (remote_ip, self.cmd_port)
        sub_sock = self.ctx.socket(zmq.SUB)
        sub_sock.connect(remote_pub)

        # Register the new subscription socket.
        self.poller.register(sub_sock, flags=zmq.POLLIN)

    def _process_publication(self, sock):
        pass

    def announce_presence(self):
        """
        Send a presence announcement to the local network segment.
        """
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 8)
        sock.sendto(msgpack.packb([self._get_ip(), self.cluster_name]), (self.mcast_group, self.mcast_port))
        sock.close()

    def _stop_signal_handler(self, signum, frame):
        logging.debug("caught termination signal: signal %d", signum)
        self.stop()

    def stop(self):
        """
        Sets the flag to stop the management process. This may take a second to react.
        :return: None
        """
        logging.info("Setting stop flag for management process.")
        self.keep_going = False

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
            asyncore.loop(timeout=500)
            events = self.poller.poll(timeout=500)
            for sock, event in events:
                self._process_publication(sock)
        logging.info("Stopped management process.")
        signal.signal(signal.SIGTERM, signal.SIG_DFL)
        return 0
