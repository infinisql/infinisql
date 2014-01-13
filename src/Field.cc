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
 * @file   Field.cc
 * @author Mark Travis <mtravis15432+src@gmail.com>
 * @date   Tue Jan  7 02:51:34 2014
 * 
 * @brief  classes pertaining to data fields
 */

#include "Field.h"
#line 30 "Field.cc"

Field::Field() : tablePtr (NULL), fieldid (-1), type (TYPE_NONE), size (-1),
                 precision (-1), scale (-1)
{
    
}

Field::Field(Table *tablePtrarg, std::string namearg, int16_t fieldidarg,
                     type_e typearg)
    : tablePtr (tablePtrarg), name (namearg), fieldid (fieldidarg),
      type (typearg), size (-1), precision (-1), scale (-1)
{
    switch (type)
    {
    case TYPE_NUMERIC:
        precision=100;
        scale=0;
        break;

    case TYPE_DECIMAL:
        precision=100;
        scale=0;
        break;
        
    case TYPE_CHARACTER:
        size=1;
        break;

    case TYPE_BIT:
        size=1;
        break;

    case TYPE_TIME:
        precision=6;
        break;

    case TYPE_TIMESTAMP:
        precision=6;
        break;

    case TYPE_TIME_WITH_TIME_ZONE:
        precision=6;
        break;

    case TYPE_TIMESTAMP_WITH_TIME_ZONE:
        precision=6;
        break;
        
    default:
        LOG("type " << type << " doesn't take 0 arguments");
    }
}    

Field::Field(Table *tablePtrarg, std::string namearg, int16_t fieldidarg,
                     type_e typearg, int64_t arg1arg)
    : tablePtr (tablePtrarg), name (namearg), fieldid (fieldidarg),
      type (typearg), scale (-1)
{
    switch (type)
    {
    case TYPE_NUMERIC:
        precision=arg1arg;
        scale=0;
        break;

    case TYPE_DECIMAL:
        precision=arg1arg;
        scale=0;
        break;
        
    case TYPE_CHARACTER:
        size=arg1arg;
        break;

    case TYPE_CHARACTER_VARYING:
        size=arg1arg;
        break;

    case TYPE_BIT:
        size=arg1arg;
        break;

    case TYPE_BIT_VARYING:
        size=arg1arg;
        break;

    case TYPE_TIME:
        precision=arg1arg;
        break;

    case TYPE_TIMESTAMP:
        precision=arg1arg;
        break;

    case TYPE_TIME_WITH_TIME_ZONE:
        precision=arg1arg;
        break;

    case TYPE_TIMESTAMP_WITH_TIME_ZONE:
        precision=arg1arg;
        
    default:
        LOG("type " << type << " doesn't take a single argument");
    }
}

Field::Field(Table *tablePtrarg, std::string namearg, int16_t fieldidarg,
                     type_e typearg, int64_t arg1arg, int64_t arg2arg)
    : tablePtr (tablePtrarg), name (namearg), fieldid (fieldidarg),
      type (typearg), size (-1), precision (arg1arg), scale (arg2arg)
{
}

Field::~Field()
{
    
}

void Field::ser(Serdes &output)
{
    output.ser(name);
    output.ser(fieldid);
    output.ser((char)type);
    output.ser((int64_t)size);
    output.ser((int64_t)precision);
    output.ser((int64_t)scale);
}

size_t Field::sersize()
{
    size_t retsize=0;
    retsize = Serdes::sersize(name);
    retsize += Serdes::sersize(fieldid);
    retsize += Serdes::sersize((char)type);
    retsize += Serdes::sersize((int64_t)size);
    retsize += Serdes::sersize((int64_t)precision);
    retsize += Serdes::sersize((int64_t)scale);

    return retsize;
}

void Field::des(Serdes &input, Table *tablePtrarg)
{
    tablePtr=tablePtrarg;
    input.des(name);
    input.des(&fieldid);
    input.des((char *)&type);
    input.des((int64_t *)&size);
    input.des((int64_t *)&precision);
    input.des((int64_t *)&scale);
}

FieldValue::FieldValue() : valtype (VAL_NONE)
{
    value.int8=0;
}

