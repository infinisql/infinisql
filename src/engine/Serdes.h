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
#include <lmdb.h>

#include "decimal/decnum.h"

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
    ~Serdes();

    // pods
    /** 
     * @brief serialize item
     *
     * @param d item to serialize
     */
    void ser(int8_t d);
    /** 
     * @brief get size of item serialized
     *
     * @param d item to measure for serialization
     *
     * @return size of item when serialized
     */
    static size_t sersize(int8_t d);
    /** 
     * @brief deserialize item
     *
     * @param d buffer into which to serialize item
     */
    void des(int8_t &d);
    /** 
     * @brief serialize item
     *
     * @param d item to serialize
     */
    void ser(int16_t d);
    /** 
     * @brief get size of item serialized
     *
     * @param d item to measure for serialization
     *
     * @return size of item when serialized
     */
    static size_t sersize(int16_t d);
    /** 
     * @brief deserialize item
     *
     * @param d buffer into which to serialize item
     */
    void des(int16_t &d);
    /** 
     * @brief serialize item
     *
     * @param d item to serialize
     */
    void ser(int32_t d);
    /** 
     * @brief get size of item serialized
     *
     * @param d item to measure for serialization
     *
     * @return size of item when serialized
     */
    static size_t sersize(int32_t d);
    /** 
     * @brief deserialize item
     *
     * @param d buffer into which to serialize item
     */
    void des(int32_t &d);
    /** 
     * @brief serialize item
     *
     * @param d item to serialize
     */
    void ser(int64_t d);
    /** 
     * @brief get size of item serialized
     *
     * @param d item to measure for serialization
     *
     * @return size of item when serialized
     */
    static size_t sersize(int64_t d);
    /** 
     * @brief deserialize item
     *
     * @param d buffer into which to serialize item
     */
    void des(int64_t &d);
    /** 
     * @brief serialize item
     *
     * @param d item to serialize
     */
    void ser(double d);
    /** 
     * @brief get size of item serialized
     *
     * @param d item to measure for serialization
     *
     * @return size of item when serialized
     */
    static size_t sersize(float d);
    /** 
     * @brief deserialize item
     *
     * @param d buffer into which to serialize item
     */
    void des(float &d);
    /** 
     * @brief serialize item
     *
     * @param d item to serialize
     */
    void ser(float d);
    /** 
     * @brief get size of item serialized
     *
     * @param d item to measure for serialization
     *
     * @return size of item when serialized
     */
    static size_t sersize(double d);
    /** 
     * @brief deserialize item
     *
     * @param d buffer into which to serialize item
     */
    void des(double &d);
    /** 
     * @brief serialize item
     *
     * @param d item to serialize
     */
    void ser(char d);
    /** 
     * @brief get size of item serialized
     *
     * @param d item to measure for serialization
     *
     * @return size of item when serialized
     */
    static size_t sersize(char d);
    /** 
     * @brief deserialize item
     *
     * @param d buffer into which to serialize item
     */
    void des(char &d);
    /** 
     * @brief serialize item
     *
     * @param d item to serialize
     */
    void ser(bool d);
    /** 
     * @brief get size of item serialized
     *
     * @param d item to measure for serialization
     *
     * @return size of item when serialized
     */
    static size_t sersize(bool d);
    /** 
     * @brief deserialize item
     *
     * @param d buffer into which to serialize item
     */
    void des(bool &d);

    // containers
    /** 
     * @brief serialize string prepend length in the output stream
     *
     * @param d string to serialize
     */
    void ser(const std::string &d);
    /** 
     * @brief get size of string if serialized, plus room to hold length
     *
     * @param d item to measure for serialization
     *
     * @return size of item when serialized plus room to hold length
     */
    static size_t sersize(const std::string &d);
    /** 
     * @brief deserialize string with length in object before string
     *
     * @param d buffer into which to serialize item
     */
    void des(std::string &d);
    /** 
     * @brief serialize string and store only the contents (not the length)
     *
     * @param d item to serialize
     * @param dsize length to serialize
     */
    void ser(const std::string &d, size_t dsize);
    /** 
     * @brief create string and deserialize into it, providing length
     *
     * @param d buffer to create and into which to deserialize
     * @param dsize length to deserialize
     */
    void des(std::string *&d, size_t dsize);
	/**
	 * @brief serialize decimal and store only the contents (not the length)
	 *
	 * @param d item to serialize
	 * @param dsize length to serialize
	 */
	void ser(const decimal &d, size_t dsize);
	/**
	 * @brief create decmial and deserialize into it, providing length
	 *
	 * @param d buffer to create and into which to deserialize
	 * @param dsize length to deserialize
	 */
	void des(decimal *&d, size_t dsize);
    /** 
     * @brief serialize byte sequence, such as packed struct
     *
     * @param d start of region to serialize
     * @param dsize size of region to serialize
     */
    void ser(void *d, size_t dsize);
    /** 
     * @brief deserialize into byte sequence, such as packed struct
     *
     * @param d region to deserialize into
     * @param dsize size of region to deserialize
     */
    void des(void *d, size_t dsize); 

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

#endif // INFINISQLSERDES_H
