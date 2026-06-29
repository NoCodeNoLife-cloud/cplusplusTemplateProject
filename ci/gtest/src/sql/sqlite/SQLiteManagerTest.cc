/**
 * @file SQLiteManagerTest.cc
 * @brief Unit tests for the SQLiteManager class
 * @details Tests cover database lifecycle, query execution, parameterized queries,
 *          and edge cases with automatic file cleanup.
 */

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include <cppforge/sql/sqlite/SQLiteManager.hpp>

using namespace cppforge::sql::sqlite;

static constexpr auto TEST_DB_NAME = "test_sqlite_manager.db";

/**
 * @brief Returns a temporary path for the test database
 */
static auto testDbPath() -> std::string
{
    return std::filesystem::temp_directory_path().append(TEST_DB_NAME).string();
}

/**
 * @brief Global test environment for database lifecycle management
 */
class SQLiteGlobalTest : public testing::Environment
{
public:
    void SetUp() override
    {
        // Delete any leftover from a previous interrupted run
        std::filesystem::remove(testDbPath());
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove(testDbPath(), ec);
    }
};

testing::Environment* const sqlite_global_env =
    ::testing::AddGlobalTestEnvironment(new SQLiteGlobalTest);

/**
 * @brief Test fixture providing a fresh SQLiteManager for each test
 */
class SQLiteManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_NO_THROW({
            mgr_.createDatabase(testDbPath());
        });
        ASSERT_TRUE(mgr_.isOpen());
    }

    void TearDown() override
    {
        mgr_.closeDatabase();
    }

    SQLiteManager mgr_;
};

// ==================== Connection/Disconnection Tests ====================

/**
 * @brief Test that default-constructed SQLiteManager is not open
 * @details Creates a default SQLiteManager instance and verifies that isOpen()
 *          returns false before any database is created
 */
TEST_F(SQLiteManagerTest, DefaultConstructor_IsNotOpen)
{
    SQLiteManager m;
    EXPECT_FALSE(m.isOpen());
}

/**
 * @brief Test that parameterized constructor opens the database
 * @details Creates a SQLiteManager with a database path and verifies that the
 *          connection is open immediately after construction
 */
TEST_F(SQLiteManagerTest, ParameterizedConstructor_OpensDatabase)
{
    SQLiteManager m(testDbPath());
    EXPECT_TRUE(m.isOpen());
    m.closeDatabase();
}

/**
 * @brief Test createDatabase opens a new connection
 * @details Creates a default SQLiteManager, then calls createDatabase with a
 *          valid path and verifies the connection is open
 */
TEST_F(SQLiteManagerTest, CreateDatabase_OpensConnection)
{
    SQLiteManager m;
    ASSERT_FALSE(m.isOpen());

    m.createDatabase(testDbPath());
    EXPECT_TRUE(m.isOpen());
}

/**
 * @brief Test closeDatabase properly closes the connection
 * @details Opens a database, then closes it and verifies that isOpen()
 *          returns false after the close operation
 */
TEST_F(SQLiteManagerTest, CloseDatabase_ClosesConnection)
{
    ASSERT_TRUE(mgr_.isOpen());
    mgr_.closeDatabase();
    EXPECT_FALSE(mgr_.isOpen());
}

/**
 * @brief Test createDatabase with empty path throws invalid_argument
 * @details Verifies that calling createDatabase with an empty string path
 *          throws a std::invalid_argument exception
 */
TEST_F(SQLiteManagerTest, CreateDatabase_EmptyPath_Throws)
{
    SQLiteManager m;
    EXPECT_THROW(m.createDatabase(""), std::invalid_argument);
}

// ==================== exec() Tests ====================

/**
 * @brief Test exec() can create a table successfully
 * @details Executes a CREATE TABLE statement via exec() and verifies that
 *          no exception is thrown
 */
TEST_F(SQLiteManagerTest, Exec_CreateTable_Success)
{
    EXPECT_NO_THROW(
        mgr_.exec("CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, val TEXT)")
    );
}

/**
 * @brief Test exec() INSERT returns correct affected row count
 * @details Creates a table, inserts a row via exec(), and verifies that the
 *          return value indicates exactly one row was affected
 */
TEST_F(SQLiteManagerTest, Exec_Insert_ReturnsAffectedRows)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS items (id INTEGER PRIMARY KEY, name TEXT)");
    const auto affected = mgr_.exec(
        "INSERT INTO items (name) VALUES ('item1')"
    );
    EXPECT_EQ(affected, 1);
}

/**
 * @brief Test exec() with parameterized INSERT returns affected row count
 * @details Creates a table, inserts a row using parameterized query syntax,
 *          and verifies that the return value indicates exactly one row affected
 */
TEST_F(SQLiteManagerTest, Exec_InsertWithParams_ReturnsAffectedRows)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS products (id INTEGER PRIMARY KEY, name TEXT)");
    const auto affected = mgr_.exec(
        "INSERT INTO products (name) VALUES (?)",
        {"widget"}
    );
    EXPECT_EQ(affected, 1);
}

