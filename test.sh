#!/usr/bin/env sh

scons -Q asan=1 verbose=1 && mv -f tests/test . && ASAN_OPTIONS="detect_leaks=1" ASAN_SYMBOLIZER_PATH=`which llvm-symbolizer-3.4` LD_LIBRARY_PATH=deps/lib ./test

