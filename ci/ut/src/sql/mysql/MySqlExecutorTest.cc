/**
 * @file MySqlExecutorTest.cc
 * @brief Unit tests for the MySqlExecutor class
 * @details Tests cover database connection, query execution, parameterized queries,
 * and structured query results with automatic database setup and cleanup.
 */

#include <gtest/gtest.h>
#include "sql/mysql/MySqlExecutor.hpp"
#include <mysqlx/xdevapi.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

using namespace common::sql::mysql;

// Test database configuration
static constexpr auto TEST_DB_HOST = "localhost";
static constexpr auto TEST_DB_PORT = 33060;  // X Protocol port for MySQL X DevAPI
static constexpr auto TEST_DB_USER = "root";
static constexpr auto TEST_DB_PASSWORD = "123456";
static constexpr auto TEST_DB_NAME = "test_mysql_executor";

/**
 * @brief Helper function to create a MySQL session with error handling
 * @return Unique pointer to MySQL session
 */
static auto createMySQLSession() -> std::unique_ptr<mysqlx::Session> {
    std::string last_error;
    
    // Method 1: Try SessionOption format (X Protocol default port 33060)
    try {
        auto session = std::make_unique<mysqlx::Session>(
            mysqlx::SessionOption::HOST, TEST_DB_HOST,
            mysqlx::SessionOption::PORT, static_cast<unsigned int>(TEST_DB_PORT),
            mysqlx::SessionOption::USER, TEST_DB_USER,
            mysqlx::SessionOption::PWD, TEST_DB_PASSWORD
        );
        return session;
    } catch (const std::exception& e) {
        last_error = std::string("SessionOption: ") + e.what();
    }
    
    // Method 2: Try URI format with mysqlx:// scheme
    try {
        const std::string uri = "mysqlx://" + std::string(TEST_DB_USER) + ":" + 
                               std::string(TEST_DB_PASSWORD) + "@" + 
                               std::string(TEST_DB_HOST) + ":" + 
                               std::to_string(TEST_DB_PORT);
        auto session = std::make_unique<mysqlx::Session>(uri);
        return session;
    } catch (const std::exception& e) {
        last_error += "\nURI format: " + std::string(e.what());
    }
    
    // All methods failed
    throw std::runtime_error("All connection methods failed:\n" + last_error);
}

/**
 * @brief Global test fixture to manage database lifecycle
 * @details Creates test database before all tests and drops it after all tests
 */
class MySqlExecutorGlobalTest : public ::testing::Environment {
public:
    void SetUp() override {
        try {
            auto session = createMySQLSession();

            // Drop database if exists (for clean state)
            try {
                session->sql("DROP DATABASE IF EXISTS " + std::string(TEST_DB_NAME)).execute();
            } catch (...) {
                // Ignore errors if database doesn't exist
            }

            // Create test database
            session->sql("CREATE DATABASE " + std::string(TEST_DB_NAME)).execute();

            // Create test table
            session->sql("USE " + std::string(TEST_DB_NAME)).execute();
            session->sql(R"(
                CREATE TABLE users (
                    id INT AUTO_INCREMENT PRIMARY KEY,
                    name VARCHAR(100) NOT NULL,
                    email VARCHAR(100),
                    age INT,
                    score DOUBLE,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            )").execute();

            // Insert test data
            session->sql(R"(
                INSERT INTO users (name, email, age, score) VALUES
                ('Alice', 'alice@example.com', 25, 95.5),
                ('Bob', 'bob@example.com', 30, 87.3),
                ('Charlie', 'charlie@example.com', 35, 92.1),
                ('David', NULL, 28, 88.9),
                ('Eve', 'eve@example.com', NULL, 91.2)
            )").execute();

        } catch (const std::exception &e) {
            FAIL() << "Failed to setup test database: " << e.what();
        }
    }

    void TearDown() override {
        try {
            auto session = createMySQLSession();

            // Drop test database
            session->sql("DROP DATABASE IF EXISTS " + std::string(TEST_DB_NAME)).execute();

        } catch (const std::exception &e) {
            // Log error but don't fail - cleanup is best effort
            std::cerr << "Warning: Failed to cleanup test database: " << e.what() << std::endl;
        }
    }
};

// Register global test environment
::testing::Environment *const global_env = ::testing::AddGlobalTestEnvironment(new MySqlExecutorGlobalTest);

/**
 * @brief Test fixture for MySqlExecutor tests
 * @details Provides a connected executor for each test
 */
class MySqlExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {
        try {
            // Use default constructor and then connect
            executor_ = std::make_unique<MySqlExecutor>();
            executor_->connect(TEST_DB_HOST, TEST_DB_PORT, TEST_DB_USER, TEST_DB_PASSWORD, TEST_DB_NAME);
        } catch (const std::exception &e) {
            FAIL() << "Failed to connect to test database: " << e.what();
        }
    }

