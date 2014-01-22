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
 * @author Mark Travis <mtravis15432DE+src@gmail.com>
 * @date   Tue Jan  7 02:51:34 2014
 * 
 * @brief  classes pertaining to data fields
 */

#include "Field.h"
#include "Table.h"
#include "decimal/decnum.h"
#line 32 "Field.cc"

FieldValue::FieldValue() :
		valtype(VAL_NONE) {
	value.int8 = 0;
}

FieldValue::FieldValue(const FieldValue &orig) {
	cp(orig);
}

FieldValue &FieldValue::operator=(const FieldValue &orig) {
	cp(orig);
	return *this;
}

void FieldValue::cp(const FieldValue &orig) {
	valtype = orig.valtype;
	switch (valtype) {
	default:
		break;
	case VAL_POD:
		value = orig.value;
		break;
	case VAL_STRING:
		value.str = new std::string { *orig.value.str };
		break;
	case VAL_DECIMAL:
		value.dec = new decimal { orig.value.dec };
		break;
	}
}

FieldValue::~FieldValue() {
	cleanup_pointers();
}

void FieldValue::cleanup_pointers() {
	switch (valtype) {
	default:
		break;
	case VAL_STRING:
		delete value.str;
		break;
	case VAL_DECIMAL:
		delete value.dec;
		break;
	}
}

void FieldValue::nullify() {
	cleanup_pointers();
	valtype = VAL_NULL;
}

void FieldValue::set(const std::string &val) {
	cleanup_pointers();
	valtype = VAL_STRING;
	value.str = new std::string { val };
}

void FieldValue::set(const decimal &val) {
	cleanup_pointers();
	valtype = VAL_DECIMAL;
	value.dec = new decimal { val };
}

void FieldValue::set(int8_t val) {
	cleanup_pointers();
	valtype = VAL_POD;
	value.int1 = val;
}

void FieldValue::set(int16_t val) {
	cleanup_pointers();
	valtype = VAL_POD;
	value.int2 = val;
}

void FieldValue::set(int32_t val) {
	cleanup_pointers();
	valtype = VAL_POD;
	value.int4 = val;
}

void FieldValue::set(int64_t val) {
	cleanup_pointers();
	valtype = VAL_POD;
	value.int8 = val;
}

void FieldValue::set(float val) {
	cleanup_pointers();
	valtype = VAL_POD;
	value.singlefloat = val;
}

void FieldValue::set(double val) {
	cleanup_pointers();
	valtype = VAL_POD;
	value.doublefloat = val;
}

void FieldValue::set(char val) {
	cleanup_pointers();
	valtype = VAL_POD;
	value.character = val;
}

void FieldValue::set(bool val) {

	cleanup_pointers();
	valtype = VAL_POD;
	value.boolean = val;
}

void FieldValue::settrue() {
	cleanup_pointers();
	valtype = VAL_POD;
	value.boolean = true;
}

void FieldValue::setfalse() {
	cleanup_pointers();
	valtype = VAL_POD;
	value.boolean = false;
}

int8_t FieldValue::get(int8_t &val, bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
		return 0;
	}
	isnull = false;
	val = value.int1;
	return value.int1;
}

int16_t FieldValue::get(int16_t &val, bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
		return 0;
	}
	isnull = false;
	val = value.int2;
	return value.int2;
}

int32_t FieldValue::get(int32_t &val, bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
		return 0;
	}
	isnull = false;
	val = value.int4;
	return value.int4;
}

int64_t FieldValue::get(int64_t &val, bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
		return 0;
	}
	isnull = false;
	val = value.int8;
	return value.int8;
}

float FieldValue::get(float &val, bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
		return 0;
	}
	isnull = false;
	val = value.singlefloat;
	return value.singlefloat;
}

double FieldValue::get(double &val, bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
		return 0;
	}
	isnull = false;
	val = value.doublefloat;
	return value.doublefloat;
}