/**
 * @brief Test exec() UPDATE returns correct affected row count
 * @details Creates a table, inserts a row, performs an UPDATE via exec(),
 *          and verifies that exactly one row was affected
 */
TEST_F(SQLiteManagerTest, Exec_Update_ReturnsAffectedRows)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS scores (id INTEGER PRIMARY KEY, score INTEGER)");
    mgr_.exec("INSERT INTO scores (score) VALUES (100)");
    const auto affected = mgr_.exec("UPDATE scores SET score = 200 WHERE id = 1");
    EXPECT_EQ(affected, 1);
}

/**
 * @brief Test exec() DELETE returns correct affected row count
 * @details Creates a table, inserts a row, performs a DELETE via exec(),
 *          and verifies that exactly one row was affected
 */
TEST_F(SQLiteManagerTest, Exec_Delete_ReturnsAffectedRows)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS tempdata (id INTEGER PRIMARY KEY, val TEXT)");
    mgr_.exec("INSERT INTO tempdata (val) VALUES ('delete_me')");
    const auto affected = mgr_.exec("DELETE FROM tempdata WHERE val = 'delete_me'");
    EXPECT_EQ(affected, 1);
}

/**
 * @brief Test exec() with empty SQL throws invalid_argument
 * @details Verifies that calling exec() with an empty string throws
 *          a std::invalid_argument exception
 */
TEST_F(SQLiteManagerTest, Exec_EmptySQL_Throws)
{
    EXPECT_THROW(mgr_.exec(""), std::invalid_argument);
}

/**
 * @brief Test exec() with invalid SQL throws runtime_error
 * @details Verifies that calling exec() with a malformed SQL statement throws
 *          a std::runtime_error exception
 */
TEST_F(SQLiteManagerTest, Exec_InvalidSQL_Throws)
{
    EXPECT_THROW(mgr_.exec("NOT VALID SQL"), std::runtime_error);
}

/**
 * @brief Test exec() on a disconnected manager throws runtime_error
 * @details Creates a default (not connected) SQLiteManager and verifies that
 *          calling exec() throws a std::runtime_error
 */
TEST_F(SQLiteManagerTest, Exec_Disconnected_Throws)
{
    SQLiteManager m;
    EXPECT_THROW(m.exec("SELECT 1"), std::runtime_error);
}

// ==================== query() Tests ====================

/**
 * @brief Test query() with a simple SELECT returns correct results
 * @details Creates a table, inserts a row, then queries it and verifies that
 *          the returned result matches the inserted data
 */
TEST_F(SQLiteManagerTest, Query_SimpleSelect_ReturnsResults)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)");
    mgr_.exec("INSERT INTO users (name) VALUES ('Alice')");

    const auto results = mgr_.query("SELECT name FROM users WHERE id = 1");
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].size(), 1);
    EXPECT_EQ(results[0][0], "Alice");
}

/**
 * @brief Test query() returns all rows for a multi-row result set
 * @details Inserts multiple rows and verifies that query() returns all of them
 *          in the correct order
 */
TEST_F(SQLiteManagerTest, Query_MultipleRows_ReturnsAll)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS items2 (id INTEGER PRIMARY KEY, name TEXT)");
    mgr_.exec("INSERT INTO items2 (name) VALUES ('A')");
    mgr_.exec("INSERT INTO items2 (name) VALUES ('B')");
    mgr_.exec("INSERT INTO items2 (name) VALUES ('C')");

    const auto results = mgr_.query("SELECT name FROM items2 ORDER BY id");
    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(results[0][0], "A");
    EXPECT_EQ(results[1][0], "B");
    EXPECT_EQ(results[2][0], "C");
}

/**
 * @brief Test query() with a single parameter returns correct row
 * @details Uses a parameterized query with one placeholder to filter results,
 *          verifying that the correct row is returned
 */
TEST_F(SQLiteManagerTest, Query_WithParams_ReturnsCorrectRow)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS employees (id INTEGER PRIMARY KEY, name TEXT)");
    mgr_.exec("INSERT INTO employees (name) VALUES ('Bob')");

    const auto results = mgr_.query("SELECT name FROM employees WHERE name = ?", {"Bob"});
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results[0][0], "Bob");
}

/**
 * @brief Test query() with multiple parameters returns correct row
 * @details Uses a parameterized query with multiple placeholders and verifies
 *          that the correct row is returned based on combined filter conditions
 */
TEST_F(SQLiteManagerTest, Query_MultipleParams_ReturnsCorrectRow)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS t (a INTEGER, b INTEGER)");
    mgr_.exec("INSERT INTO t VALUES (1, 10)");
    mgr_.exec("INSERT INTO t VALUES (2, 20)");

    const auto results = mgr_.query("SELECT a FROM t WHERE a > ? AND b < ?", {"0", "15"});
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0][0], "1");
}