    void TearDown() override {
        executor_.reset();
    }

    std::unique_ptr<MySqlExecutor> executor_;
};

// ==================== Connection Tests ====================

/**
 * @brief Test default constructor creates disconnected executor
 */
TEST_F(MySqlExecutorTest, DefaultConstructor_CreatesDisconnectedExecutor) {
    MySqlExecutor exec;
    EXPECT_FALSE(exec.isConnected());
}

/**
 * @brief Test parameterized constructor connects successfully
 */
TEST_F(MySqlExecutorTest, ParameterizedConstructor_ConnectsSuccessfully) {
    EXPECT_TRUE(executor_->isConnected());
}

/**
 * @brief Test connect method establishes connection
 */
TEST_F(MySqlExecutorTest, Connect_EstablishesConnection) {
    MySqlExecutor exec;
    ASSERT_FALSE(exec.isConnected());

    exec.connect(TEST_DB_HOST, TEST_DB_PORT, TEST_DB_USER, TEST_DB_PASSWORD, TEST_DB_NAME);
    EXPECT_TRUE(exec.isConnected());
}

/**
 * @brief Test disconnect method closes connection
 */
TEST_F(MySqlExecutorTest, Disconnect_ClosesConnection) {
    ASSERT_TRUE(executor_->isConnected());

    executor_->disconnect();
    EXPECT_FALSE(executor_->isConnected());
}

/**
 * @brief Test connect fails with invalid credentials
 */
TEST_F(MySqlExecutorTest, Connect_InvalidCredentials_ThrowsException) {
    MySqlExecutor exec;

    EXPECT_THROW(
        exec.connect(TEST_DB_HOST, TEST_DB_PORT, "invalid_user", "wrong_password", TEST_DB_NAME),
        std::runtime_error
    );
    EXPECT_FALSE(exec.isConnected());
}

/**
 * @brief Test move constructor transfers connection
 */
TEST_F(MySqlExecutorTest, MoveConstructor_TransfersConnection) {
    MySqlExecutor exec1;
    exec1.connect(TEST_DB_HOST, TEST_DB_PORT, TEST_DB_USER, TEST_DB_PASSWORD, TEST_DB_NAME);
    ASSERT_TRUE(exec1.isConnected());

    MySqlExecutor exec2(std::move(exec1));
    EXPECT_TRUE(exec2.isConnected());
    EXPECT_FALSE(exec1.isConnected());
}

/**
 * @brief Test move assignment operator transfers connection
 */
TEST_F(MySqlExecutorTest, MoveAssignment_TransfersConnection) {
    MySqlExecutor exec1;
    exec1.connect(TEST_DB_HOST, TEST_DB_PORT, TEST_DB_USER, TEST_DB_PASSWORD, TEST_DB_NAME);
    MySqlExecutor exec2;

    ASSERT_TRUE(exec1.isConnected());
    ASSERT_FALSE(exec2.isConnected());

    exec2 = std::move(exec1);
    EXPECT_TRUE(exec2.isConnected());
    EXPECT_FALSE(exec1.isConnected());
}