char FieldValue::get(char &val, bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
		return (char) 0;
	}
	isnull = false;
	val = value.character;
	return value.character;
}

void FieldValue::get(std::string &val, bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
	}
	isnull = false;
	if (value.str == nullptr) {
		return;
	}
	val = *value.str;
}

void FieldValue::get(decimal &val, bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
	}
	isnull = false;
	if (value.dec == nullptr) {
		return;
	}
	val = *value.dec;
}

void FieldValue::get(bool &val, bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
	}
	isnull = false;
	val = value.boolean;
}

bool FieldValue::getbool(bool &isnull) {
	if (valtype == VAL_NULL) {
		isnull = true;
		return false;
	}
	isnull = false;
	return value.boolean;
}

bool FieldValue::getnull() {
	if (valtype == VAL_NULL) {
		return true;
	}
	return false;
}

void FieldValue::ser(Serdes &output) {
	switch (valtype) {
	case VAL_NONE:
		output.ser((char) valtype);
		break;

	case VAL_POD:
		output.ser((char) valtype);
		output.ser(value.int8);
		break;

	case VAL_NULL:
		output.ser((char) valtype);
		break;

	case VAL_STRING: {
		output.ser((char) valtype);
		size_t s = value.str->size();
		output.ser((int64_t) s);
		output.ser(*value.str);
	}
		break;

	case VAL_DECIMAL: {
		output.ser((char) valtype);
		const std::string st { value.dec->to_string() };
		size_t s = st.size();
		output.ser((int64_t) s);
		output.ser(st);
	}
		;
		break;

	default:
		LOG("can't serialize type " << valtype);
	}
}

size_t FieldValue::sersize() {
	switch (valtype) {
	case VAL_NONE:
		return 1;
		break;

	case VAL_POD:
		return 1 + sizeof(value.int8);
		break;

	case VAL_NULL:
		return 1;
		break;

	case VAL_STRING: {
		return 1 + sizeof(int64_t) + value.str->size();
	}
		break;

	case VAL_DECIMAL: {
		return 1 + sizeof(int64_t) + value.dec->to_string().size();
	}
		break;

	default:
		LOG("can't get size of type " << valtype);
	}

	return 0;
}

void FieldValue::des(Serdes &input) {
	cleanup_pointers();
	input.des((char &) valtype);

	switch (valtype) {
	case VAL_NONE:
		break;

	case VAL_POD:
		input.des(value.int8);
		break;

	case VAL_NULL:
		break;

	case VAL_STRING: {
		int64_t s;
		value.str = new std::string { };
		input.des(s);
		input.des(*(value.str));
	}
		break;

	case VAL_DECIMAL: {
		int64_t s;
		std::string st;
		input.des(s);
		input.des(st);
		value.dec = new decimal { st };
	}
		break;

	default:
		LOG("can't deserialize type " << valtype);
	}
}

Field::Field() :
		Metadata(), type(TYPE_NONE), size(-1), precision(-1), scale(-1), nullconstraint(
				false) {
	defaultValue.nullify();
}

Field::Field(std::shared_ptr<Table> parentTablearg, const std::string& namearg,
		type_e typearg) :
		Field() {
	type = typearg;

	if (initializer(parentTablearg, namearg) == false) {
		return;
	}

	switch (type) {
	case TYPE_NUMERIC:
		precision = 100;
		scale = 0;
		break;

	case TYPE_DECIMAL:
		precision = 100;
		scale = 0;
		break;

	case TYPE_CHARACTER:
		size = 1;
		break;

	case TYPE_BIT:
		size = 1;
		break;

	case TYPE_TIME:
		precision = 6;
		break;

	case TYPE_TIMESTAMP:
		precision = 6;
		break;

	case TYPE_TIME_WITH_TIME_ZONE:
		precision = 6;
		break;

	case TYPE_TIMESTAMP_WITH_TIME_ZONE:
		precision = 6;
		break;

	default:
		LOG("type " << type << " doesn't take 0 arguments");
	}
}

