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
 * @file   Field.h
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Jan  7 02:50:15 2014
 * 
 * @brief  classes pertaining to data fields
 */

#ifndef INFINISQLFIELD_H
#define INFINISQLFIELD_H

#include <memory>
#include "Metadata.h"

/** 
 * @brief field contents
 *
 */
class FieldValue
{
public:
    /** 
     * @brief type of value contained (not field type)
     */
    enum valtype_e : uint8_t
    {
        VAL_NONE=0,
        VAL_POD,
        VAL_NULL,
        VAL_STRING
    };

    /** 
     * @brief field contents, or pointer to string
     */
    union value_u
    {
        int8_t int1;
        int16_t int2;
        int32_t int4;
        int64_t int8;
        float singlefloat;
        double doublefloat;
        int32_t character;
        bool boolean;
        std::string *str;
    };
    
    FieldValue();
    FieldValue(const FieldValue &orig);
    FieldValue &operator= (const FieldValue &orig);
    /** 
     * @brief deep copy of FieldValue
     *
     * @param orig original FieldValue
     */
    void cp(const FieldValue &orig);
    virtual ~FieldValue();

    /** 
     * @brief delete if value is string
     *
     */
    void deletestr();
    /** 
     * @ brief set field to nullptr
     *
     */
    void nullify();
    /** 
     * @brief set value to a string
     *
     * @param val value to set
     */
    void set(const std::string &val);
    /** 
     * @brief set value to 8bit int
     *
     * @param val value to set
     */
    void set(int8_t val);
    /** 
     * @brief set value to 16bit int
     *
     * @param val value to set
     */
    void set(int16_t val);
    /** 
     * @brief set value to 32bit int
     *
     * @param val value to set
     */
    void set(int32_t val);
    /** 
     * @brief set value to 64bit int
     *
     * @param val value to set
     */
    void set(int64_t val);
    /** 
     * @brief set value to float
     *
     * @param val value to set
     */
    void set(float val);
    /** 
     * @brief set value to double
     *
     * @param val value to set
     */
    void set(double val);
    /** 
     * @brief set value to char
     *
     * @param val value to set
     */
    void set(char val);
    /** 
     * @brief set value to boolean true
     *
     * @param val value to set
     */
    void settrue();
    /** 
     * @brief set value to boolean false
     *
     * @param val value to set
     */
    void setfalse();
    /** 
     * @brief retrieve 8bit int value
     *
     * @param val returned value
     *
     * @return 8bit int
     */
    int8_t get(int8_t &val, bool &isnull);
    /** 
     * @brief retrieve 16bit int value
     *
     * @param val returned value
     *
     * @return 16bit int
     */
    int16_t get(int16_t &val, bool &isnull);
    /** 
     * @brief retrieve 32bit int value
     *
     * @param val returned value
     * @param isnull whether is null or not
     *
     * @return 32bit int
     */
    int32_t get(int32_t &val, bool &isnull);
    /** 
     * @brief retrieve 64bit int value
     *
     * @param val returned value
     * @param isnull whether is null or not
     *
     * @return 64bit int
     */
    int64_t get(int64_t &val, bool &isnull);
    /** 
     * @brief retrieve float value
     *
     * @param val returned value
     * @param isnull whether is null or not
     *
     * @return float
     */
    float get(float &val, bool &isnull);
    /** 
     * @brief retrieve double value
     *
     * @param val returned value
     * @param isnull whether is null or not
     *
     * @return double
     */
    double get(double &val, bool &isnull);
    /** 
     * @brief retrieve single char value
     *
     * @param val returned value
     * @param isnull whether is null or not
     *
     * @return single char
     */
    char get(char &val, bool &isnull);
    /** 
     * @brief retrieve string value
     *
     * @param val returned value
     * @param isnull whether is null or not
     */
    void get(std::string &val, bool &isnull);
    /** 
     * @brief return boolean value
     *
     * @param isnull whether is null or not
     *
     * @return true if true, false if false
     */
    bool getbool(bool &isnull);
    /** 
     * @brief check if null or not
     *
     * @return true if null, false if not null
     */
    bool getnull();
    /** 
     * @brief serialize to character array
     *
     * @param output 
     *
     * @return size of serialized object
     */
    void ser(Serdes &output);
    /** 
     * @brief get size of object if serialized
     *
     */
    size_t sersize();
    /** 
     *
     * @param input input serialized object
     *
     */
    void des(Serdes &input);
    