FieldValue::FieldValue(Field &fieldarg, char *inputarg, size_t *len)
{
    switch (fieldarg.type)
    {
    case Field::TYPE_SMALLINT:
        valtype=VAL_POD;
        *len=sizeof(value.int2);
        memcpy(&value.int2, inputarg, *len);
        break;

    case Field::TYPE_INT:
        valtype=VAL_POD;
        *len=sizeof(value.int4);
        memcpy(&value.int4, inputarg, *len);
        break;

    case Field::TYPE_BIGINT:
        valtype=VAL_POD;
        *len=sizeof(value.int8);
        memcpy(&value.int8, inputarg, *len);
        break;

    case Field::TYPE_BOOLEAN:
        valtype=VAL_POD;
        *len=sizeof(value.boolean);
        memcpy(&value.boolean, inputarg, *len);

    case Field::TYPE_NUMERIC:
        valtype=VAL_STRING;
        *len=fieldarg.precision;
        value.str=new string(inputarg, *len);
        break;

    case Field::TYPE_DECIMAL:
        valtype=VAL_STRING;
        *len=fieldarg.precision;
        value.str=new string(inputarg, *len);
        break;

    case Field::TYPE_REAL:
        valtype=VAL_POD;
        *len=sizeof(value.singlefloat);
        memcpy(&value.singlefloat, inputarg, *len);
        break;

    case Field::TYPE_DOUBLE_PRECISION:
        valtype=VAL_POD;
        *len=sizeof(value.doublefloat);
        memcpy(&value.doublefloat, inputarg, *len);
        break;

    case Field::TYPE_FLOAT:
        valtype=VAL_POD;
        *len=sizeof(value.doublefloat);
        memcpy(&value.doublefloat, inputarg, *len);
        break;

    case Field::TYPE_CHARACTER:
        if (fieldarg.size==1)
        {
            valtype=VAL_POD;
            *len=1;
            value.character=*inputarg;
        }
        else
        {
            valtype=VAL_STRING;
            *len=fieldarg.size;
            value.str=new string(inputarg, *len);
        }
        break;

    case Field::TYPE_CHARACTER_VARYING:
    {
        valtype=VAL_STRING;
        size_t strlength;
        memcpy(&strlength, inputarg, sizeof(strlength));
        *len=strlength+sizeof(strlength);
        value.str=new string(inputarg+sizeof(strlength), strlength);
    }
    break;

    case Field::TYPE_BIT:
        if (fieldarg.size <= 8)
        {
            valtype=VAL_POD;
            *len=1;
            value.character=*inputarg;
        }
        else
        {
            valtype=VAL_STRING;
            *len=(fieldarg.size+7)/8;
            value.str=new string(inputarg, *len);
        }
        break;

    case Field::TYPE_BIT_VARYING:
    {
        valtype=VAL_STRING;
        size_t strlength;
        memcpy(&strlength, inputarg, sizeof(strlength));
        *len=((strlength+7)/8) + sizeof(strlength);
        value.str=new string(inputarg+sizeof(strlength), strlength);
    }
    break;

    /**
     * @todo date & time functions, likely use boost::date_time
     * 
     */

        /*
          // probably use boost::date_time 8 bytes without tz
          // mm-dd-yyyy
    case TYPE_DATE:
        break;

        // hh:mm:ss.xxxxxx
    case TYPE_TIME:
        break;

        // mm-dd-yyyy hh:mm:ss.xxxxxx
    case TYPE_TIMESTAMP:
        break;

        // -12:59 to +13:00 is 1560 minutes, 2 bytes?, so the above 
    case TYPE_TIME_WITH_TIME_ZONE:
        break;

    case TYPE_TIMESTAMP_WITH_TIME_ZONE:
        break;
        */
        
    default:
        LOG("field type " << fieldarg.type << " not implemented");
    }
}

FieldValue::FieldValue(const FieldValue &orig)
{
    cp(orig);
}

FieldValue &FieldValue::operator= (const FieldValue &orig)
{
    cp(orig);
    return *this;
}

void FieldValue::cp(const FieldValue &orig)
{
    valtype=orig.valtype;
    if (valtype==VAL_POD)
    {
        value=orig.value;
    }
    else if (valtype==VAL_STRING)
    {
        value.str=new string;
    }
}

FieldValue::~FieldValue()
{
    deletestr();
}

void FieldValue::deletestr()
{
    if (valtype==VAL_STRING)
    {
        delete value.str;
    }
}