Field::Field(std::shared_ptr<Table> parentTablearg, const std::string& namearg,
		type_e typearg, int64_t arg1arg) :
		Field() {
	type = typearg;

	if (initializer(parentTablearg, namearg) == false) {
		return;
	}

	switch (type) {
	case TYPE_NUMERIC:
		precision = arg1arg;
		scale = 0;
		break;

	case TYPE_DECIMAL:
		precision = arg1arg;
		scale = 0;
		break;

	case TYPE_CHARACTER:
		size = arg1arg;
		break;

	case TYPE_CHARACTER_VARYING:
		size = arg1arg;
		break;

	case TYPE_BIT:
		size = arg1arg;
		break;

	case TYPE_BIT_VARYING:
		size = arg1arg;
		break;

	case TYPE_TIME:
		precision = arg1arg;
		break;

	case TYPE_TIMESTAMP:
		precision = arg1arg;
		break;

	case TYPE_TIME_WITH_TIME_ZONE:
		precision = arg1arg;
		break;

	case TYPE_TIMESTAMP_WITH_TIME_ZONE:
		precision = arg1arg;
		break;

	default:
		LOG("type " << type << " doesn't take a single argument");
	}
}

Field::Field(std::shared_ptr<Table> parentTablearg, const std::string& namearg,
		type_e typearg, int64_t arg1arg, int64_t arg2arg) :
		Field() {
	type = typearg;
	precision = arg1arg;
	scale = arg2arg;

	if (initializer(parentTablearg, namearg) == false) {
		return;
	}
}

Field::Field(const Field &orig) :
		Metadata(orig) {
	cp(orig);
}

Field &Field::operator=(const Field &orig) {
	(Metadata) *this = Metadata(orig);
	cp(orig);
	return *this;
}

void Field::cp(const Field &orig) {
	type = orig.type;
	size = orig.size;
	precision = orig.precision;
	scale = orig.scale;
	defaultValue = orig.defaultValue;
	nullconstraint = orig.nullconstraint;
}

Field::~Field() {

}

bool Field::initializer(std::shared_ptr<Table> parentTablearg,
		const std::string& namearg) {
	if (parentTablearg->fieldName2Id.count(namearg)) {
		id = -1;
		return false;
	}
	parentTable = parentTablearg;
	getparents();
	id = parentTable->getnextfieldid();
	name = namearg;
	parentTable->fieldName2Id[name] = id;
	parentTable->fieldid2Field[id] = this;
	defaultValue.nullify();

	return true;
}

void Field::getparents() {
	parentCatalog = parentTable->parentCatalog;
	parentSchema = parentTable->parentSchema;
	parentcatalogid = parentTable->parentcatalogid;
	parentschemaid = parentTable->parentschemaid;
	parenttableid = parentTable->id;
}

void Field::ser(Serdes &output) {
	Metadata::ser(output);
	output.ser((char) type);
	output.ser((int64_t) size);
	output.ser((int64_t) precision);
	output.ser((int64_t) scale);
	defaultValue.ser(output);
}

size_t Field::sersize() {
	size_t retsize = Metadata::sersize();
	retsize += Serdes::sersize((char) type);
	retsize += Serdes::sersize((int64_t) size);
	retsize += Serdes::sersize((int64_t) precision);
	retsize += Serdes::sersize((int64_t) scale);
	retsize += defaultValue.sersize();

	return retsize;
}

void Field::des(Serdes &input) {
	Metadata::des(input);
	input.des((char &) type);
	input.des((int64_t &) size);
	input.des((int64_t &) precision);
	input.des((int64_t &) scale);
	defaultValue.des(input);
}

