__author__ = 'Christopher Nelson'

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(__file__), "../"))

from infinisqlmgr.management.heartbeat import Heartbeat

node1_id = ("10.0.0.1", 11000)
node2_id = ("10.0.0.2", 11000)
node3_id = ("10.0.0.3", 11000)

class TestHeartbeat(unittest.TestCase):
    def test_create(self):
        h = Heartbeat(node1_id, 10, "/tmp/hb")

    def test_capture_heartbeat(self):
        h = Heartbeat(node1_id, 10, "/tmp/hb")
        h.capture(100)

    def test_min(self):
        h = Heartbeat(node1_id, 0, "/tmp/hb")

        for i in range(0, 1000):
            h.capture(i)

        self.assertLess(0, h.min("cpu.load"))

    def test_max(self):
        h = Heartbeat(node1_id, 0, "/tmp/hb")

        for i in range(0, 1000):
            h.capture(i)

        self.assertLess(0, h.max("cpu.load"))

    def test_avg(self):
        h = Heartbeat(node1_id, 0, "/tmp/hb")

        for i in range(0, 1000):
            h.capture(i)

        self.assertLess(0, h.avg("cpu.load"))


if __name__ == "__main__":
    unittest.main()
