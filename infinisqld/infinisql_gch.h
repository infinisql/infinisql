/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL (tm). It is available either under the
 * GNU Affero Public License or under a commercial license. Contact the
 * copyright holder for information about a commercial license if terms
 * of the GNU Affero Public License do not suit you.
 *
 * This copy of InfiniSQL is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with InfiniSQL. It should be in the top level of the source
 * directory in a file entitled "COPYING".
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GCH_HPP
#define GCH_HPP

// 3rd party, non-sys
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
//#include "/usr/local/include/msgpack.hpp"
#include "/usr/local/include/m/msgpack.hpp"
#include <cryptopp/sha.h>
//#include <jemalloc/jemalloc.h>
#include <pcrecpp.h>
#include <zmq.h>
#include "infinisql_spooky.h"

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

// project headers
#include "infinisql_Mbox.h"
#include "infinisql_defs.h"

#endif // GCH_HPP
