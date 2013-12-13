__author__ = 'Christopher Nelson'

import os
import time

import psutil

from infinisqlmgr.management.data_point import DataPoint

memory = ["total", "available", "percent", "used", "free", "active", "inactive", "buffers", "cached"]
swap = ["total", "used", "free", "percent", "sin", "sout"]
cpu = ["user", "nice", "system", "idle", "iowait", "irq", "softirq", "steal", "guest", "guest_nice"]
disk_space = ["total", "used", "free", "percent"]
disk_io = ["read_count", "write_count", "read_bytes", "write_bytes", "read_time", "write_time"]
net_io = ["bytes_sent", "bytes_recv", "packets_sent", "packets_recv", "errin", "errout", "dropin", "dropout"]

class Health(object):
    def __init__(self, node_id, data_dir):
        self.path = os.path.join(data_dir, "heartbeat", node_id[0], str(node_id[1]))
        self.node_id = node_id
        self.memory_alert = False
        self.swap_alert = False

        self.cpu_load = DataPoint(self.path, "cpu.load")
        self.mem = [DataPoint(self.path, "mem.%s" % item) for item in memory]
        self.swp = [DataPoint(self.path, "swp.%s" % item) for item in swap]
        self.cpu = [DataPoint(self.path, "cpu.%s" % item) for item in cpu]
        self.dsk_sp = {}
        self.dsk_io = {}
        self.net = {}

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

        for i,value in enumerate(psutil.swap_memory()):
            self.swp[i].update(value)

        net_io_data = psutil.net_io_counters(pernic=True)
        for name in net_io_data:
            if name not in self.net:
                self.net[name] = [DataPoint(self.path, "net.io.%s.%s" % (name,item)) for item in net_io]
            net = self.net[name]
            for i,value in enumerate(net_io_data[name]):
                net[i].update(value)

        dsk_io_data = psutil.disk_io_counters(perdisk=True)
        for name in dsk_io_data:
            if name not in self.dsk_io:
                self.dsk_io[name] = [DataPoint(self.path, "dsk.io.%s.%s" % (name,item)) for item in disk_io]
            dsk_io = self.dsk_io[name]
            for i,value in enumerate(dsk_io_data[name]):
                dsk_io[i].update(value)

        self.disk_partitions = psutil.disk_partitions()
        for disks in self.disk_partitions:
            device = disks[0].replace("/dev/", "")
            name = "-".join([el for el in device.split("/") if el])
            # Create an new set of data points if we find a new disk.
            if name not in self.dsk_sp:
                self.dsk_sp[name] = [DataPoint(self.path, "dsk.space.%s.%s" % (name,item)) for item in disk_space]
            # Find the disk we are storing data for
            dsk = self.dsk_sp[name]
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
            return self.dsk_sp[parts[1]][disk_space.index(parts[2])]

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

    def is_healthy(self, dp, seconds, has_alert, low_water, high_water):
        """
        Checks to see if the given metric has been healthy over the last 'seconds' seconds. If 'has_alert' is true then
        the metric must be lower than 'low_water', otherwise it must be lower than 'high_water'. Returns True if it's
        healthy, false if it's not.

        :param dp: The metric to check.
        :param seconds: The number of seconds of history to evaluate.
        :param has_alert: True if the metric was previously in an unhealthy state.
        :param low_water: The low water mark if has_alert is True.
        :param high_water:  The high water mark.
        :return: True if the metric is healthy, False otherwise.
        """
        percent_used = self.avg(dp, time.time() - seconds)
        if has_alert:
            return percent_used < low_water
        return percent_used < high_water

    def is_memory_healthy(self, seconds, low_water, high_water):
        """
        Checks to see if memory is in a healthy state. This is a convenience for is_healthy("mem.percent")

        :param seconds: The number of seconds of history to check for health.
        :param low_water: The low water level in memory percent used.
        :param high_water: The high water level in memory percent used.
        :return: True if memory is healthy, False otherwise.
        """
        self.memory_alert = not self.is_healthy("mem.percent", seconds, self.memory_alert, low_water, high_water)
        return not self.memory_alert

    def is_swap_healthy(self, seconds, low_water, high_water):
        """
        Checks to see if swap is in a healthy state. This is a convenience for is_healthy("swp.percent")

        :param seconds: The number of seconds of history to check for health.
        :param low_water: The low water level in swap percent used.
        :param high_water: The high water level in swap percent used.
        :return: True if swap is healthy, False otherwise.
        """
        self.swap_alert = not self.is_healthy("swp.percent", seconds, self.swap_alert, low_water, high_water)
        return not self.swap_alert

