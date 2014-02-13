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

#ifndef INFINISQLGLOBAL_H
#define INFINISQLGLOBAL_H

/**
 * @file   global.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan  6 14:20:07 2014
 * 
 * @brief  widely used headers and global declarations
 */

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <system_error>
#include <functional>
#include <mutex>
#include <atomic>
#include <utility>
#include <sys/stat.h>
#include <unistd.h>
#include <lmdb.h>
#include <lz4.h>
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <endian.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#ifndef __USE_POSIX
#define __USE_POSIX
#endif
#include <netdb.h>
#include <fcntl.h>

using namespace std;
using std::string;

extern std::ofstream logfile;

#define LOG(...) logfile << __FILE__ << " " << __LINE__ << " errno: " << errno << " '" << strerror(errno) << "' " << __VA_ARGS__ << std::endl
#define IBGWRCVBUF 16777216
#define NUMSOCKETS 1048576

/**
 * @todo this should go in the generated header with common symbol
 * defs between manager and daemon
 */
enum actortypes_e
{
    ACTOR_NONE=0,
    ACTOR_TOPOLOGYMGR,
    ACTOR_TRANSACTIONAGENT,
    ACTOR_PARTITION_WRITER,
    ACTOR_TRANSACTION_LOGGER,
    ACTOR_USERSCHEMAMGR,
    ACTOR_OBGATEWAY,
    ACTOR_IBGATEWAY,
    ACTOR_LISTENER,
    ACTOR_ADMIN_LISTENER
};

void setprio();

/* InfiniSQL headers that most, or all parts of the project need */
#include "../mbox/Serdes.h"
#include "../mbox/Message.h"

#endif // INFINISQLGLOBAL_H