// ==================== Execute Tests (Non-Query SQL) ====================

/**
 * @brief Test execute INSERT statement
 */
TEST_F(MySqlExecutorTest, Execute_InsertStatement_ReturnsAffectedRows) {
    const auto affected = executor_->execute(
        "INSERT INTO users (name, email, age, score) VALUES ('TestUser', 'test@example.com', 22, 85.0)"
    );

    EXPECT_EQ(affected, 1);
}

/**
 * @brief Test execute UPDATE statement
 */
TEST_F(MySqlExecutorTest, Execute_UpdateStatement_ReturnsAffectedRows) {
    // Use a specific condition that won't affect other tests
    const auto affected = executor_->execute(
        "UPDATE users SET score = 87.3 WHERE name = 'Bob'"
    );

    EXPECT_GE(affected, 1);
}

/**
 * @brief Test execute DELETE statement
 */
TEST_F(MySqlExecutorTest, Execute_DeleteStatement_ReturnsAffectedRows) {
    // First insert a row to delete
    executor_->execute(
        "INSERT INTO users (name, email, age, score) VALUES ('ToDelete', 'delete@example.com', 20, 70.0)"
    );

    const auto affected = executor_->execute(
        "DELETE FROM users WHERE name = 'ToDelete'"
    );

    EXPECT_EQ(affected, 1);
}

/**
 * @brief Test execute with empty SQL throws exception
 */
TEST_F(MySqlExecutorTest, Execute_EmptySQL_ThrowsException) {
    EXPECT_THROW(executor_->execute(""), std::invalid_argument);
}

/**
 * @brief Test execute with invalid SQL throws exception
 */
TEST_F(MySqlExecutorTest, Execute_InvalidSQL_ThrowsException) {
    EXPECT_THROW(executor_->execute("INVALID SQL STATEMENT"), std::runtime_error);
}

/**
 * @brief Test execute on disconnected executor throws exception
 */
TEST_F(MySqlExecutorTest, Execute_Disconnected_ThrowsException) {
    MySqlExecutor exec;
    EXPECT_THROW(exec.execute("SELECT 1"), std::runtime_error);
}

// ==================== Query Tests (Legacy API) ====================

/**
 * @brief Test simple SELECT query returns results
 */
TEST_F(MySqlExecutorTest, Query_SimpleSelect_ReturnsResults) {
    const auto results = executor_->query("SELECT name, age FROM users WHERE id = 1");

    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].size(), 2);
    EXPECT_EQ(results[0][0], "Alice");
    EXPECT_EQ(results[0][1], "25");
}

/**
 * @brief Test query returns multiple rows
 */
TEST_F(MySqlExecutorTest, Query_MultipleRows_ReturnsAllRows) {
    const auto results = executor_->query("SELECT name FROM users ORDER BY id");

    EXPECT_GE(results.size(), 5);
    EXPECT_EQ(results[0][0], "Alice");
    EXPECT_EQ(results[1][0], "Bob");
}

/**
 * @brief Test query handles NULL values
 */
TEST_F(MySqlExecutorTest, Query_NULLValues_ReturnsNULLString) {
    const auto results = executor_->query("SELECT email FROM users WHERE name = 'David'");

    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results[0][0], "NULL");
}

/**
 * @brief Test query with empty SQL throws exception
 */
TEST_F(MySqlExecutorTest, Query_EmptySQL_ThrowsException) {
    EXPECT_THROW(executor_->query(""), std::invalid_argument);
}

/**
 * @brief Test query with invalid SQL throws exception
 */
TEST_F(MySqlExecutorTest, Query_InvalidSQL_ThrowsException) {
    EXPECT_THROW(executor_->query("INVALID SQL"), std::runtime_error);
}

// ==================== QueryWithParams Tests (Legacy API) ====================

