__author__ = 'Christopher Nelson'

import unittest
from infinisqlmgr import engine, config

class SimpleConfig(object):
    def __init__(self, dist_dir):
        self.dist_dir = dist_dir

dist_dir = "/home/christopher/workspace/infinisql/dist"

class TestDatabaseEngine(unittest.TestCase):
    def setUp(self):
        #self.c = config.Configuration(SimpleConfig(dist_dir))
        #self.e = engine.Configuration(("127.0.0.1", 11000), self.c)
        #self.e.start()
        pass

    def tearDown(self):
        #self.e.stop()
        pass

    def test_flap_engine(self):
        #for i in range(0, 10):
        #    self.e.process()
        #self.assertNotEqual(0, len(self.c.state.actors))
        pass
