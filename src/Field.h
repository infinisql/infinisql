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

#include "main.h"

class Table;
/** 
 * @brief SQL field type
 */
class Field
{
public:
    enum type_e
    {
        TYPE_NONE=0,
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
     * @param tablePtrarg associated with Table
     * @param namearg field name
     * @param fieldidarg fieldid (index within table)
     * @param typearg field type
     */
    Field(Table *tablePtrarg, std::string namearg, int16_t fieldidarg,
              type_e typearg);
    /** 
     * @brief create Field
     *
     * @param tablePtrarg associated with Table
     * @param namearg field name
     * @param fieldidarg fieldid (index within table)
     * @param typearg field type
     * @param arg1arg parameter to create field
     */
    Field(Table *tablePtrarg, std::string namearg, int16_t fieldidarg,
              type_e typearg, int64_t arg1arg);
    /** 
     * @brief create Field
     *
     * @param tablePtrarg associated with Table
     * @param namearg field name
     * @param fieldidarg fieldid (index within table)
     * @param typearg field type
     * @param arg1arg 1st parameter to create field
     * @param arg2arg 2nd parameter to create field
     */
    Field(Table *tablePtrarg, std::string namearg, int16_t fieldidarg,
              type_e typearg, int64_t arg1arg, int64_t arg2arg);
    ~Field();

    Table *tablePtr;
    std::string name;
    int16_t fieldid;
    type_e type;
    ssize_t size;
    ssize_t precision;
    ssize_t scale;
};

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
    enum __attribute__ ((__packed__)) valtype_e
    {
        VAL_NONE=0,
        VAL_POD,
        VAL_NULL,
        VAL_STRING
    };

    /** 
     * @brief field contents, or pointer to string
     */
    union __attribute__ ((__packed__)) value_u
    {
        int16_t int2;
        int32_t int4;
        int64_t int8;
        float singlefloat;
        double doublefloat;
        char character;
        bool boolean;
        std::string *str;
    };
    
    FieldValue();
    /** 
     * @brief populate field value from character stream
     *
     * intended to read a section of a Table row
     *
     * @param fieldTypearg field type
     * @param inputarg input data
     * @param len length of data read
     */
    FieldValue(Field &fieldarg, char *inputarg, size_t *len);
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
     * @brief write out field contents to a row string
     *
     * @param field field
     * @param output row string segment
     *
     * @return length of contents
     */
    size_t torow(Field &field, char *output);
    /** 
     * @brief get the size of field contents
     *
     * @param field
     * 
     * @return length in bytes
     */
    size_t size(Field &field);
    /** 
     * @brief send field output to pg wire protocol output buffer
     *
     * @param field field
     * @param outmsg output buffer
     */
    void pgoutput(Field &field, std::string &outmsg);
    /** 
     * @brief send 32bit integer to pg wire protocol output buffer
     *
     * @param val integer to send
     * @param outmsg output buffer
     */
    void pgoutint32(int32_t val, std::string &outmsg);
    /** 
     * @ brief set field to NULL
     *
     */
    void nullify();
    /** 
     * @brief set value to a string
     *
     * @param val value to set
     */
    void set(std::string &val);
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
     * @brief convert SQL operand to relevant field type
     *
     * @param field 
     */
    void convert(Field &field);
    /** 
     * @brief retrieve 16bit int value
     *
     * @param val returned value
     *
     * @return 16bit int
     */
    int16_t get(int16_t *val, bool *isnull);
    /** 
     * @brief retrieve 32bit int value
     *
     * @param val returned value
     * @param isnull whether is null or not
     *
     * @return 32bit int
     */
    int32_t get(int32_t *val, bool *isnull);
    /** 
     * @brief retrieve 64bit int value
     *
     * @param val returned value
     * @param isnull whether is null or not
     *
     * @return 64bit int
     */
    int64_t get(int64_t *val, bool *isnull);
    /** 
     * @brief retrieve float value
     *
     * @param val returned value
     * @param isnull whether is null or not
     *
     * @return float
     */
    float get(float *val, bool *isnull);
    /** 
     * @brief retrieve double value
     *
     * @param val returned value
     * @param isnull whether is null or not
     *
     * @return double
     */
    double get(double *val, bool *isnull);
    /** 
     * @brief retrieve single char value
     *
     * @param val returned value
     * @param isnull whether is null or not
     *
     * @return single char
     */
    char get(char *val, bool *isnull);
    /** 
     * @brief retrieve string value
     *
     * @param val returned value
     * @param isnull whether is null or not
     */
    void get(std::string &val, bool *isnull);
    /** 
     * @brief return boolean value
     *
     * @param isnull whether is null or not
     *
     * @return true if true, false if false
     */
    bool getbool(bool *isnull);
    /** 
     * @brief check if null or not
     *
     * @return true if null, false if not null
     */
    bool getnull();
    
   
    valtype_e valtype;
    value_u value;
};

#endif // INFINISQLFIELD_H
