#include <limits>
#include <gtest/gtest.h>
#include "decimal/decimal.h"

TEST(DecimalTest, CreateFromInt32) {
	decimal d{10};
	ASSERT_EQ(10, d.to_int32());
}

TEST(DecimalTest, CreateFromString) {
	std::string s{"105.12"};
	decimal d{s};

	ASSERT_EQ(s, d.to_string());
}

TEST(DecimalTest, Add) {
	std::string s1{"1.2"};
	std::string s2{"2.9"};
	std::string s3{"4.1"};

	decimal d1{s1};
	decimal d2{s2};
	decimal d3{d1+d2};

	ASSERT_EQ(s3, d3.to_string());
}

TEST(DecimalTest, Subtract) {
	std::string s1{"1.2"};
	std::string s2{"2.9"};
	std::string s3{"4.1"};

	decimal d1{s3};
	decimal d2{s2};
	decimal d3{d1-d2};

	ASSERT_EQ(s1, d3.to_string());
}

TEST(DecimalTest, Multiply) {
	std::string s1{"1.2"};
	std::string s2{"2.9"};
	std::string s3{"3.48"};

	decimal d1{s1};
	decimal d2{s2};
	decimal d3{d1*d2};

	ASSERT_EQ(s3, d3.to_string());
}

TEST(DecimalTest, Divide) {
	std::string s1{"1.2"};
	std::string s2{"2.9"};
	std::string s3{"3.48"};

	decimal d1{s3};
	decimal d2{s2};
	decimal d3{d1/d2};

	ASSERT_EQ(s1, d3.to_string());
}