void Field::serValue(FieldValue &fieldValue, Serdes &output) {
	switch (type) {
	case TYPE_TINYINT:
		output.ser(fieldValue.value.int1);
		break;

	case TYPE_SMALLINT:
		output.ser(fieldValue.value.int2);
		break;

	case TYPE_INT:
		output.ser(fieldValue.value.int4);
		break;

	case TYPE_BIGINT:
		output.ser(fieldValue.value.int8);
		break;

	case TYPE_BOOLEAN:
		output.ser(fieldValue.value.boolean);
		break;

	case TYPE_NUMERIC:
		output.ser(*(fieldValue.value.dec));
		break;

	case TYPE_DECIMAL:
		output.ser(*(fieldValue.value.dec));
		break;

	case TYPE_REAL:
		output.ser(fieldValue.value.singlefloat);
		break;

	case TYPE_DOUBLE_PRECISION:
		output.ser(fieldValue.value.doublefloat);
		break;

	case TYPE_FLOAT:
		output.ser(fieldValue.value.doublefloat);
		break;

	case TYPE_CHARACTER:
		if (fieldValue.valtype == FieldValue::VAL_POD) {
			output.ser(fieldValue.value.character);
		} else {
			output.ser(*fieldValue.value.str, size);
		}
		break;

	case TYPE_CHARACTER_VARYING: {
		output.ser(*fieldValue.value.str);
	}
		break;

	case TYPE_BIT:
		if (fieldValue.valtype == FieldValue::VAL_POD) {
			output.ser(fieldValue.value.character);
		} else {
			output.ser(*fieldValue.value.str, size);
		}
		break;

	case TYPE_BIT_VARYING: {
		output.ser(*fieldValue.value.str);
	}
		break;

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
		LOG("field type " << type << " not implemented");
	}
}

void Field::desValue(Serdes &input, FieldValue &fieldValue) {
	switch (type) {
	case TYPE_TINYINT:
		fieldValue.valtype = FieldValue::VAL_POD;
		input.des(fieldValue.value.int1);
		break;

	case TYPE_SMALLINT:
		fieldValue.valtype = FieldValue::VAL_POD;
		input.des(fieldValue.value.int2);
		break;

	case TYPE_INT:
		fieldValue.valtype = FieldValue::VAL_POD;
		input.des(fieldValue.value.int4);
		break;

	case TYPE_BIGINT:
		fieldValue.valtype = FieldValue::VAL_POD;
		input.des(fieldValue.value.int8);
		break;

	case TYPE_BOOLEAN:
		fieldValue.valtype = FieldValue::VAL_POD;
		input.des(fieldValue.value.boolean);
		break;

	case TYPE_NUMERIC:
		fieldValue.valtype = FieldValue::VAL_DECIMAL;
		input.des(fieldValue.value.dec);
		break;

	case TYPE_DECIMAL:
		fieldValue.valtype = FieldValue::VAL_DECIMAL;
		input.des(fieldValue.value.dec);
		break;

	case TYPE_REAL:
		fieldValue.valtype = FieldValue::VAL_POD;
		input.des(fieldValue.value.singlefloat);
		break;

	case TYPE_DOUBLE_PRECISION:
		fieldValue.valtype = FieldValue::VAL_POD;
		input.des(fieldValue.value.doublefloat);
		break;

	case TYPE_FLOAT:
		fieldValue.valtype = FieldValue::VAL_POD;
		input.des(fieldValue.value.doublefloat);
		break;

	case TYPE_CHARACTER:
		if (size == 1) {
			fieldValue.valtype = FieldValue::VAL_POD;
			input.des(fieldValue.value.character);
		} else {
			fieldValue.valtype = FieldValue::VAL_STRING;
			input.des(fieldValue.value.str, size);
		}
		break;

	case TYPE_CHARACTER_VARYING: {
		fieldValue.valtype = FieldValue::VAL_STRING;
		input.des(*fieldValue.value.str);
	}
		break;

	case TYPE_BIT:
		if (size <= 8) {
			fieldValue.valtype = FieldValue::VAL_POD;
			input.des(fieldValue.value.character);
		} else {
			fieldValue.valtype = FieldValue::VAL_STRING;
			size_t len = (size + 7) / 8;
			input.des(fieldValue.value.str, len);
		}
		break;

	case TYPE_BIT_VARYING: {
		fieldValue.valtype = FieldValue::VAL_STRING;
		input.des(*fieldValue.value.str);
	}
		break;

		/**
		 * @todo date & time functions, likely use std::date_time
		 *
		 */

		/*
		 // probably use std::date_time 8 bytes without tz
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
		LOG("field type " << type << " not implemented");
	}
}

size_t Field::valueSize(FieldValue &fieldValue) {
	switch (type) {
	case TYPE_TINYINT:
		return sizeof(fieldValue.value.int1);

	case TYPE_SMALLINT:
		return sizeof(fieldValue.value.int2);

	case TYPE_INT:
		return sizeof(fieldValue.value.int4);

	case TYPE_BIGINT:
		return sizeof(fieldValue.value.int8);

	case TYPE_BOOLEAN:
		return sizeof(fieldValue.value.boolean);

		/**
		 * @todo NUMERIC and DECIMAL types
		 *
		 */

		/*
		 case TYPE_NUMERIC:
		 case TYPE_DECIMAL:
		 */
	case TYPE_REAL:
		return sizeof(fieldValue.value.singlefloat);

	case TYPE_DOUBLE_PRECISION:
		return sizeof(fieldValue.value.doublefloat);

	case TYPE_FLOAT:
		return sizeof(fieldValue.value.doublefloat);

	case TYPE_CHARACTER:
		return size;

	case TYPE_CHARACTER_VARYING:
		return fieldValue.value.str->size();

	case TYPE_BIT:
		return (size + 7) / 8;

	case TYPE_BIT_VARYING:
		return fieldValue.value.str->size();

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
		LOG("can't get size of field type" << type);
	}

	return -1;
}

