__author__ = 'Christopher Nelson'

import unittest
from infinisqlmgr import management, config

import logging
logging.basicConfig(level=logging.DEBUG)

class SimpleConfig(object):
    def __init__(self, dist_dir):
        self.dist_dir = dist_dir

class TestNetworkPartition(unittest.TestCase):
    def setUp(self):
        args = object()
        co1 = config.Configuration(SimpleConfig("/tmp/i1"))
        co2 = config.Configuration(SimpleConfig("/tmp/i2"))
        co3 = config.Configuration(SimpleConfig("/tmp/i3"))

        co1.config.set("management", "management_port", "11000")
        co2.config.set("management", "management_port", "12000")
        co3.config.set("management", "management_port", "13000")

        self.c1 = management.Controller(co1)
        self.c2 = management.Controller(co2)
        self.c3 = management.Controller(co3)

        self.c1.announce_presence()
        self.c2.announce_presence()
        self.c3.announce_presence()

    def tearDown(self):
        self.c1.shutdown()
        self.c2.shutdown()
        self.c3.shutdown()

    def test_simple_partition(self):
        # Run process up to settle so that an election is forced.
        for i in range(0, 25):
            self.c1.process()
            self.c2.process()
            self.c3.process()

        self.assertEqual(3, len(self.c1.get_nodes()))
        self.assertEqual(self.c3.node_id, self.c1.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c2.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c3.leader_node_id)

        # Now simulate a partition event by not processing a single node. The other two nodes
        # should recognize that a partition has occurred. In this case we are not partitioning
        # the leader (c3), so no election should occur.
        for i in range(0, int(self.c1.node_partition_threshold*1.25)):
            self.c1.process()
            self.c3.process()

        self.assertEqual(2, len(self.c1.get_nodes()))
        self.assertIs(None, self.c1.current_election)
        self.assertEqual(self.c3.node_id, self.c1.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c2.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c3.leader_node_id)

    def test_election_partition(self):
        # Run process up to settle so that an election is forced.
        for i in range(0, 25):
            self.c1.process()
            self.c2.process()
            self.c3.process()

        self.assertEqual(3, len(self.c1.get_nodes()))
        self.assertEqual(self.c3.node_id, self.c1.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c2.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c3.leader_node_id)

        # Now simulate a more complex partition that requires an election.
        for i in range(0, int(self.c1.node_partition_threshold*1.5)):
            self.c1.process()
            self.c2.process()

        self.assertEqual(2, len(self.c1.get_nodes()))
        self.assertIs(None, self.c1.current_election)
        self.assertEqual(self.c2.node_id, self.c1.leader_node_id)
        self.assertEqual(self.c2.node_id, self.c2.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c3.leader_node_id)

    def test_heal_partition(self):
        # Run process up to settle so that an election is forced.
        for i in range(0, 25):
            self.c1.process()
            self.c2.process()
            self.c3.process()

        self.assertEqual(3, len(self.c1.get_nodes()))
        self.assertEqual(self.c3.node_id, self.c1.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c2.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c3.leader_node_id)

        # Now simulate a complex partition that requires an election.
        for i in range(0, int(self.c1.node_partition_threshold*1.5)):
            self.c1.process()
            self.c2.process()

        self.assertEqual(2, len(self.c1.get_nodes()))
        self.assertIs(None, self.c1.current_election)
        self.assertEqual(self.c2.node_id, self.c1.leader_node_id)
        self.assertEqual(self.c2.node_id, self.c2.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c3.leader_node_id)

        self.c1.announce_presence()
        self.c2.announce_presence()
        self.c3.announce_presence()

        # Finally, simulate a partition healing.
        for i in range(0, int(self.c1.node_partition_threshold*2)):
            self.c1.process()
            self.c2.process()
            self.c3.process()

        self.assertEqual(3, len(self.c1.get_nodes()))
        self.assertIs(None, self.c1.current_election)
        self.assertEqual(self.c3.node_id, self.c1.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c2.leader_node_id)
        self.assertEqual(self.c3.node_id, self.c3.leader_node_id)

if __name__ == "__main__":
    unittest.main()
