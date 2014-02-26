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
 * @file   User.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 14:40:39 2014
 * 
 * @brief  user record
 */

#ifndef INFINISQLUSER_H
#define INFINISQLUSER_H

#include "Metadata.h"

class User : public Metadata
{
public:
    User();
    User(const User &orig);
    User &operator= (const User &orig);
    /** 
     * @brief copy sufficient for reproduction elsewhere
     *
     * requires post-processing for destination actors' pointers to related
     * objects
     *
     * @param orig 
     */
    void cp(const User &orig);
    ~User();
    
    std::string password;
};

void ser(const User &d, Serdes &output);
size_t sersize(const User &d);
void des(Serdes &input, User &d);

#endif // INFINISQLUSER_H
