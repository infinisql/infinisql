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
 * @file   Serdes.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Jan  7 17:09:56 2014
 * 
 * @brief  serialization and deserialization for POD and generic data types
 */

#ifndef INFINISQLSERDES_H
#define INFINISALSERDES_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

class Serdes
{
public:
    Serdes();
    ~Serdes();

    // pods
    static size_t ser(int16_t d, char *output);
    static size_t sersize(int16_t d);
    static size_t des(char *input, int16_t *d);
    static size_t ser(int32_t d, char *output);
    static size_t sersize(int32_t d);
    static size_t des(char *input, int32_t *d);
    static size_t ser(int64_t d, char *output);
    static size_t sersize(int64_t d);
    static size_t des(char *input, int64_t *d);
    static size_t ser(char d, char *output);
    static size_t sersize(char d);
    static size_t des(char *input, char *d);
    static size_t ser(bool d, char *output);
    static size_t sersize(bool d);
    static size_t des(char *input, bool *d);

    // containers
    static size_t ser(std::string &d, char *output);
    static size_t sersize(std::string &d);
    static size_t des(char *input, std::string &d);    
};

#endif // INFINISQLSERDES_H
