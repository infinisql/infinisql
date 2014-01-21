/*
 * Copyright (c) 2013 Christopher Nelson <nadiasvertex@gmail.com>
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
 * @file   decimal.cc
 * @author Christopher Nelson <nadiasvertex@gmail.com>
 * @date   Tue January 21, 2014
 *
 * @brief  decimal math wrapper for the decNumber C library
 */

#include "decimal.h"

void
decimal::_initialize_context(int precision) {
	context = std::make_shared<decContext>();
	decContextDefault(context.get(), DEC_INIT_BASE);
	context->traps = 0;
	context->digits = precision;
}

decimal::decimal(const std::string& value, int precision) {
	_initialize_context(precision);
	decNumberFromString(&number, value.c_str(), context.get());
}

decimal::decimal(const std::string& value) :
		decimal(value, DECNUMDIGITS) {
}

decimal::decimal(int32_t value, int precision) {
	_initialize_context(precision);
	decNumberFromInt32(&number, value);
}

decimal::decimal(int32_t value) :
		decimal(value, DECNUMDIGITS) {
}

decimal::decimal(context_type context):context(context) {
}

decimal::decimal() :
		decimal(0) {
}

std::string
decimal::to_string() const {
	// As per the decNumber spec.
	char buffer[context->digits+14];
	decNumberToString(&number, buffer);
	return std::string{buffer};
}

int32_t
decimal::to_int32() const {
	return decNumberToInt32(&number, context.get());
}

bool
decimal::isnan() const {
	return decNumberIsNaN(&number) == 1;
}

int
decimal::compare(const decimal& rhs) const {
	number_type r;
	decNumberCompare(&r, &number, &rhs.number, context.get());
	return decNumberToInt32(&r, context.get());
}

decimal
decimal::operator+(const decimal& rhs) const {
	decimal r{context};
	decNumberAdd(&r.number, &number, &rhs.number, context.get());
	return r;
}

decimal
decimal::operator-(const decimal& rhs) const {
	decimal r{context};
	decNumberSubtract(&r.number, &number, &rhs.number, context.get());
	return r;
}

decimal
decimal::operator*(const decimal& rhs) const {
	decimal r{context};
	decNumberMultiply(&r.number, &number, &rhs.number, context.get());
	return r;
}

decimal
decimal::operator/(const decimal& rhs) const {
	decimal r{context};
	decNumberDivide(&r.number, &number, &rhs.number, context.get());
	return r;
}

bool
decimal::operator<(const decimal& rhs) const {
	return compare(rhs)<0;
}

bool decimal::operator>(const decimal& rhs) const{
	return compare(rhs)>0;
}

bool decimal::operator<=(const decimal& rhs) const{
	return compare(rhs)<=0;
}

bool decimal::operator>=(const decimal& rhs) const{
	return compare(rhs)>=0;
}

bool decimal::operator==(const decimal& rhs) const {
	return compare(rhs)==0;
}

bool decimal::operator!=(const decimal& rhs) const {
	return compare(rhs)!=0;
}
