__author__ = 'Christopher Nelson'

import time
import unittest

from infinisqlmgr.management.health import Health

node1_id = ("10.0.0.1", 11000)
node2_id = ("10.0.0.2", 11000)
node3_id = ("10.0.0.3", 11000)

class TestHealth(unittest.TestCase):
    def test_create(self):
        h = Health(node1_id, "/tmp/hb")

    def test_capture_heartbeat(self):
        h = Health(node1_id, "/tmp/hb")
        h.capture()

    def test_min(self):
        h = Health(node1_id, "/tmp/hb")

        for i in range(0, 1000):
            h.capture()

        self.assertLessEqual(0.0, h.min("cpu.load", time.time()-100))

    def test_max(self):
        h = Health(node1_id,  "/tmp/hb")

        for i in range(0, 1000):
            h.capture()

        self.assertLess(0, h.max("cpu.load", time.time()-100))

    def test_avg(self):
        h = Health(node1_id, "/tmp/hb")

        for i in range(0, 1000):
            h.capture()

        self.assertLess(0, h.avg("cpu.load", time.time()-100))


if __name__ == "__main__":
    unittest.main()
