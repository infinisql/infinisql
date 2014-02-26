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
 * @file   User.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Mon Jan 13 14:42:44 2014
 * 
 * @brief  user record
 */

#include "User.h"
#include "Catalog.h"

User::User()
{
    
}

User::User(const User &orig) : Metadata (orig)
{
    cp(orig);
}

User &User::operator= (const User &orig)
{
    (Metadata)*this=Metadata(orig);
    cp(orig);
    return *this;
}

void User::cp(const User &orig)
{
    password=orig.password;
}

User::~User()
{
    
}

void ser(const User &d, Serdes &output)
{
    ser((const Metadata &)d, output);
    ser(d.password, output);
}

size_t sersize(const User &d)
{
    return sersize((const Metadata &)d) + sersize(d.password);
}

void des(Serdes &input, User &d)
{
    des(input, (Metadata &)d);
    des(input, d.password);
}