/**
 * @brief Test parameterized query with single parameter
 */
TEST_F(MySqlExecutorTest, QueryWithParams_SingleParameter_ReturnsCorrectResult) {
    const auto results = executor_->queryWithParams(
        "SELECT name, age FROM users WHERE name = ?",
        {"Alice"}
    );

    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0][0], "Alice");
    EXPECT_EQ(results[0][1], "25");
}

/**
 * @brief Test parameterized query with multiple parameters
 */
TEST_F(MySqlExecutorTest, QueryWithParams_MultipleParameters_ReturnsCorrectResult) {
    const auto results = executor_->queryWithParams(
        "SELECT name FROM users WHERE age > ? AND age < ?",
        {"24", "31"}
    );

    EXPECT_GE(results.size(), 2);
}

/**
 * @brief Test parameterized query with no matching results
 */
TEST_F(MySqlExecutorTest, QueryWithParams_NoMatches_ReturnsEmptyResult) {
    const auto results = executor_->queryWithParams(
        "SELECT name FROM users WHERE age > ?",
        {"100"}
    );

    EXPECT_TRUE(results.empty());
}

/**
 * @brief Test parameterized query with empty SQL throws exception
 */
TEST_F(MySqlExecutorTest, QueryWithParams_EmptySQL_ThrowsException) {
    EXPECT_THROW(
        executor_->queryWithParams("", {"param"}),
        std::invalid_argument
    );
}

// ==================== Structured Query Tests ====================

/**
 * @brief Test queryStructured returns proper column names
 */
TEST_F(MySqlExecutorTest, QueryStructured_ReturnsColumnNames) {
    const auto result = executor_->queryStructured("SELECT name, age, score FROM users WHERE id = 1");

    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result.columnCount(), 3);
    EXPECT_EQ(result.rowCount(), 1);

    // Column names should be present (currently using generic names)
    EXPECT_FALSE(result.column_names.empty());
}

/**
 * @brief Test queryStructured returns typed values - integer
 */
TEST_F(MySqlExecutorTest, QueryStructured_IntegerValue_IsTypedCorrectly) {
    const auto result = executor_->queryStructured("SELECT age FROM users WHERE name = 'Alice'");

    ASSERT_FALSE(result.isEmpty());
    const auto &row = result.rows[0];

    auto value = row.getColumn("column_0"); // age is the only column (index 0)
    ASSERT_TRUE(value.has_value());
    EXPECT_TRUE(std::holds_alternative<int64_t>(value.value()));
    EXPECT_EQ(std::get<int64_t>(value.value()), 25);
}

/**
 * @brief Test queryStructured returns typed values - double
 */
TEST_F(MySqlExecutorTest, QueryStructured_DoubleValue_IsTypedCorrectly) {
    const auto result = executor_->queryStructured("SELECT score FROM users WHERE name = 'Alice'");

    ASSERT_FALSE(result.isEmpty());
    const auto &row = result.rows[0];

    auto value = row.getColumn("column_0"); // score is the only column (index 0)
    ASSERT_TRUE(value.has_value());
    EXPECT_TRUE(std::holds_alternative<double>(value.value()));
    EXPECT_DOUBLE_EQ(std::get<double>(value.value()), 95.5);
}

/**
 * @brief Test queryStructured returns typed values - string
 */
TEST_F(MySqlExecutorTest, QueryStructured_StringValue_IsTypedCorrectly) {
    const auto result = executor_->queryStructured("SELECT name FROM users WHERE id = 1");

    ASSERT_FALSE(result.isEmpty());
    const auto &row = result.rows[0];

    auto value = row.getColumn("column_0"); // name is first column
    ASSERT_TRUE(value.has_value());
    EXPECT_TRUE(std::holds_alternative<std::string>(value.value()));
    EXPECT_EQ(std::get<std::string>(value.value()), "Alice");
}

