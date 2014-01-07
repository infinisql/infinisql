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

#include <cstdlib>
#include <string>
#include <fstream>
#include <ios>
#include <unistd.h>
#include <ostream>
#include <cstring>
#include <iostream>
#include <cerrno>
#include <cassert>

using namespace std;
using std::string;

extern std::ofstream logfile;
extern string zmqsocket;
extern void *zmqcontext;

#endif // INFINISQLMAIN_H
