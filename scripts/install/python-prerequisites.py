#!/usr/bin/env python3
import pip

cmd = ["install", "--upgrade"]
libs = ["pyzmq", "psutil", "lmdb", "msgpack-python", "tornado"]

pip.main(cmd + libs)

