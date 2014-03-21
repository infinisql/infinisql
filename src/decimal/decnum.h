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
 * @file   decimal.h
 * @author Christopher Nelson <nadiasvertex@gmail.com>
 * @date   Tue January 21, 2014
 *
 * @brief  decimal math wrapper for the decNumber C library
 */
#ifndef INFINISQL_DECIMAL_H
#define INFINISQL_DECIMAL_H

#include <memory>
#include <string>

extern "C" {
	// This value is a compile time constant, and determines what maximum
	// precision can be supported at runtime.
	#define DECNUMDIGITS 100
	#include "decNumber.h"
}

class decimal {
public:
	typedef std::shared_ptr<decContext> context_type;
	typedef decNumber number_type;
protected:
	context_type context;
	number_type number;

	void _initialize_context(int precision);

public:
	decimal();
	decimal(const decimal *other);
	decimal(context_type context);
	decimal(int32_t value);
	decimal(int32_t value, int precision);
	decimal(const std::string& value);
	decimal(const std::string& value, int precision);
	virtual ~decimal();

	std::string to_string() const;
	void from_string(const std::string& value);

	int32_t to_int32() const;

	bool isnan() const;
	decimal abs() const;
	decimal exp() const;
	decimal invert() const;
	decimal ln() const;
	decimal logB() const;
	decimal log10() const;
	decimal max() const;
	decimal min() const;
	decimal power() const;

	decimal operator+(const decimal& rhs) const;
	decimal operator-(const decimal& rhs) const;
	decimal operator*(const decimal& rhs) const;
	decimal operator/(const decimal& rhs) const;
	decimal operator&(const decimal& rhs) const;
	decimal operator|(const decimal& rhs) const;
	decimal operator^(const decimal& rhs) const;
	decimal operator%(const decimal& rhs) const;
	decimal operator<<(const decimal& rhs) const;

	int compare(const decimal& rhs) const;
	bool operator<(const decimal& rhs) const;
	bool operator>(const decimal& rhs) const;
	bool operator<=(const decimal& rhs) const;
	bool operator>=(const decimal& rhs) const;
	bool operator==(const decimal& rhs) const;
	bool operator!=(const decimal& rhs) const;

};


#endif //INFINISQL_DECIMAL_H
