//============================================================================
// Name        : J2735MessageTest.cpp
// Description : Unit tests for the J2735 Message library.
//============================================================================

#include <gtest/gtest.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <database/DbConnectionPool.h>

using namespace std;
using namespace tmx;
using namespace tmx::utils;

namespace unit_test {

class DBConnectionTest: public testing::Test {

protected:
	DBConnectionTest() {
	}

	virtual ~DBConnectionTest() {
	}

public:
	DbConnectionPool _pool;
};

TEST_F(DBConnectionTest, PoolTest) {
	DbConnection conn1 = _pool.Connection();

	ASSERT_EQ("tcp://127.0.0.1:3306", conn1.GetConnectionUrl());
	ASSERT_EQ("", conn1.GetDatabase());
	ASSERT_TRUE(conn1.Get());
	ASSERT_FALSE(conn1.Get()->isClosed());

	ASSERT_EQ(1, _pool.ActiveSize());
	ASSERT_EQ(1, _pool.NumConnections());

	// Run a query
	{
		sql::Statement *stmt1 = conn1.Get()->createStatement();
		sql::ResultSet *rs1 = stmt1->executeQuery("SELECT version();");

		ASSERT_TRUE(rs1->next());
		ASSERT_GT(rs1->getString(1).compare("5.5.52"), 0);

		rs1->close();
		stmt1->close();

		delete rs1;
		delete stmt1;
		rs1 = NULL;
		stmt1 = NULL;
	}

	DbConnection conn2 = _pool.Connection();
	ASSERT_EQ(conn1.GetConnectionUrl(), conn2.GetConnectionUrl());
	ASSERT_EQ(conn1.GetDatabase(), conn2.GetDatabase());
	ASSERT_TRUE(conn2.Get());
	ASSERT_FALSE(conn2.Get()->isClosed());
	ASSERT_NE(conn1.Get(), conn2.Get());

	ASSERT_EQ(2, _pool.ActiveSize());
	ASSERT_EQ(2, _pool.NumConnections());

	{
		sql::Statement *stmt2 = conn2.Get()->createStatement();
		sql::ResultSet *rs2 = stmt2->executeQuery("SELECT version();");

		ASSERT_TRUE(rs2->next());
		ASSERT_GT(rs2->getString(1).compare("5.5.52"), 0);

		rs2->close();
		stmt2->close();

		delete rs2;
		delete stmt2;
		rs2 = NULL;
		stmt2 = NULL;
	}

	// Make sure we can recycle unused connections
	{
		DbConnection conn3 = _pool.Connection();

		ASSERT_EQ(3, _pool.ActiveSize());
		ASSERT_EQ(3, _pool.NumConnections());
	}

	ASSERT_EQ(2, _pool.ActiveSize());
	ASSERT_EQ(3, _pool.NumConnections());


}

TEST_F(DBConnectionTest, ConnTest) {
	string url = "tcp://localhost:3306";
	DbConnection conn1(url);
	ASSERT_TRUE(conn1.Get());
	ASSERT_FALSE(conn1->isClosed());

	// Run a query
	{
		sql::Statement *stmt1 = conn1->createStatement();
		sql::ResultSet *rs1 = stmt1->executeQuery("SELECT version();");

		ASSERT_TRUE(rs1->next());
		ASSERT_GT(rs1->getString(1).compare("5.5.52"), 0);

		rs1->close();
		stmt1->close();

		delete rs1;
		delete stmt1;
		rs1 = NULL;
		stmt1 = NULL;
	}

	// Try a different database
	conn1.SetDatabase("IVP");

	{
		sql::Statement *stmt1 = conn1.Get()->createStatement();
		sql::ResultSet *rs1 = stmt1->executeQuery("SELECT version();");

		ASSERT_TRUE(rs1->next());
		ASSERT_GT(rs1->getString(1).compare("5.5.52"), 0);

		rs1->close();
		stmt1->close();

		delete rs1;
		delete stmt1;
		rs1 = NULL;
		stmt1 = NULL;
	}

	conn1->close();
	ASSERT_TRUE(conn1.Reconnect());

	// Make sure the two connections are separate
	DbConnection conn2(url);
	ASSERT_TRUE(conn1.Get());
	ASSERT_FALSE(conn1->isClosed());

	{
		sql::Statement *stmt1 = conn2->createStatement();
		sql::ResultSet *rs1 = stmt1->executeQuery("SELECT version();");

		ASSERT_TRUE(rs1->next());
		ASSERT_GT(rs1->getString(1).compare("5.5.52"), 0);

		rs1->close();
		stmt1->close();

		delete rs1;
		delete stmt1;
		rs1 = NULL;
		stmt1 = NULL;
	}

	ASSERT_FALSE(conn2.Reconnect());
}

}
