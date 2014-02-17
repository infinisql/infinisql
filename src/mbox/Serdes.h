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
 *
 * uses MDB_val as data storage type to facilitate working with LMDB
 */

#ifndef INFINISQLSERDES_H
#define INFINISQLSERDES_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <lmdb.h>

#include "../decimal/decnum.h"

extern std::ofstream logfile;
#define LOG(...) logfile << __FILE__ << " " << __LINE__ << " errno: " << errno << " '" << strerror(errno) << "' " << __VA_ARGS__ << std::endl

/** 
 * @brief object to serialize anything
 *
 * position counter moves automatically when serializing to and
 * deserializing from. to use, first get the size of the entire object,
 * can add sersize() methods to do so. Then serialize into /
 * deserialize from. All pod and external 3rd party types should be
 * ser-des'd from methods in this class. Each object should have its
 * own ser(), sersize() and des() methods that work with a Serdes object.
 *
 */
class Serdes
{
public:
    Serdes();
    Serdes(size_t mv_sizearg);
    /** 
     * @brief create object to deserialize from an MDB_val
     *
     * The MDB_val.mv_data item will not be deleted when this object
     * ends. This is intented to deserialize from an LMDB key or value without
     * having to first copy it.
     *
     * If the item is not in an LMDB database, and needs to be deleted when
     * finished, then set isreadonly to true.
     *
     * @param valarg 
     */
    Serdes(MDB_val &valarg);
    /** 
     * @brief create object to deserialize from character sequence
     *
     * MDB_val.mv_data will be deleted when object is destroyed
     * this is intended to carry inter-node messages from IbGateway
     * to destination actor's mbox
     *
     * @param data 
     * @param size 
     */
    Serdes(const char *data, size_t size);
    ~Serdes();

    /** 
     * @brief set position to beginning of data
     *
     */
    void rewind();
    /** 
     * @brief set position to end of data
     *
     */
    void ffwd();
    /** 
     * @brief checks if at beginning of data
     *
     *
     * @return true at beginning, false otherwise
     */
    bool isbegin();
    /** 
     * @brief checks if at end of data
     *
     *
     * @return true at end, false otherwise
     */
    bool isend();

    bool isreadonly;
    size_t pos;
    struct MDB_val val;
};

template < typename T >
void serpod(T d, Serdes &output)
{
    memcpy((char *)output.val.mv_data+output.pos, &d, sizeof(d));
    output.pos+=sizeof(d);
}

template < typename T >
void despod(Serdes &input, T &d)
{
    memcpy(&d, (char *)input.val.mv_data+input.pos, sizeof(d));
    input.pos+=sizeof(d);
}

void ser(int8_t d, Serdes &output);
size_t sersize(int8_t d);
void des(Serdes &input, int8_t &d);
void ser(int16_t d, Serdes &output);
size_t sersize(int16_t d);
void des(Serdes &input, int16_t &d);
void ser(int32_t d, Serdes &output);
size_t sersize(int32_t d);
void des(Serdes &input, int32_t &d);
void ser(int64_t d, Serdes &output);
size_t sersize(int64_t d);
void des(Serdes &input, int64_t &d);
void ser(uint8_t d, Serdes &output);
size_t sersize(uint8_t d);
void des(Serdes &input, uint8_t &d);
void ser(uint16_t d, Serdes &output);
size_t sersize(uint16_t d);
void des(Serdes &input, uint16_t &d);
void ser(uint32_t d, Serdes &output);
size_t sersize(uint32_t d);
void des(Serdes &input, uint32_t &d);
void ser(uint64_t d, Serdes &output);
size_t sersize(uint64_t d);
void des(Serdes &input, uint64_t &d);
void ser(float d, Serdes &output);
size_t sersize(float d);
void des(Serdes &input, float &d);
void ser(double d, Serdes &output);
size_t sersize(double d);
void des(Serdes &input, double &d);
void ser(char d, Serdes &output);
size_t sersize(char d);
void des(Serdes &input, char &d);
void ser(bool d, Serdes &output);
size_t sersize(bool d);
void des(Serdes &input, bool &d);

template < typename T >
void ser(std::vector<T> d, Serdes &output)
{
    // nelem, item[, item, ...]
    size_t s=d.size();
    ser(s, output);
    for (size_t n=0; n < d.size(); ++n)
    {
        ser(d[n], output);
    }
}

template < typename T >
size_t sersize(std::vector<T> d)
{
    size_t retval=sizeof(size_t);
    for (size_t n=0; n < d.size(); ++n)
    {
        retval += sersize(d[n]);
    }
    return retval;
}

template < typename T >
void des(Serdes &input, std::vector<T> &d)
{
    size_t s;
    des(input, s);
    d.reserve(s);
    T val;
    for (size_t n=0; n < s; ++n)
    {
        des(input, val);
        d.push_back(val);
    }
}

template < typename T, typename U >
void ser(std::map<T, U> d, Serdes &output)
{
    // nelem, item[, item, ...]
    size_t s=d.size();
    ser(s, output);
    typename std::map<T, U>::iterator it;
    for (it = d.begin(); it != d.end(); ++it)
    {
        ser(it->first, output);
        ser(it->second, output);
    }
}

template < typename T, typename U >
size_t sersize(std::map<T, U> d)
{
    size_t retval=sizeof(size_t);
    typename std::map<T, U>::iterator it;
    for (it = d.begin(); it != d.end(); ++it)
    {
        retval += sersize(it->first);
        retval += sersize(it->second);
    }

    return retval;
}

template < typename T, typename U, typename V, typename W >
void des(Serdes &input, std::map<T, U> &d)
{
    size_t s;
    des(input, s);
    T key;
    U val;
    for (size_t n=0; n < s; ++n)
    {
        des(input, key);
        des(input, val);
        d[key]=val;
    }
}

template < typename T, typename U >
void ser(std::unordered_map<T, U> d, Serdes &output)
{
    // nelem, item[, item, ...]
    size_t s=d.size();
    ser(s, output);
    typename std::unordered_map<T, U>::iterator it;
    for (it = d.begin(); it != d.end(); ++it)
    {
        ser(it->first, output);
        ser(it->second, output);
    }
}

template < typename T, typename U >
size_t sersize(std::unordered_map<T, U> d)
{
    size_t retval=sizeof(size_t);
    typename std::unordered_map<T, U>::iterator it;
    for (it = d.begin(); it != d.end(); ++it)
    {
        retval += sersize(it->first);
        retval += sersize(it->second);
    }

    return retval;
}

template < typename T, typename U, typename V, typename W >
void des(Serdes &input, std::unordered_map<T, U> &d)
{
    size_t s;
    des(input, s);
    T key;
    U val;
    for (size_t n=0; n < s; ++n)
    {
        des(input, key);
        des(input, val);
        d[key]=val;
    }
}

/* for string with length prepended in serialization object */
void ser(const std::string &d, Serdes &output);
size_t sersize(const std::string &d);
void des(Serdes &input, std::string &d);

/* for string with length not in serialization object */
void ser(const std::string &d, size_t dsize, Serdes &output);
void des(Serdes &input, std::string &d, size_t dsize);

/* for arbitrary data */
void ser(const void *d, size_t dsize, Serdes &output);
void des(Serdes &input, void *d, size_t dsize);

#endif // INFINISQLSERDES_H