void Field::pgoutput(FieldValue &fieldValue, std::string &outmsg) {
	if (fieldValue.valtype == FieldValue::VAL_NULL) {
		pgoutint32(-1, outmsg);
		return;
	}

	switch (type) {
	case TYPE_TINYINT: {
		char val[5]; // length of largest int8_t plus - and \n
		int32_t len = sprintf(val, "%i", fieldValue.value.int1);
		pgoutint32(len, outmsg);
		outmsg.append(val);
	}
		break;

	case TYPE_SMALLINT: {
		char val[7];  // length of largest int16_t plus - and \n
		int32_t len = sprintf(val, "%i", fieldValue.value.int2);
		pgoutint32(len, outmsg);
		outmsg.append(val);
	}
		break;

	case TYPE_INT: {
		char val[12]; // length of largest int32_t plus - and \n
		int32_t len = sprintf(val, "%i", fieldValue.value.int4);
		pgoutint32(len, outmsg);
		outmsg.append(val);
	}
		break;

	case TYPE_BIGINT: {
		char val[21]; // length of largest int64_t plus - and \n
		int32_t len = sprintf(val, "%li", fieldValue.value.int8);
		pgoutint32(len, outmsg);
		outmsg.append(val);
	}
		break;

	case TYPE_BOOLEAN:
		pgoutint32((int32_t) sizeof(fieldValue.value.boolean), outmsg);
		if (fieldValue.value.boolean == true) {
			outmsg.append(1, 't');
		} else {
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
	case TYPE_REAL: {
		std::stringstream val;
		val << fieldValue.value.singlefloat;

		if (fieldValue.value.singlefloat
				/ (int64_t) fieldValue.value.singlefloat == 1) {
			val << ".0";
		}

		int32_t len = val.str().size();
		pgoutint32(len, outmsg);
		outmsg.append(val.str().c_str());
	}
		break;

	case TYPE_DOUBLE_PRECISION: {
		std::stringstream val;
		val << fieldValue.value.doublefloat;

		if (fieldValue.value.doublefloat
				/ (int64_t) fieldValue.value.doublefloat == 1) {
			val << ".0";
		}

		int32_t len = val.str().size();
		pgoutint32(len, outmsg);
		outmsg.append(val.str().c_str());
	}
		break;

	case TYPE_FLOAT: {
		std::stringstream val;
		val << fieldValue.value.doublefloat;

		if (fieldValue.value.doublefloat
				/ (int64_t) fieldValue.value.doublefloat == 1) {
			val << ".0";
		}

		int32_t len = val.str().size();
		pgoutint32(len, outmsg);
		outmsg.append(val.str().c_str());
	}
		break;

	case TYPE_CHARACTER:
		pgoutint32(size, outmsg);
		if (size == 1) {
			outmsg.append(1, fieldValue.value.character);
		} else {
			outmsg.append(*fieldValue.value.str);
		}
		break;

	case TYPE_CHARACTER_VARYING:
		pgoutint32(fieldValue.value.str->size(), outmsg);
		outmsg.append(*fieldValue.value.str);
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
		LOG("can't output field type" << type);
	}
}

void Field::pgoutint32(int32_t val, std::string &outmsg) {
	size_t curpos = outmsg.size();
	outmsg.resize(curpos + sizeof(int32_t));
	val = htobe32(val);
	memcpy(&outmsg[curpos], &val, sizeof(int32_t));
}

void Field::convertValue(FieldValue &fieldValue) {
	switch (fieldValue.valtype) {
	case FieldValue::VAL_NULL:
		break;

	case FieldValue::VAL_DECIMAL:
		switch (type) {
		case TYPE_REAL: {
			float singlefloat = std::stof(fieldValue.value.dec->to_string(), nullptr);
			fieldValue.cleanup_pointers();
			fieldValue.valtype = FieldValue::VAL_POD;
			fieldValue.value.singlefloat = singlefloat;
		}
			break;

		case TYPE_DOUBLE_PRECISION: {
			float doublefloat = std::stod(fieldValue.value.dec->to_string(), nullptr);
			fieldValue.cleanup_pointers();
			fieldValue.valtype = FieldValue::VAL_POD;
			fieldValue.value.doublefloat = doublefloat;
		}
			break;

		case TYPE_FLOAT: {
			float doublefloat = std::stod(fieldValue.value.dec->to_string(), nullptr);
			fieldValue.cleanup_pointers();
			fieldValue.valtype = FieldValue::VAL_POD;
			fieldValue.value.doublefloat = doublefloat;
		}
			break;

		default:
			;
		}
		break;

	case FieldValue::VAL_STRING:
		switch (type) {
		case TYPE_REAL: {
			float singlefloat = std::stof(*fieldValue.value.str, nullptr);
			fieldValue.cleanup_pointers();
			fieldValue.valtype = FieldValue::VAL_POD;
			fieldValue.value.singlefloat = singlefloat;
		}
			break;

		case TYPE_DOUBLE_PRECISION: {
			float doublefloat = std::stod(*fieldValue.value.str, nullptr);
			fieldValue.cleanup_pointers();
			fieldValue.valtype = FieldValue::VAL_POD;
			fieldValue.value.doublefloat = doublefloat;
		}
			break;

		case TYPE_FLOAT: {
			float doublefloat = std::stod(*fieldValue.value.str, nullptr);
			fieldValue.cleanup_pointers();
			fieldValue.valtype = FieldValue::VAL_POD;
			fieldValue.value.doublefloat = doublefloat;
		}
			break;

		default:
			;
		}
		break;

	case FieldValue::VAL_POD:
		switch (type) {
		case TYPE_REAL: {
			float singlefloat = fieldValue.value.int8;
			fieldValue.value.singlefloat = singlefloat;
		}
			break;

		case TYPE_DOUBLE_PRECISION: {
			float doublefloat = fieldValue.value.int8;
			fieldValue.value.doublefloat = doublefloat;
		}
			break;

		case TYPE_FLOAT: {
			float doublefloat = fieldValue.value.int8;
			fieldValue.value.doublefloat = doublefloat;
		}
			break;

		default:
			;
		}
		break;

	default:
		LOG("no way to convert type " << fieldValue.valtype);
	}
}