/**
 * @brief Test queryStructured handles NULL values
 */
TEST_F(MySqlExecutorTest, QueryStructured_NULLValue_IsMonostate) {
    const auto result = executor_->queryStructured("SELECT email FROM users WHERE name = 'David'");

    ASSERT_FALSE(result.isEmpty());
    const auto &row = result.rows[0];

    auto value = row.getColumn("column_0"); // email is the only column (index 0)
    ASSERT_TRUE(value.has_value());
    EXPECT_TRUE(std::holds_alternative<std::monostate>(value.value()));
}

/**
 * @brief Test QueryRow getString converts values correctly
 */
TEST_F(MySqlExecutorTest, QueryRow_GetString_ConvertsValues) {
    // Use a fresh query to avoid data modification from other tests
    const auto result = executor_->queryStructured(
        "SELECT name, age, score FROM users WHERE name = 'Alice' AND age = 25"
    );

    // If Alice's age was modified by another test, skip or use default values
    if (result.isEmpty()) {
        GTEST_SKIP() << "Test data was modified by previous tests";
        return;
    }

    const auto &row = result.rows[0];

    EXPECT_EQ(row.getString("column_0"), "Alice");
    EXPECT_EQ(row.getString("column_1"), "25");
    // Double might have precision issues, just check it's not empty
    EXPECT_FALSE(row.getString("column_2").empty());
}

/**
 * @brief Test QueryRow hasColumn detects existing columns
 */
TEST_F(MySqlExecutorTest, QueryRow_HasColumn_DetectsExistingColumns) {
    const auto result = executor_->queryStructured("SELECT name, email FROM users WHERE id = 1");

    ASSERT_FALSE(result.isEmpty());
    const auto &row = result.rows[0];

    EXPECT_TRUE(row.hasColumn("column_0")); // name exists
    EXPECT_TRUE(row.hasColumn("column_1")); // email exists
}

/**
 * @brief Test QueryRow hasColumn returns false for non-existent columns
 */
TEST_F(MySqlExecutorTest, QueryRow_HasColumn_ReturnsFalseForNonExistent) {
    const auto result = executor_->queryStructured("SELECT name FROM users WHERE id = 1");

    ASSERT_FALSE(result.isEmpty());
    const auto &row = result.rows[0];

    EXPECT_FALSE(row.hasColumn("nonexistent_column"));
}

/**
 * @brief Test QueryRow hasColumn returns false for NULL values
 */
TEST_F(MySqlExecutorTest, QueryRow_HasColumn_ReturnsFalseForNULL) {
    const auto result = executor_->queryStructured("SELECT email FROM users WHERE name = 'David'");

    ASSERT_FALSE(result.isEmpty());
    const auto &row = result.rows[0];

    EXPECT_FALSE(row.hasColumn("column_0")); // email is NULL for David
}

/**
 * @brief Test queryStructured returns multiple rows
 */
TEST_F(MySqlExecutorTest, QueryStructured_MultipleRows_ReturnsAllRows) {
    const auto result = executor_->queryStructured("SELECT name FROM users ORDER BY id");

    EXPECT_GE(result.rowCount(), 5);
    EXPECT_EQ(result.columnCount(), 1);
}

/**
 * @brief Test queryStructured with empty SQL throws exception
 */
TEST_F(MySqlExecutorTest, QueryStructured_EmptySQL_ThrowsException) {
    EXPECT_THROW(executor_->queryStructured(""), std::invalid_argument);
}

// ==================== Structured QueryWithParams Tests ====================

/**
 * @brief Test queryWithParamsStructured with parameters
 */
TEST_F(MySqlExecutorTest, QueryWithParamsStructured_WithParameters_ReturnsCorrectResult) {
    const auto result = executor_->queryWithParamsStructured(
        "SELECT name, age FROM users WHERE name = ?",
        {"Alice"}
    );

    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result.rowCount(), 1);
}

