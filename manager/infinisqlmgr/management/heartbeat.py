__author__ = 'Christopher Nelson'

import msgpack
import os
import psutil

class Heartbeat(object):
    # The amount of node ticks to not hear a heartbeat before we assume that node
    # has been partitioned from us.
    partition_threshold = 50

    def __init__(self, node_id, current_node_time, data_dir, history=1000):
        self.path = os.path.join(data_dir, "heartbeat", node_id[0], str(node_id[1]))
        if not os.path.exists(self.path):
            whisper.create(self.path, self.archives)

        self.history = history
        self.node_id = node_id
        self.last_heartbeat = current_node_time
        self.serialized = None

    def _save(self, current_node_time):
        with self.db.begin(write=True) as txn:
            txn.put(current_node_time, self.serialized)
            self._garbage_collect(txn.cursor())

    def _serialize(self):
        return msgpack.packb((self.cpu_percent, self.cpu_times, self.virtual_memory,
                              self.disk_partitions, self.disk_usage))

    def capture(self, current_node_time):
        """
        Captures stats of the local system. This should only be run on the heartbeat representing the local node. For
        updating stats of nodes representing remote systems use "update".
        :return: None
        """
        self.last_heartbeat = current_node_time
        self.cpu_percent = psutil.cpu_percent(interval=1)
        self.cpu_times = psutil.cpu_times()
        self.virtual_memory = psutil.virtual_memory()
        self.disk_partitions = psutil.disk_partitions()
        for disks in self.disk_partitions:
            self.disk_usage[disks[1]] = psutil.disk_usage(disks[1])

        # Cache the serialized data.
        self.serialized = self._serialize()
        self._save(current_node_time)


    def update(self, m, current_node_time):
        """
        Updates the heartbeat values associated with this heartbeat node.
        :param m: The packed message containing the updated stats.
        :param current_node_time: The node time when this message was received.
        :return: None
        """
        self.last_heartbeat = current_node_time
        self.cpu_percent, self.cpu_times,
        self.virtual_memory, self.disk_partitions,
        self.disk_usage = msgpack.unpackb(m)

        # Cache and store the serialized data.
        self.serialized = m
        self._save(current_node_time)

    def serialize(self):
        return self.serialized

    def is_partitioned(self, current_node_time):
        """
        Indicates if this node has been partitioned from the container node.
        :param current_node_time: The current node time.
        :return: True if the node represented by this Heartbeat object is partitioned, else False
        """
        return current_node_time - self.last_heartbeat > self.partition_threshold


