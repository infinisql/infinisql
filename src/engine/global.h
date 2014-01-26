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
#include <unordered_map>
#include <sys/stat.h>
#include <unistd.h>
#include <lmdb.h>
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <endian.h>
#include <sys/epoll.h>

using namespace std;
using std::string;

extern std::ofstream logfile;

#define LOG(...) logfile << __FILE__ << " " << __LINE__ << " errno: " << errno << " '" << strerror(errno) << "' " << __VA_ARGS__ << std::endl

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

/* InfiniSQL headers that most, or all parts of the project need */
#include "Serdes.h"
#include "Message.h"

#endif // INFINISQLGLOBAL_H
