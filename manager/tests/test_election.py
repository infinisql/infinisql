__author__ = 'christopher'

import os
import sys
import unittest

sys.path.append(os.path.join(os.path.dirname(__file__), "../"))

from infinisqlmgr.management import election

node1_id = ("10.0.0.1", 11000)
node2_id = ("10.0.0.2", 11000)
node3_id = ("10.0.0.3", 11000)

class TestElection(unittest.TestCase):
    def setUp(self):
        self.nodes = set([node1_id, node2_id, node3_id])
        self.election = election.Election(self.nodes, 10)

    def test_tally_vote(self):
        self.election.tally(node1_id, node1_id)

        self.assertFalse(self.election.ready(11))
        self.assertEqual(len(self.nodes), len(self.election.votes))
        self.assertEqual(1, len(self.election.voters))


        self.election.tally(node1_id, node2_id)
        self.assertFalse(self.election.ready(12))
        self.assertEqual(len(self.nodes), len(self.election.votes))
        self.assertEqual(2, len(self.election.voters))

    def test_election_ready(self):
        self.election.tally(node1_id, node1_id)
        self.election.tally(node1_id, node2_id)
        self.election.tally(node1_id, node3_id)

        self.assertTrue(self.election.ready(25))

    def test_get_best_candidate(self):
        self.assertEqual(node3_id, self.election.get_best_candidate())

    def test_get_winner(self):
        self.election.tally(node1_id, node1_id)
        self.election.tally(node1_id, node2_id)
        self.election.tally(node1_id, node3_id)

        self.assertTrue(self.election.ready(25))
        self.assertEqual(node1_id, self.election.get_winner())

if __name__ == '__main__':
    unittest.main()