/**
 * @brief Test query() returns empty result when no rows match
 * @details Executes a SELECT with a condition that matches no rows and verifies
 *          that the returned result set is empty
 */
TEST_F(SQLiteManagerTest, Query_NoMatch_ReturnsEmpty)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS empty_test (id INTEGER PRIMARY KEY)");
    const auto results = mgr_.query("SELECT * FROM empty_test WHERE id > 100");
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Test query() returns "NULL" string for SQL NULL values
 * @details Inserts a row with a NULL column value and verifies that query()
 *          returns the string "NULL" to represent the SQL NULL
 */
TEST_F(SQLiteManagerTest, Query_NULLValues_ReturnsNULLString)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS nullable (id INTEGER PRIMARY KEY, val TEXT)");
    mgr_.exec("INSERT INTO nullable (id) VALUES (1)");

    const auto results = mgr_.query("SELECT val FROM nullable WHERE id = 1");
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results[0][0], "NULL");
}

/**
 * @brief Test query() with empty SQL throws invalid_argument
 * @details Verifies that calling query() with an empty string throws
 *          a std::invalid_argument exception
 */
TEST_F(SQLiteManagerTest, Query_EmptySQL_Throws)
{
    EXPECT_THROW(mgr_.query(""), std::invalid_argument);
}

/**
 * @brief Test query() with invalid SQL throws runtime_error
 * @details Verifies that calling query() with a malformed SQL statement throws
 *          a std::runtime_error exception
 */
TEST_F(SQLiteManagerTest, Query_InvalidSQL_Throws)
{
    EXPECT_THROW(mgr_.query("BAD SQL HERE"), std::runtime_error);
}

/**
 * @brief Test query() on a disconnected manager throws runtime_error
 * @details Creates a default (not connected) SQLiteManager and verifies that
 *          calling query() throws a std::runtime_error
 */
TEST_F(SQLiteManagerTest, Query_Disconnected_Throws)
{
    SQLiteManager m;
    EXPECT_THROW(m.query("SELECT 1"), std::runtime_error);
}

// ==================== Move Semantics ====================

/**
 * @brief Test move constructor transfers database ownership
 * @details Creates a connected SQLiteManager, moves it to a new instance via
 *          move constructor, and verifies that the new instance owns the
 *          connection while the original is closed
 */
TEST_F(SQLiteManagerTest, MoveConstructor_TransfersOwnership)
{
    SQLiteManager m1(testDbPath());
    ASSERT_TRUE(m1.isOpen());

    SQLiteManager m2(std::move(m1));
    EXPECT_TRUE(m2.isOpen());
    EXPECT_FALSE(m1.isOpen());
}

/**
 * @brief Test move assignment transfers database ownership
 * @details Creates two SQLiteManager instances, moves a connected one into
 *          a default one via move assignment, and verifies that the target
 *          now owns the connection while the source is closed
 */
TEST_F(SQLiteManagerTest, MoveAssignment_TransfersOwnership)
{
    SQLiteManager m1(testDbPath());
    ASSERT_TRUE(m1.isOpen());

    SQLiteManager m2;
    ASSERT_FALSE(m2.isOpen());

    m2 = std::move(m1);
    EXPECT_TRUE(m2.isOpen());
    EXPECT_FALSE(m1.isOpen());
}

// ==================== Edge Cases ====================

/**
 * @brief Test multiple sequential queries on the same connection
 * @details Executes several INSERT and SELECT operations in sequence on the
 *          same database connection and verifies that all queries return
 *          correct results
 */
TEST_F(SQLiteManagerTest, MultipleSequentialQueries_Work)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS seq (id INTEGER PRIMARY KEY, val TEXT)");
    mgr_.exec("INSERT INTO seq (val) VALUES ('a')");
    mgr_.exec("INSERT INTO seq (val) VALUES ('b')");

    const auto r1 = mgr_.query("SELECT val FROM seq WHERE id = 1");
    const auto r2 = mgr_.query("SELECT val FROM seq WHERE id = 2");

    ASSERT_EQ(r1.size(), 1);
    ASSERT_EQ(r2.size(), 1);
    EXPECT_EQ(r1[0][0], "a");
    EXPECT_EQ(r2[0][0], "b");
}

/**
 * @brief Test exec() with a complex SQL expression
 * @details Executes a CREATE TABLE, INSERT, and a complex UPDATE with an
 *          arithmetic expression via exec(), verifying that the operation
 *          succeeds and returns the correct affected row count
 */
TEST_F(SQLiteManagerTest, Exec_ComplexSQL_Success)
{
    mgr_.exec("CREATE TABLE IF NOT EXISTS complex (id INTEGER PRIMARY KEY, cnt INTEGER)");
    mgr_.exec("INSERT INTO complex (cnt) VALUES (10)");

    const auto affected = mgr_.exec(
        "UPDATE complex SET cnt = cnt + 1 WHERE cnt = 10"
    );
    EXPECT_EQ(affected, 1);
}
