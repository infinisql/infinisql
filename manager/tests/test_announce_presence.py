__author__ = 'Christopher Nelson'

import asyncore
import logging
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "../"))

from infinisqlmgr import management

logging.basicConfig(level=logging.DEBUG)

c = management.Controller("default_cluster", cmd_port=11000)
c.announce_presence()
c.process()
