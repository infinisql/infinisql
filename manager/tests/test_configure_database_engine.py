__author__ = 'Christopher Nelson'

import unittest
from infinisqlmgr import engine

dist_dir = "/home/christopher/workspace/infinisql/dist"

class TestDatabaseEngine(unittest.TestCase):
    def test_flap_engine(self):
        c = engine.Configuration("n1", dist_dir, "127.0.0.1", 30000)
        c.start()
        c.stop()
