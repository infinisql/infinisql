__author__ = 'Christopher Nelson'

import os
import random
import sys
import time
import unittest

sys.path.append(os.path.join(os.path.dirname(__file__), "../"))

from infinisqlmgr.management import whisper
from infinisqlmgr.management.metric import Metric

node1_id = ("10.0.0.1", 11000)
node2_id = ("10.0.0.2", 11000)
node3_id = ("10.0.0.3", 11000)

class TestMetric(unittest.TestCase):
    def setUp(self):
        self.dp1 = Metric("/tmp/dp", "test.cpu")
        self.dp2 = Metric("/tmp/dp", "test.memory")

    def tearDown(self):
        self.dp1.purge()
        self.dp2.purge()

    def test_validate_archives(self):
        self.assertTrue(whisper.validateArchiveList(self.dp1.archives))

    def test_update(self):
        start = int(time.time())
        for i in range(start-1000,start,1):
            self.dp1.update(random.randint(0, 100), i)

        data_range, values = self.dp1.fetch(start-1000, start)
        self.assertEqual(1000, len(values))
        self.assertEqual(data_range[0], start-999)
        self.assertEqual(data_range[1], start+1)

    def test_fetch(self):
        points = [random.randint(0, 100) for i in range(0,1000)]
        start = int(time.time())
        index = 0
        for i in range(start-1000,start,1):
            self.dp1.update(points[index], i)
            index+=1

        data_range, values = self.dp1.fetch(start-1000, start)
        self.assertEqual(points[1:], values[0:-1])
        self.assertEqual(sum(points[1:]), sum(values[0:-1]))


if __name__ == "__main__":
    unittest.main()



