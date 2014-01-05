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
        self.cfg2.config.set("management", "management_port", "22000")

        self.c1 = management.Controller(self.cfg1)
        self.c2 = management.Controller(self.cfg2)

    def tearDown(self):
        self.c1.shutdown()
        self.c2.shutdown()

    def test_announce_ignores_self(self):
        self.c1.announce_presence()
        self.c1.process()
        self.c2.process()

        self.assertEqual(2, len(self.c1.get_nodes()))
        self.assertEqual(4, len(self.c2.get_nodes()))

    def test_mutual_announce(self):
        ip_list = [interface[0].ip.compressed for interface in self.cfg1.interfaces().values()]

        self.c1.announce_presence()
        self.c2.announce_presence()

        for i in range(0, 2):
            self.c1.process()
            self.c2.process()

        c1_nodes = self.c1.get_nodes()
        c2_nodes = self.c2.get_nodes()

        self.assertEqual(4, len(c1_nodes))
        self.assertEqual(4, len(c2_nodes))

        for ip in ip_list:
            self.assertIn((ip, 21000), c2_nodes)
            self.assertIn((ip, 22000), c1_nodes)


if __name__ == "__main__":
    unittest.main()
