__author__ = 'Christopher Nelson'

import unittest
from infinisqlmgr import engine

dist_dir = "/home/christopher/workspace/infinisql/dist"

class TestDatabaseEngine(unittest.TestCase):
    def setUp(self):
        self.c = engine.Configuration(100, dist_dir, "*", 19897)
        self.c.start()

    def tearDown(self):
        self.c.stop()

    def test_flap_engine(self):
        for i in range(0, 10):
            self.c.process()
        self.assertNotEqual(0, len(self.c.state.actors))
