#!/usr/bin/env python3
import pip

cmd = ["install", "--upgrade"]
libs = ["parsedatetime", "pyzmq", "psutil", "lmdb", "msgpack-python", "tornado"]

pip.main(cmd + libs)