    valtype_e valtype;
    value_u value;
};

class Table;
/** 
 * @brief SQL field type
 */
class Field : public Metadata
{
public:
    enum type_e : uint8_t
    {
        TYPE_NONE=0,
        TYPE_TINYINT,
        TYPE_SMALLINT,
        TYPE_INT,
        TYPE_BIGINT,
        TYPE_BOOLEAN,
        TYPE_NUMERIC,
        TYPE_DECIMAL,
        TYPE_REAL,
        TYPE_DOUBLE_PRECISION,
        TYPE_FLOAT,
        TYPE_CHARACTER,
        TYPE_CHARACTER_VARYING,
        TYPE_BIT,
        TYPE_BIT_VARYING,
        TYPE_DATE,
        TYPE_TIME,
        TYPE_TIMESTAMP,
        TYPE_TIME_WITH_TIME_ZONE,
        TYPE_TIMESTAMP_WITH_TIME_ZONE
    };
    
    Field();
    /** 
     * @brief create Field
     *
     * @param namearg field name
     * @param typearg field type
     */
    Field(Table *parentTablearg, const std::string& namearg, type_e typearg);
    /** 
     * @brief create Field
     *
     * @param namearg field name
     * @param typearg field type
     * @param arg1arg parameter to create field
     */
    Field(Table *parentTablearg, const std::string& namearg, type_e typearg,
          int64_t arg1arg);
    /** 
     * @brief create Field
     *
     * @param namearg field name
     * @param typearg field type
     * @param arg1arg 1st parameter to create field
     * @param arg2arg 2nd parameter to create field
     */
    Field(Table *parentTablearg, const std::string& namearg, type_e typearg,
          int64_t arg1arg, int64_t arg2arg);
    Field(const Field &orig);
    Field &operator= (const Field &orig);
    /** 
     * @brief copy sufficient for reproduction elsewhere
     *
     * requires post-processing for destination actors' pointers to related
     * objects
     *
     * @param orig 
     */
    void cp(const Field &orig);
    ~Field();

    /** 
     * @brief intialize field parents and maps, get fieldid
     *
     * @param parentTablearg parent table
     * @param namearg name
     * 
     * @return 
     */
    bool initializer(Table *parentTablearg, const std::string& namearg);
    /** 
     * @brief get metadata parent information from parentTable
     *
     */
    void getparents();
    
    /** 
     * @brief serialize to character array
     *
     * don't serialize tablePtr because that will be different next time
     * the Field is loaded. Plus, it will be loaded after its Table
     *
     * @param output 
     *
     * @return size of serialized object
     */
    void ser(Serdes &output);
    /** 
     * @brief get size of object if serialized
     *
     */
    size_t sersize();
    /** 
     * @brief deserialize from byte array
     *
     * @param input array to deserialize from
     *
     */
    void des(Serdes &input);

    /** 
     * @brief serialize a field value, such as into an LMDB value
     *
     * @param fieldValue input field value
     * @param output output serialized object
     */
    void serValue(FieldValue &fieldValue, Serdes &output);
    /** 
     * @brief populate field value from serialized object, such as LMDB value
     *
     * @param input input object
     * @param fieldValue field value to populate
     */
    void desValue(Serdes &input, FieldValue &fieldValue);
    /** 
     * @brief get length of field contents
     *
     * @param fieldValue field contents
     *
     * @return length of contents
     */
    size_t valueSize(FieldValue &fieldValue);
    /** 
     * @brief send field output to pg wire protocol output buffer
     *
     * @param fieldValue field value
     * @param outmsg output buffer
     */
    void pgoutput(FieldValue &fieldValue, std::string &outmsg);
    /** 
     * @brief send 32bit integer to pg wire protocol output buffer
     *
     * @param val integer to send
     * @param outmsg output buffer
     */
    static void pgoutint32(int32_t val, std::string &outmsg);
    /** 
     * @brief convert SQL operand to relevant field type
     *
     * @param field 
     */
    void convertValue(FieldValue &fieldValue);
    
    type_e type;
    ssize_t size;
    ssize_t precision;
    ssize_t scale;

    FieldValue defaultValue;
    bool nullconstraint;
};

#endif // INFINISQLFIELD_H
