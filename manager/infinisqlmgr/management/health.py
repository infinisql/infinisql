__author__ = 'Christopher Nelson'

import os
import psutil

from infinisqlmgr.management.data_point import DataPoint

memory = ["total", "available", "percent", "used", "free", "active", "inactive", "buffers", "cached"]
cpu = ["user", "nice", "system", "idle", "iowait", "irq", "softirq", "steal", "guest", "guest_nice"]
disk = ["total", "used", "free", "percent"]

class Health(object):
    def __init__(self, node_id, data_dir):
        self.path = os.path.join(data_dir, "heartbeat", node_id[0], str(node_id[1]))
        self.node_id = node_id

        self.cpu_load = DataPoint(self.path, "cpu.load")
        self.mem = [DataPoint(self.path, "mem.%s" % item) for item in memory]
        self.cpu = [DataPoint(self.path, "cpu.%s" % item) for item in cpu]
        self.dsk = {}

    def capture(self):
        """
        Captures stats of the local system.
        :return: None
        """
        self.cpu_load.update(psutil.cpu_percent(interval=None))
        for i, value in enumerate(psutil.cpu_times()):
            self.cpu[i].update(value)

        for i,value in enumerate(psutil.virtual_memory()):
            self.mem[i].update(value)

        self.disk_partitions = psutil.disk_partitions()
        for disks in self.disk_partitions:
            name = "root" if disks[1] == "/" else "-".join([el for el in disks[1].split("/") if el])
            # Create an new set of data points if we find a new disk.
            if name not in self.dsk:
                self.dsk[name] = [DataPoint(self.path, "dsk.%s.%s" % (name,item)) for item in disk]
            # Find the disk we are storing data for
            dsk = self.dsk[name]
            # Update the disk stats
            for i, value in enumerate(psutil.disk_usage(disks[1])):
                dsk[i].update(value)

    def lookup(self, name):
        parts = name.split(".")
        if parts[0] == "cpu":
            if parts[1] == "load":
                return self.cpu_load
            return self.cpu[cpu.index(parts[1])]
        elif parts[0] == "mem":
            return self.mem[memory.index(parts[1])]
        elif parts[0] == "dsk":
            return self.dsk[parts[1]][disk.index(parts[2])]

        return None

    def min(self, dp, from_time, until_time=None):
        if type(dp) == type(str()):
            dp = self.lookup(dp)

        return min([x for x in dp.fetch(from_time, until_time)[1] if x is not None])

    def max(self, dp, from_time, until_time=None):
        if type(dp) == type(str()):
            dp = self.lookup(dp)

        return max([x for x in dp.fetch(from_time, until_time)[1] if x is not None])

    def avg(self, dp, from_time, until_time=None):
        if type(dp) == type(str()):
            dp = self.lookup(dp)

        values = [x for x in dp.fetch(from_time, until_time)[1] if x is not None]
        return sum(values) / len(values)


