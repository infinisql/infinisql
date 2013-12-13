__author__ = 'Christopher Nelson'

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(__file__), "../"))

from infinisqlmgr import management

class TestAnnouncePresence(unittest.TestCase):
    def setUp(self):
        self.c1 = management.Controller("default_cluster", cmd_port=11000)
        self.c2 = management.Controller("default_cluster", cmd_port=12000)

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
        ip = self.c1._get_ip()

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