size_t FieldValue::torow(Field &field, char *output)
{
    switch (field.type)
    {
    case Field::TYPE_SMALLINT:
        memcpy(output, &value.int2, sizeof(value.int2));
        return sizeof(value.int2);
//        break;

    case Field::TYPE_INT:
        memcpy(output, &value.int4, sizeof(value.int4));
        return sizeof(value.int4);
//        break;

    case Field::TYPE_BIGINT:
        memcpy(output, &value.int8, sizeof(value.int8));
        return sizeof(value.int8);
//        break;

    case Field::TYPE_BOOLEAN:
        memcpy(output, &value.boolean, sizeof(value.boolean));
        return sizeof(value.boolean);
//        break;

    case Field::TYPE_NUMERIC:
        value.str->copy(output, field.precision, 0);
        return field.precision;
//        break;

    case Field::TYPE_DECIMAL:
        value.str->copy(output, field.precision, 0);
        return field.precision;
//        break;

    case Field::TYPE_REAL:
        memcpy(output, &value.singlefloat, sizeof(value.singlefloat));
        return sizeof(value.singlefloat);
//        break;

    case Field::TYPE_DOUBLE_PRECISION:
        memcpy(output, &value.doublefloat, sizeof(value.doublefloat));
        return sizeof(value.doublefloat);
//        break;

    case Field::TYPE_FLOAT:
        memcpy(output, &value.doublefloat, sizeof(value.doublefloat));
        return sizeof(value.doublefloat);
//        break;

    case Field::TYPE_CHARACTER:
        if (valtype==VAL_POD)
        {
            memcpy(output, &value.character, 1);
            return 1;
        }
        else
        {
            value.str->copy(output, field.size, 0);
            return field.size;
        }
//        break;

    case Field::TYPE_CHARACTER_VARYING:
    {
        size_t strlength=value.str->size();
        memcpy(output, &strlength, sizeof(strlength));
        value.str->copy(output+sizeof(strlength), value.str->size(), 0);
        return value.str->size() + sizeof(strlength);
    }
//    break;

    case Field::TYPE_BIT:
        if (valtype==VAL_POD)
        {
            memcpy(output, &value.character, 1);
            return 1;
        }
        else
        {
            value.str->copy(output, field.size, 0);
            return field.size;
        }
//        break;

    case Field::TYPE_BIT_VARYING:
    {
        size_t strlength=value.str->size();
        memcpy(output, &strlength, sizeof(strlength));
        value.str->copy(output+sizeof(strlength), value.str->size(), 0);
        return value.str->size() + sizeof(strlength);
    }
//    break;


    /**
     * @todo date & time fields
     * 
     */

        /*
    case TYPE_DATE:
        break;

    case TYPE_TIME:
        break;

    case TYPE_TIMESTAMP:
        break;

    case TYPE_TIME_WITH_TIME_ZONE:
        break;

    case TYPE_TIMESTAMP_WITH_TIME_ZONE:
        break;
        */
        
    default:
        LOG("field type " << field.type << " not implemented");
    }

    return 0;
}

size_t FieldValue::size(Field &field)
{
    switch (field.type)
    {
    case Field::TYPE_SMALLINT:
        return sizeof(value.int2);
        
    case Field::TYPE_INT:
        return sizeof(value.int4);
        
    case Field::TYPE_BIGINT:
        return sizeof(value.int8);
        
    case Field::TYPE_BOOLEAN:
        return sizeof(value.boolean);

        /**
         * @todo NUMERIC and DECIMAL types
         * 
         */

        /*
    case TYPE_NUMERIC:
    case TYPE_DECIMAL:
        */
    case Field::TYPE_REAL:
        return sizeof(value.singlefloat);
        
    case Field::TYPE_DOUBLE_PRECISION:
        return sizeof(value.doublefloat);
        
    case Field::TYPE_FLOAT:
        return sizeof(value.doublefloat);
        
    case Field::TYPE_CHARACTER:
        return field.size;
        
    case Field::TYPE_CHARACTER_VARYING:
        return value.str->size();
        
    case Field::TYPE_BIT:
        return field.size;
        
    case Field::TYPE_BIT_VARYING:
        return value.str->size();

        /**
         * @todo date & time types
         * 
         */

        /*
    case TYPE_DATE:
    case TYPE_TIME:
    case TYPE_TIMESTAMP:
    case TYPE_TIME_WITH_TIME_ZONE:
    case TYPE_TIMESTAMP_WITH_TIME_ZONE:
        */
        
    default:
        LOG("can't get size of field type" << field.type);
    }

    return -1;
}

