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

void Serdes::ser(int8_t d)
{
    if (val.mv_data==nullptr)
    {
        //LOG("shouldn't try to serialize to NULL object");
        return;
    }
    memcpy((char *)val.mv_data+pos, &d, sizeof(d));
    pos+=sersize(d);
}

size_t Serdes::sersize(int8_t d)
{
    return sizeof(d);
}

void Serdes::des(int8_t &d)
{
    memcpy(&d, (char *)val.mv_data+pos, sizeof(d));
    pos+=sersize(d);
}

void Serdes::ser(int16_t d)
{
    memcpy((char *)val.mv_data+pos, &d, sizeof(d));
    pos+=sersize(d);
}

size_t Serdes::sersize(int16_t d)
{
    return sizeof(d);
}

void Serdes::des(int16_t &d)
{
    memcpy(&d, (char *)val.mv_data+pos, sizeof(d));
    pos+=sersize(d);
}

void Serdes::ser(int32_t d)
{
    memcpy((char *)val.mv_data+pos, &d, sizeof(d));
    pos+=sersize(d);
}

size_t Serdes::sersize(int32_t d)
{
    return sizeof(d);
}

void Serdes::des(int32_t &d)
{
    memcpy(&d, (char *)val.mv_data+pos, sizeof(d));
    pos+=sersize(d);
}

void Serdes::ser(int64_t d)
{
    memcpy((char *)val.mv_data+pos, &d, sizeof(d));
    pos+=sersize(d);
}

size_t Serdes::sersize(int64_t d)
{
    return sizeof(d);
}

void Serdes::des(int64_t &d)
{
    memcpy(&d, (char *)val.mv_data+pos, sizeof(d));
    pos+=sersize(d);
}

void Serdes::ser(float d)
{
    memcpy((char *)val.mv_data+pos, &d, sizeof(d));
    pos+=sersize(d);
}

size_t Serdes::sersize(float d)
{
    return sizeof(d);
}

void Serdes::des(float &d)
{
    memcpy(&d, (char *)val.mv_data+pos, sizeof(d));
    pos+=sersize(d);
}

void Serdes::ser(double d)
{
    memcpy((char *)val.mv_data+pos, &d, sizeof(d));
    pos+=sersize(d);
}

size_t Serdes::sersize(double d)
{
    return sizeof(d);
}

void Serdes::des(double &d)
{
    memcpy(&d, (char *)val.mv_data+pos, sizeof(d));
    pos+=sersize(d);
}

void Serdes::ser(char d)
{
    memcpy((char *)val.mv_data+pos, &d, sizeof(d));
    pos+=sersize(d);
}

size_t Serdes::sersize(char d)
{
    return sizeof(d);
}

void Serdes::des(char &d)
{
    memcpy(&d, (char *)val.mv_data+pos, sizeof(d));
    pos+=sersize(d);
}

void Serdes::ser(bool d)
{
    memcpy((char *)val.mv_data+pos, &d, sizeof(d));
    pos+=sersize(d);
}

size_t Serdes::sersize(bool d)
{
    return sizeof(d);
}

void Serdes::des(bool &d)
{
    memcpy(&d, (char*)val.mv_data+pos, sizeof(d));
    pos+=sersize(d);
}

void Serdes::ser(const std::string &d)
{
    size_t s=d.size();
    ser((int64_t)s);
    ser(d, s);
}

size_t Serdes::sersize(const std::string &d)
{
    return sizeof(int64_t)+d.size();
}

void Serdes::des(std::string &d)
{
    int64_t s;
    des(s);
    d.assign((const char *)val.mv_data+pos, s);
    pos+=s;
}

void Serdes::ser(const std::string &d, size_t dsize)
{
    d.copy((char *)val.mv_data+pos, dsize, 0);
    pos+=dsize;
}

void Serdes::des(std::string *&d, size_t dsize)
{
    d = new std::string{(const char *)val.mv_data, dsize};
    if (d != nullptr)
    {
        pos+=dsize;
    }
}

void Serdes::ser(const decimal &d)
{
	ser(d.to_string());
}

void Serdes::des(decimal *&d)
{
	std::string st;
	des(st);
    d = new decimal{st};
}


void Serdes::ser(void *d, size_t dsize)
{
    memcpy((char *)val.mv_data+pos, d, dsize);
}

void Serdes::des(void *d, size_t dsize)
{
    memcpy(d, (char *)val.mv_data+pos, dsize);
    pos+=dsize;
}

void Serdes::rewind()
{
    pos=0;
}

void Serdes::ffwd()
{
    pos=val.mv_size-1;
}

bool Serdes::isbegin()
{
    return pos==0 ? true : false;
}

bool Serdes::isend()
{
    return pos==val.mv_size-1 ? true : false;
}
