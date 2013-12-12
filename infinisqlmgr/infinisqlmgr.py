#! /usr/bin/env python2

# Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
# All rights reserved. No warranty, explicit or implicit, provided.
#
# This file is part of InfiniSQL(tm).
 
# InfiniSQL is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3
# as published by the Free Software Foundation.
#
# InfiniSQL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with InfiniSQL. If not, see <http://www.gnu.org/licenses/>.

import cfg
import os

os.environ["LD_PRELOAD"] = "/usr/local/lib/libllalloc.so.1.4"

# launch and configure each node
for id in sorted(cfg.nodes):
#  print "enable " + str(id) + ", " + str(cfg.nodes[id].id)
  cfg.nodes[id].enable()
#  n.enable()