void FieldValue::pgoutput(Field &field, std::string &outmsg)
{
    if (valtype==VAL_NULL)
    {
        pgoutint32(-1, outmsg);
        return;
    }
    
    switch (field.type)
    {
    case Field::TYPE_SMALLINT:
    {
        char val[7];  // length of largest int16_t plus - and \n
        int32_t len=sprintf(val, "%i", value.int2);
        pgoutint32(len, outmsg);
        outmsg.append(val);
    }
        
    case Field::TYPE_INT:
    {
        char val[12]; // length of largest int32_t plus - and \n
        int32_t len=sprintf(val, "%i", value.int4);
        pgoutint32(len, outmsg);
        outmsg.append(val);
    }
    break;
        
    case Field::TYPE_BIGINT:
    {
        char val[21]; // length of largest int64_t plus - and \n
        int32_t len=sprintf(val, "%li", value.int8);
        pgoutint32(len, outmsg);
        outmsg.append(val);
    }
    break;
        
    case Field::TYPE_BOOLEAN:
        pgoutint32((int32_t)sizeof(value.boolean), outmsg);
        if (value.boolean==true)
        {
            outmsg.append(1, 't');
        }
        else
        {
            outmsg.append(1, 'f');
        }
        break;

        /**
         * @todo NUMERIC and DECIMAL types
         * 
         */

        /*
    case TYPE_NUMERIC:
    case TYPE_DECIMAL:
        */
    case Field::TYPE_REAL:
    {
        std::stringstream val;
        val << value.singlefloat;

        if (value.singlefloat / (int64_t)value.singlefloat == 1)
        {
            val << ".0";
        }

        int32_t len=val.str().size();
        pgoutint32(len, outmsg);
        outmsg.append(val.str().c_str());
    }
    break;
        
    case Field::TYPE_DOUBLE_PRECISION:
    {
        std::stringstream val;
        val << value.doublefloat;

        if (value.doublefloat / (int64_t)value.doublefloat == 1)
        {
            val << ".0";
        }

        int32_t len=val.str().size();
        pgoutint32(len, outmsg);
        outmsg.append(val.str().c_str());
    }
    break;
        
    case Field::TYPE_FLOAT:
    {
        std::stringstream val;
        val << value.doublefloat;

        if (value.doublefloat / (int64_t)value.doublefloat == 1)
        {
            val << ".0";
        }

        int32_t len=val.str().size();
        pgoutint32(len, outmsg);
        outmsg.append(val.str().c_str());
    }
    break;
        
    case Field::TYPE_CHARACTER:
        pgoutint32(field.size, outmsg);
        if (field.size==1)
        {
            outmsg.append(1, value.character);
        }
        else
        {
            outmsg.append(*value.str);
        }
        break;
        
    case Field::TYPE_CHARACTER_VARYING:
        pgoutint32(value.str->size(), outmsg);
        outmsg.append(*value.str);
        break;

        /**
         * @todo bit & bit_varying data types conversion to 1 & 0's
         * 
         */

        /*
    case Field::TYPE_BIT:
        
    case Field::TYPE_BIT_VARYING:
        */

        /**
         * @todo date & time types
         * 
         */

        /*
    case TYPE_DATE:
    case TYPE_TIME:
    case TYPE_TIMESTAMP:
    case TYPE_TIME_WITH_TIME_ZONE:
    case TYPE_TIMESTAMP_WITH_TIME_ZONE:
        */
        
    default:
        LOG("can't output field type" << field.type);
    }    
}

void FieldValue::pgoutint32(int32_t val, std::string &outmsg)
{
    size_t curpos=outmsg.size();
    outmsg.resize(curpos + sizeof(int32_t));
    val=htobe32(val);
    memcpy(&outmsg[curpos], &val, sizeof(int32_t));    
}

void FieldValue::nullify()
{
    deletestr();
    valtype=VAL_NULL;
}

void FieldValue::set(std::string &val)
{
    deletestr();
    valtype=VAL_STRING;
    value.str=new string(val);
}

void FieldValue::set(int16_t val)
{
    deletestr();
    valtype=VAL_POD;
    value.int2=val;
}

void FieldValue::set(int32_t val)
{
    deletestr();
    valtype=VAL_POD;
    value.int4=val;
}

void FieldValue::set(int64_t val)
{
    deletestr();
    valtype=VAL_POD;
    value.int8=val;
}

void FieldValue::set(float val)
{
    deletestr();
    valtype=VAL_POD;
    value.singlefloat=val;
}

void FieldValue::set(double val)
{
    deletestr();
    valtype=VAL_POD;
    value.doublefloat=val;
}

void FieldValue::set(char val)
{
    deletestr();
    valtype=VAL_POD;
    value.character=val;
}

void FieldValue::settrue()
{
    deletestr();
    valtype=VAL_POD;
    value.boolean=true;
}

void FieldValue::setfalse()
{
    deletestr();
    valtype=VAL_POD;
    value.boolean=false;
}

