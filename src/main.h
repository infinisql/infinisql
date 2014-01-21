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

#ifndef INFINISQLMAIN_H
#define INFINISQLMAIN_H

/**
 * @file   main.h
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
#include <unordered_map>

using namespace std;
using std::string;

extern std::ofstream logfile;
extern string zmqsocket;
extern void *zmqcontext;

#define LOG(...) logfile << __FILE__ << " " << __LINE__ << ": " << __VA_ARGS__ \
    << std::endl

/* InfiniSQL headers that most, or all parts of the project need */
#include "Serdes.h"

#endif // INFINISQLMAIN_H
