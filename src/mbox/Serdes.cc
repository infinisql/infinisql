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
 * @file   Serdes.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Jan  7 17:16:03 2014
 * 
 * @brief  serialization and deserialization for POD and generic data types
 */

#include "Serdes.h"

Serdes::Serdes() : isreadonly(false), pos(0), val({0, nullptr})
{
    
}

Serdes::Serdes(size_t mv_sizearg) : isreadonly(false), pos(0)
{
    val.mv_data=new (std::nothrow) char[mv_sizearg];
    if (val.mv_data != nullptr)
    {
        val.mv_size=mv_sizearg;
    }
    else
    {
        val.mv_size=0;
        // this should result in node killing itself
    }
}

Serdes::Serdes(MDB_val &valarg) : isreadonly(true), pos(0), val(valarg)
{
    
}

Serdes::Serdes(const char *data, size_t size) : isreadonly(false), pos(0)
{
    val.mv_size=size;
    val.mv_data=new (std::nothrow) char[val.mv_size];
    if (val.mv_data==nullptr)
    {
        // this should result in node killing itself
        return;
    }
    memcpy(val.mv_data, data, val.mv_size);    
}

Serdes::~Serdes()
{
    if (isreadonly==false)
    {
        delete [] (const char *)val.mv_data;
    }
}

void Serdes::rewind()
{
    pos=0;
}

void Serdes::ffwd()
{
    pos=val.mv_size ? val.mv_size-1 : 0;
}

bool Serdes::isbegin()
{
    return pos==0 ? true : false;
}

bool Serdes::isend()
{
    if (val.mv_size)
    {
        return pos==val.mv_size-1 ? true : false;
    }
    return pos==0 ? true : false;
}

// void Serdes::ser(const decimal &d)
// {
// 	ser(d.to_string());
// }

// void Serdes::des(decimal *&d)
// {
// 	std::string st;
// 	des(st);
//     d = new decimal{st};
// }

void ser(int8_t d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(int8_t d)
{
    return sizeof(d);
}

void des(Serdes &input, int8_t &d)
{
    despod(input, d);
}

void ser(int16_t d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(int16_t d)
{
    return sizeof(d);
}

void des(Serdes &input, int16_t &d)
{
    despod(input, d);
}

void ser(int32_t d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(int32_t d)
{
    return sizeof(d);
}

void des(Serdes &input, int32_t &d)
{
    despod(input, d);
}

void ser(int64_t d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(int64_t d)
{
    return sizeof(d);
}

void des(Serdes &input, int64_t &d)
{
    despod(input, d);
}

void ser(uint8_t d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(uint8_t d)
{
    return sizeof(d);
}

void des(Serdes &input, uint8_t &d)
{
    despod(input, d);
}

void ser(uint16_t d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(uint16_t d)
{
    return sizeof(d);
}

void des(Serdes &input, uint16_t &d)
{
    despod(input, d);
}

void ser(uint32_t d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(uint32_t d)
{
    return sizeof(d);
}

void des(Serdes &input, uint32_t &d)
{
    despod(input, d);
}

void ser(uint64_t d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(uint64_t d)
{
    return sizeof(d);
}

void des(Serdes &input, uint64_t &d)
{
    despod(input, d);
}

void ser(float d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(float d)
{
    return sizeof(d);
}

void des(Serdes &input, float &d)
{
    despod(input, d);
}

void ser(double d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(double d)
{
    return sizeof(d);
}

void des(Serdes &input, double &d)
{
    despod(input, d);
}

void ser(char d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(char d)
{
    return sizeof(d);
}

void des(Serdes &input, char &d)
{
    despod(input, d);
}

void ser(bool d, Serdes &output)
{
    serpod(d, output);
}

size_t sersize(bool d)
{
    return sizeof(d);
}

void des(Serdes &input, bool &d)
{
    despod(input, d);
}

void ser(const std::string &d, Serdes &output)
{
    size_t s=d.size();
    ser(s, output);
    memcpy((char *)output.val.mv_data+output.pos, d.c_str(), s);
    output.pos += s;
}

size_t sersize(const std::string &d)
{
    return sizeof(size_t) + d.size();
}

void des(Serdes &input, std::string &d)
{
    size_t s;
    des(input, s);
    d.assign((const char *)input.val.mv_data+input.pos, s);
    input.pos += s;
}

void ser(const std::string &d, size_t dsize, Serdes &output)
{
    memcpy((char *)output.val.mv_data+output.pos, d.c_str(), dsize);
    output.pos += dsize;
}

void des(Serdes &input, std::string &d, size_t dsize)
{
    d.assign((const char *)input.val.mv_data+input.pos, dsize);
    input.pos += dsize;
}

void ser(const void *d, size_t dsize, Serdes &output)
{
    memcpy((char *)output.val.mv_data+output.pos, d, dsize);
    output.pos += dsize;
}

void des(Serdes &input, void *d, size_t dsize)
{
    memcpy(d, (char *)input.val.mv_data+input.pos, dsize);
    input.pos += dsize;
}
