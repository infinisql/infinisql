#!/usr/bin/env sh
scons -Q && LD_LIBRARY_PATH=deps/lib tests/test