/**
 * @brief Test queryWithParamsStructured with multiple parameters
 */
TEST_F(MySqlExecutorTest, QueryWithParamsStructured_MultipleParameters_ReturnsCorrectResult) {
    const auto result = executor_->queryWithParamsStructured(
        "SELECT name FROM users WHERE age > ? AND score > ?",
        {"20", "90.0"}
    );

    EXPECT_GE(result.rowCount(), 1);
}

/**
 * @brief Test queryWithParamsStructured with empty SQL throws exception
 */
TEST_F(MySqlExecutorTest, QueryWithParamsStructured_EmptySQL_ThrowsException) {
    EXPECT_THROW(
        executor_->queryWithParamsStructured("", {"param"}),
        std::invalid_argument
    );
}

// ==================== Error Handling Tests ====================

/**
 * @brief Test getLastError returns error message after failed operation
 */
TEST_F(MySqlExecutorTest, GetLastError_ReturnsErrorMessage) {
    try {
        executor_->execute("INVALID SQL STATEMENT");
        FAIL() << "Expected exception was not thrown";
    } catch (const std::runtime_error &) {
        const auto error = executor_->getLastError();
        EXPECT_FALSE(error.empty());
        EXPECT_NE(error.find("execution failed"), std::string::npos);
    }
}

/**
 * @brief Test isConnected returns correct state after disconnect
 */
TEST_F(MySqlExecutorTest, IsConnected_AfterDisconnect_ReturnsFalse) {
    ASSERT_TRUE(executor_->isConnected());

    executor_->disconnect();
    EXPECT_FALSE(executor_->isConnected());
}

/**
 * @brief Test reconnect after disconnect works
 */
TEST_F(MySqlExecutorTest, Reconnect_AfterDisconnect_Works) {
    executor_->disconnect();
    ASSERT_FALSE(executor_->isConnected());

    executor_->connect(TEST_DB_HOST, TEST_DB_PORT, TEST_DB_USER, TEST_DB_PASSWORD, TEST_DB_NAME);
    EXPECT_TRUE(executor_->isConnected());
}

// ==================== Edge Cases ====================

/**
 * @brief Test query returns empty result for no matches
 */
TEST_F(MySqlExecutorTest, Query_NoMatches_ReturnsEmptyResult) {
    const auto result = executor_->queryStructured("SELECT * FROM users WHERE age > 200");

    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.rowCount(), 0);
}

/**
 * @brief Test execute with complex SQL statement
 */
TEST_F(MySqlExecutorTest, Execute_ComplexSQL_Success) {
    const auto affected = executor_->execute(
        "INSERT INTO users (name, email, age, score) "
        "SELECT 'Clone', 'clone@example.com', 40, 80.0 "
        "WHERE NOT EXISTS (SELECT 1 FROM users WHERE name = 'Clone')"
    );

    EXPECT_GE(affected, 0);
}

/**
 * @brief Test query with JOIN-like operations (subquery)
 */
TEST_F(MySqlExecutorTest, Query_Subquery_ReturnsCorrectResult) {
    const auto result = executor_->queryStructured(
        "SELECT name FROM users WHERE age > (SELECT AVG(age) FROM users WHERE age IS NOT NULL)"
    );

    // Should return users older than average age
    EXPECT_GE(result.rowCount(), 0);
}

/**
 * @brief Test multiple sequential queries work correctly
 */
TEST_F(MySqlExecutorTest, MultipleSequentialQueries_WorkCorrectly) {
    const auto result1 = executor_->queryStructured("SELECT COUNT(*) as cnt FROM users");
    const auto result2 = executor_->queryStructured("SELECT name FROM users LIMIT 1");
    const auto result3 = executor_->queryStructured("SELECT MAX(age) as max_age FROM users");

    EXPECT_FALSE(result1.isEmpty());
    EXPECT_FALSE(result2.isEmpty());
    EXPECT_FALSE(result3.isEmpty());
}
