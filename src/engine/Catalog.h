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
 * @file   Catalog.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 08:05:51 2014
 * 
 * @brief  catalog is a collection of schemata and users
 */

#ifndef INFINISQLCATALOG_H
#define INFINISQLCATALOG_H

#include "Metadata.h"

class User;
class Schema;
class Table;
class Index;

class Catalog : public Metadata
{
public:
    Catalog();
    Catalog(const Catalog &orig);
    Catalog &operator= (const Catalog &orig);
    /** 
     * @brief copy sufficient for reproduction elsewhere
     *
     * requires post-processing for destination actors' pointers to related
     * objects
     *
     * @param orig 
     */
    void cp(const Catalog &orig);
    
    ~Catalog();

    /** 
     * @brief create and open LMDB environment
     *
     * @param path filesystem path for environment
     *
     * @return 0 for success, otherwise result from mdb_env_create() or
     * mdb_env_open()
     */
    int openEnvironment(std::string path);
    /** 
     * @brief close LMDB environment
     *
     * from the FM: "All transactions, databases, and cursors must already be
     * closed before calling this function. Attempts to use any such handles
     * after calling this function will cause a SIGSEGV."
     */
    void closeEnvironment();
    /** 
     * @brief delete data and lock files in LMDB environment directory
     *
     * the environment must not be open
     *
     * @param path filesystem path of LMDB environment
     *
     * @return status from remove() for either of the files
     */
    int deleteEnvironment(std::string path);
    
    int16_t nextuserid;

    std::unordered_map<std::string, int16_t> userName2Id;
    std::unordered_map<std::string, int16_t> schemaName2Id;
    
    std::unordered_map<int16_t, User *> userid2User;
};

void ser(const Catalog &d, Serdes &output);
size_t sersize(const Catalog &d);
void des(Serdes &input, Catalog &d);

#endif // INFINISQLCATALOG_H
