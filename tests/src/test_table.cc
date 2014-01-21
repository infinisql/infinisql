#include <limits>
#include <gtest/gtest.h>
#include "engine/Table.h"


TEST(TableTest, Create) {
	Table t;
}

TEST(TableTest, GetNextFieldId) {
	Table t;

	ASSERT_EQ(0, t.getnextfieldid());
	ASSERT_EQ(1, t.getnextfieldid());
	ASSERT_EQ(2, t.getnextfieldid());
}
