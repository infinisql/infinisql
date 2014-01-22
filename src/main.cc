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

/**
 * @file   main.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan  6 11:25:19 2014
 * 
 * @brief  main() and global functions
 */

/** @mainpage InfiniSQL(tm)
 * This is the doxygen-generated documentation for InfiniSQL's C++ source code.
 * User documentation, reference, FAQ, how to obtain the source code, contacts
 * and other information is available at the project's main website:
 * http://www.infinisql.org
 */

#include "main.h"
#include "version.h"
#include "Lightning.h"
#include "Metadata.h"
#include "TopologyManager.h"
#include "global.h"

std::ofstream logfile;
string zmqsocket;
void *zmqcontext;

int main(int argc, char **argv)
{
    assert(sizeof(double)==8);
    string logfilename;
    int c;
    long nodeid=0; // this needs to be part of global topology

    while ((c = getopt(argc, argv, "l:m:n:hv")) != -1)
    {
        switch (c)
        {
        case 'm':
            zmqsocket.assign("tcp://");
            zmqsocket.append(optarg, strlen(optarg));
            break;

        case 'l':
            logfilename.assign(optarg, strlen(optarg));
            break;

        case 'n':
            nodeid=atol(optarg);
            break;

        case 'v':
            std::cout << version << std::endl;
            exit(0);

        case 'h':
            std::cout << "-m <management ip:port> -n <nodeid>" <<
                "-l <log path/file> -v" << std::endl;
            exit(0);

        default:
            ;
        }
    }

    if (!zmqsocket.size())
    {
        std::cerr << __FILE__ << " " << __LINE__ << " management ip:port must"
                  << "be set with -m option" << std::endl;
        exit(1);
    }
    if (nodeid <= 0)
    {
        std::cerr << __FILE__ << " " << __LINE__ << " nodeid must be set >0"
                  << " with -n option" << std::endl;
        exit(1);
    }
    
    if (!logfilename.size())
    {
        logfilename.assign("/tmp/infinisqld.log");
    }
    logfile.open(logfilename.c_str(), std::ofstream::out | std::ofstream::app);
    if (logfile.fail())
    {
        char strerrorbuf[256];
        std::cerr << __FILE__ << " " << __LINE__ << " errno " << errno <<
            " " << strerror_r(errno, strerrorbuf, 256)  << std::endl;
        exit(1);
    }

    Actor::identity_s id;
    id.address.nodeid=nodeid;
    id.address.actorid=1;
    TopologyManager tm(id);
    tm();

    return 0;
}
