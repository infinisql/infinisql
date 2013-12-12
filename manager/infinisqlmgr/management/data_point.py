__author__ = 'Christopher Nelson'

import os
from infinisqlmgr.management import whisper

class DataPoint(object):
    # The default storage roll-up: one hour of one-second resolution ticks, one day of one-minute resolution ticks,
    # 30 days of one-hour resolution ticks, five years of one-day resolution ticks
    archives = [(1, 3600), (60, 1440), (3600, 720), (86400, 1825)]

    def __init__(self, base_data_dir, data_name):
        data_file = os.path.join(base_data_dir, data_name.replace(".", "/")) + ".dp"
        data_dir = os.path.dirname(data_file)
        if not os.path.exists(data_dir):
            os.makedirs(data_dir)
        if not os.path.exists(data_file):
            whisper.create(data_file, self.archives)
        self.data_file = data_file

    def purge(self):
        os.unlink(self.data_file)
        whisper.create(self.data_file, self.archives)

    def update(self, value, timestamp=None):
        whisper.update(self.data_file, value, timestamp)

    def fetch(self, from_time, until_time=None):
        return whisper.fetch(self.data_file, from_time, until_time)
