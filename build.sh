#!/usr/bin/env sh
scons && LD_LIBRARY_PATH=deps/lib tests/test