void FieldValue::convert(Field &field)
{
    switch (valtype)
    {
    case VAL_NULL:
        break;

    case VAL_STRING:
        switch (field.type)
        {
        case Field::TYPE_REAL:
        {
            float singlefloat=std::stof(*value.str, NULL);
            deletestr();
            valtype=VAL_POD;
            value.singlefloat=singlefloat;
        }
        break;

        case Field::TYPE_DOUBLE_PRECISION:
        {
            float doublefloat=std::stod(*value.str, NULL);
            deletestr();
            valtype=VAL_POD;
            value.doublefloat=doublefloat;
        }
        break;
        
        case Field::TYPE_FLOAT:
        {
            float doublefloat=std::stod(*value.str, NULL);
            deletestr();
            valtype=VAL_POD;
            value.doublefloat=doublefloat;
        }
        break;

        default:
            ;
        }

    case VAL_POD:
        switch (field.type)
        {
        case Field::TYPE_REAL:
        {
            float singlefloat=value.int8;
            value.singlefloat=singlefloat;
        }
        break;
        
        case Field::TYPE_DOUBLE_PRECISION:
        {
            float doublefloat=value.int8;
            value.doublefloat=doublefloat;
        }
        break;

        case Field::TYPE_FLOAT:
        {
            float doublefloat=value.int8;
            value.doublefloat=doublefloat;
        }
        break;

        default:
            ;
        }
        

    default:
        LOG("no way to convert type " << valtype);
    }
}

int16_t FieldValue::get(int16_t *val, bool *isnull)
{
    if (valtype==VAL_NULL)
    {
        *isnull=true;
        return 0;
    }
    *isnull=false;
    *val=value.int2;
    return value.int2;
}

int32_t FieldValue::get(int32_t *val, bool *isnull)
{
    if (valtype==VAL_NULL)
    {
        *isnull=true;
        return 0;
    }
    *isnull=false;
    *val=value.int4;
    return value.int4;
}

int64_t FieldValue::get(int64_t *val, bool *isnull)
{
    if (valtype==VAL_NULL)
    {
        *isnull=true;
        return 0;
    }
    *isnull=false;
    *val=value.int8;
    return value.int8;
}

float FieldValue::get(float *val, bool *isnull)
{
    if (valtype==VAL_NULL)
    {
        *isnull=true;
        return 0;
    }
    *isnull=false;
    *val=value.singlefloat;
    return value.singlefloat;
}

double FieldValue::get(double *val, bool *isnull)
{
    if (valtype==VAL_NULL)
    {
        *isnull=true;
        return 0;
    }
    *isnull=false;
    *val=value.doublefloat;
    return value.doublefloat;
}

char FieldValue::get(char *val, bool *isnull)
{
    if (valtype==VAL_NULL)
    {
        *isnull=true;
        return (char)0;
    }
    *isnull=false;
    *val=value.character;
    return value.character;
}

void FieldValue::get(std::string &val, bool *isnull)
{
    if (valtype==VAL_NULL)
    {
        *isnull=true;
    }
    *isnull=false;
    val=*value.str;
}

bool FieldValue::getbool(bool *isnull)
{
    if (valtype==VAL_NULL)
    {
        *isnull=true;
        return false;
    }
    *isnull=false;
    return value.boolean;
}

bool FieldValue::getnull()
{
    if (valtype==VAL_NULL)
    {
        return true;
    }
    return false;
}

void FieldValue::ser(Serdes &output)
{
    switch (valtype)
    {
    case VAL_NONE:
        output.ser((char)valtype);
        break;

    case VAL_POD:
        output.ser((char)valtype);
        output.ser(value.int8);
        break;

    case VAL_NULL:
        output.ser((char)valtype);
        break;

    case VAL_STRING:
    {
        output.ser((char)valtype);
        size_t s=value.str->size();
        output.ser((int64_t)s);
        output.ser(*value.str);
    }
    break;

    default:
        LOG("can't serialize type " << valtype);
    }
}

size_t FieldValue::sersize()
{
    switch (valtype)
    {
    case VAL_NONE:
        return 1;
        break;

    case VAL_POD:
        return 1+sizeof(value.int8);
        break;

    case VAL_NULL:
        return 1;
        break;

    case VAL_STRING:
    {
        return 1+sizeof(int64_t)+value.str->size();
    }
    break;

    default:
        LOG("can't get size of type " << valtype);
    }

    return 0;
}

void FieldValue::des(Serdes &input)
{
    input.des((char *)&valtype);
    
    switch (valtype)
    {
    case VAL_NONE:
        break;

    case VAL_POD:
        input.des(&value.int8);
        break;

    case VAL_NULL:
        break;

    case VAL_STRING:
    {
        int64_t s;
        input.des(&s);
        input.des(*value.str);
    }
    break;

    default:
        LOG("can't deserialize type " << valtype);
    }
}
