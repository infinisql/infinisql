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

__author__ = 'Christopher Nelson'

import apt
import sys

required_packages = ["binutils", "g++", "flex", "bison",
                     "make", "autoconf", "libtool",
                     "libboost-dev",
                     "libmsgpack-dev", "libreadline-dev",
                     "libcrypto++-dev", "libzmq3-dev", "libpcre3-dev",
                     "libreadline-dev", "liblz4-dev", "zlib1g-dev",
                     "python-dev", "python-pip", "msgpack-python",
                     "python-zmq"
                     ]
print ("Reading and updating package cache...")
c=apt.cache.Cache()
c.update(fetch_progress=apt.progress.text.AcquireProgress())
c.open(progress=apt.progress.text.OpProgress())

print("Checking for required packages...")
to_install = []
for pkg in required_packages:
    p = c[pkg]
    if not p.is_installed:
        to_install.append(p)
        p.mark_install()

if len(to_install)==0:
    print("All required packages are already installed.")
    sys.exit(0)

print("Installing packages...")
c.commit(fetch_progress=apt.progress.text.AcquireProgress(),
         install_progress=apt.progress.base.InstallProgress())
print("System is now up to date.")
