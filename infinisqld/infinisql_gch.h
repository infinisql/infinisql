/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL(tm).
 
 * InfiniSQL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InfiniSQL. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INFINISQLGCH_H
#define INFINISQLGCH_H

// 3rd party, non-sys
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <msgpack.hpp>
#include <cryptopp/sha.h>
#include <pcrecpp.h>
#include <zmq.h>
#include "infinisql_spooky.h"
#include <lz4.h>

// sys C++
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <ctime>
#include <utility>
#include <algorithm>
#include <sstream>
#include <stack>
// sys C
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <endian.h>
#include <sys/epoll.h>
#include <poll.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dlfcn.h>
#include <mcheck.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sched.h>

// project headers
#include "infinisql_Mbox.h"
#include "infinisql_defs.h"

#endif // INFINISQLGCH_H
