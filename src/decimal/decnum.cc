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

#include "decnum.h"

void
decimal::_initialize_context(int precision) {
	context = std::make_shared<decContext>();
	decContextDefault(context.get(), DEC_INIT_BASE);
	context->traps = 0;
	context->digits = precision;
}

decimal::decimal(const std::string& value, int precision) {
	_initialize_context(precision);
	from_string(value);
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

decimal::decimal(const decimal *other):context(other->context) {
	decNumberCopy(&number, &other->number);
}

decimal::decimal() :
		decimal(0) {
}

decimal::~decimal() {
}

std::string
decimal::to_string() const {
	// As per the decNumber spec.
	char buffer[context->digits+14];
	decNumberToString(&number, buffer);
	return std::string{buffer};
}

void
decimal::from_string(const std::string& value) {
	decNumberFromString(&number, value.c_str(), context.get());
}

int32_t
decimal::to_int32() const {
	return decNumberToInt32(&number, context.get());
}

bool
decimal::isnan() const {
	return decNumberIsNaN(&number) == 1;
}

decimal
decimal::abs() const {
	decimal r{context};
	decNumberAbs(&r.number, &number, context.get());
	return r;
}

decimal
decimal::exp() const {
	decimal r{context};
	decNumberExp(&r.number, &number, context.get());
	return r;
}

decimal
decimal::invert() const {
	decimal r{context};
	decNumberInvert(&r.number, &number, context.get());
	return r;
}

decimal
decimal::ln() const {
	decimal r{context};
	decNumberLn(&r.number, &number, context.get());
	return r;
}

/* ------------------------------------------------------------------ */
/* logB -- get adjusted exponent, by 754 rules                        */
/*   This computes and returns adjustedexponent(number)               */
/* ------------------------------------------------------------------ */
decimal
decimal::logB() const {
	decimal r{context};
	decNumberLogB(&r.number, &number, context.get());
	return r;
}

decimal
decimal::log10() const {
	decimal r{context};
	decNumberLog10(&r.number, &number, context.get());
	return r;
}

decimal
decimal::max(const decimal& rhs) const {
	decimal r{context};
	decNumberMax(&r.number, &number, &rhs.number, context.get());
	return r;
}

decimal
decimal::min(const decimal& rhs) const {
	decimal r{context};
	decNumberMin(&r.number, &number, &rhs.number, context.get());
	return r;
}

/* ------------------------------------------------------------------ */
/* power -- raise a number to a power                                 */
/*   This computes and returns number ** rhs                          */
/* ------------------------------------------------------------------ */
decimal
decimal::power(const decimal& rhs) const {
	decimal r{context};
	decNumberPower(&r.number, &number, &rhs.number, context.get());
	return r;
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

decimal
decimal::operator&(const decimal& rhs) const {
	decimal r{context};
	decNumberAnd(&r.number, &number, &rhs.number, context.get());
	return r;
}

decimal
decimal::operator|(const decimal& rhs) const {
	decimal r{context};
	decNumberOr(&r.number, &number, &rhs.number, context.get());
	return r;
}

/* ------------------------------------------------------------------ */
/* operator ^ -- XOR two Numbers, digitwise                           */
/*   This computes and returns number ^ rhs                           */
/* ------------------------------------------------------------------ */
decimal
decimal::operator^(const decimal& rhs) const {
	decimal r{context};
	decNumberXor(&r.number, &number, &rhs.number, context.get());
	return r;
}

decimal
decimal::operator%(const decimal& rhs) const {
	decimal r{context};
	decNumberRemainder(&r.number, &number, &rhs.number, context.get());
	return r;
}

decimal
decimal::operator<<(const decimal& rhs) const {
	decimal r{context};
	decNumberShift(&r.number, &number, &rhs.number, context.get());
	return r;
}

int
decimal::compare(const decimal& rhs) const {
	number_type r;
	decNumberCompare(&r, &number, &rhs.number, context.get());
	return decNumberToInt32(&r, context.get());
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
