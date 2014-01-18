#include <gtest/gtest.h>
#include "Mbox.h"
#include "Schema.h"
#include "Topology.h"
#include "TransactionAgent.h"

class SqlTest: public ::testing::Test {

protected:
	Schema *schema = nullptr;
	Mbox mbox{1};
	Topology topo;
	TransactionAgent *agent = nullptr;
	int epollfd;

	virtual void SetUp() {
		vector<string> nodes;
		vector<string> services;

		schema = new Schema(0);
		epollfd = epoll_create(1);
		/*agent = new TransactionAgent(
		 topo.newActor(
		 ACTOR_LISTENER, &mbox,
		 epollfd, string(),
		 0, nodes,
		 services));*/
	}

	virtual void TearDown() {
		close(epollfd);
		delete schema;
		//delete agent;
	}
};

TEST_F(SqlTest, SelectLiteral) {
	Larxer l("SELECT 1", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, SelectFromLiteralTable) {
	Larxer l("SELECT 1 FROM (1,2,3)", nullptr, schema);
	l.printstack();
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTable) {
	Larxer l("CREATE TABLE test(a INT);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTemporaryTable) {
	Larxer l("CREATE TEMPORARY TABLE test(a INT);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableIfNotExists) {
	Larxer l("CREATE TABLE IF NOT EXISTS test (a INT);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTemporaryTableIfNotExists) {
	Larxer l("CREATE TEMPORARY TABLE IF NOT EXISTS test (a INT);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithAllNumericTypes) {
	Larxer l("CREATE TABLE test(a BIGINT, b INT, c DECIMAL);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithAllDateTypes) {
	Larxer l("CREATE TABLE test(d DATETIME, e DATE, f INTERVAL);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithAllTextTypes) {
	Larxer l("CREATE TABLE test(d CHARACTER VARYING, e TEXT);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithNullConstraint) {
	Larxer l("CREATE TABLE test(a INT NULL);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithNotNullConstraint) {
	Larxer l("CREATE TABLE test(a INT NOT NULL);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithSimpleDefault) {
	Larxer l("CREATE TABLE test(a INT DEFAULT 1);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithComplexDefault) {
	Larxer l("CREATE TABLE test(a INT DEFAULT 5+8);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithUnique) {
	Larxer l("CREATE TABLE test(a INT UNIQUE(a));", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithPrimaryKey) {
	Larxer l("CREATE TABLE test(a INT PRIMARY KEY(a));", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithUniqueSimple) {
	Larxer l("CREATE TABLE test(a INT UNIQUE);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithPrimaryKeySimple) {
	Larxer l("CREATE TABLE test(a INT PRIMARY KEY);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithUniqueCompound) {
	Larxer l("CREATE TABLE test(a INT UNIQUE(a,b), b INT);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithPrimaryKeyCompound) {
	Larxer l("CREATE TABLE test(a INT PRIMARY KEY(a,b), b INT);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithCollation) {
	Larxer l("CREATE TABLE test(a INT COLLATE 'utf8');", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithReferences) {
	Larxer l("CREATE TABLE test(a INT REFERENCES test2(ra));", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithReferencesCompound) {
	Larxer l("CREATE TABLE test(a INT REFERENCES test2(ra,rb), b INT);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableWithReferencesSimple) {
	Larxer l("CREATE TABLE test(a INT REFERENCES test2, b INT);", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableLikeOtherTableSimple) {
	Larxer l("CREATE TABLE test LIKE test2;", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableLikeOtherTable) {
	Larxer l("CREATE TABLE test LIKE test2 INCLUDING ALL;", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableLikeOtherTableExplicitInclude) {
	Larxer l("CREATE TABLE test LIKE test2 INCLUDING DEFAULTS CONSTRAINTS INDEXES STORAGE COMMENTS;", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableLikeOtherTableExcludingAll) {
	Larxer l("CREATE TABLE test LIKE test2 EXCLUDING ALL;", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableLikeOtherTableExplicitExclude) {
	Larxer l("CREATE TABLE test LIKE test2 EXCLUDING DEFAULTS CONSTRAINTS INDEXES STORAGE COMMENTS;", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}

TEST_F(SqlTest, CreateTableLikeOtherTableExplicitIncludeExclude) {
	Larxer l("CREATE TABLE test LIKE test2 INCLUDING ALL EXCLUDING STORAGE;", nullptr, schema);
	EXPECT_NE(nullptr, l.statementPtr);
}
