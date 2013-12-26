__author__ = 'Christopher Nelson'

import logging
logging.basicConfig(level=logging.DEBUG)

import unittest
from infinisqlmgr import management, config

class SimpleConfig(object):
    def __init__(self):
        self.dist_dir = "/tmp/announce"

class TestAnnouncePresence(unittest.TestCase):
    def setUp(self):
        self.cfg1 = config.Configuration(SimpleConfig())
        self.cfg2 = config.Configuration(SimpleConfig())

        self.cfg1.config.set("management", "management_port", "21000")
        self.cfg1.config.set("management", "management_port", "22000")

        self.c1 = management.Controller(self.cfg1)
        self.c2 = management.Controller(self.cfg2)

    def tearDown(self):
        self.c1.shutdown()
        self.c2.shutdown()

    def test_announce_ignores_self(self):
        self.c1.announce_presence()
        self.c1.process()
        self.c2.process()

        self.assertEqual(1, len(self.c1.get_nodes()))
        self.assertEqual(2, len(self.c2.get_nodes()))

    def test_mutual_announce(self):
        ip = self.cfg1.interfaces()

        self.c1.announce_presence()
        self.c2.announce_presence()

        for i in range(0, 2):
            self.c1.process()
            self.c2.process()

        c1_nodes = self.c1.get_nodes()
        c2_nodes = self.c2.get_nodes()

        self.assertEqual(2, len(c1_nodes))
        self.assertEqual(2, len(c2_nodes))

        self.assertTrue((ip, 11000) in c2_nodes)
        self.assertTrue((ip, 12000) in c1_nodes)


if __name__ == "__main__":
    unittest.main()
