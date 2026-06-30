/**
 * @file HealthTest.cc
 * @brief Unit tests for the health check subsystem.
 * @details Tests cover health check registration and JSON output format.
 */

#include <string>
#include <gtest/gtest.h>

#include <cppforge/http/IHttpRequest.hpp>
#include <cppforge/http/IHttpResponse.hpp>
#include <cppforge/observability/health/HealthEndpoint.hpp>
#include <cppforge/observability/health/IHealthCheck.hpp>
#include <unordered_map>

using namespace cppforge::http;
using namespace cppforge::observability::health;

namespace
{

/// @brief Mock HTTP request for testing.
class MockHttpRequest : public IHttpRequest
{
  public:
    std::string method() const override { return "GET"; }
    std::string path() const override { return "/health"; }
    std::string header(const std::string & /*key*/) const override { return ""; }
    std::string body() const override { return ""; }
    std::string queryParam(const std::string & /*key*/) const override { return ""; }
};

/// @brief Mock HTTP response for testing.
class MockHttpResponse : public IHttpResponse
{
  public:
    void status(int code) override { statusCode_ = code; }
    void header(const std::string &key, const std::string &value) override { headers_[key] = value; }
    void body(const std::string &content) override { body_ = content; }

    int statusCode_ = 200;
    std::string body_;
    std::unordered_map<std::string, std::string> headers_;
};

/// @brief A health check that always reports healthy.
class AlwaysHealthyCheck : public IHealthCheck
{
  public:
    explicit AlwaysHealthyCheck(std::string name) : name_(std::move(name)) {}
    std::string name() const override { return name_; }
    bool check() const override { return true; }

  private:
    std::string name_;
};

/// @brief A health check that always reports unhealthy.
class AlwaysUnhealthyCheck : public IHealthCheck
{
  public:
    explicit AlwaysUnhealthyCheck(std::string name) : name_(std::move(name)) {}
    std::string name() const override { return name_; }
    bool check() const override { return false; }

  private:
    std::string name_;
};

} // namespace

/// @brief Test fixture for HealthEndpoint tests.
class ObservabilityHealthEndpointTest : public testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test all checks UP returns 200.
 * @details Verifies that when all health checks pass, the response is 200 with status UP.
 */
TEST_F(ObservabilityHealthEndpointTest, AllChecksUp)
{
    HealthEndpoint endpoint;
    endpoint.registerCheck(std::make_shared<AlwaysHealthyCheck>("database"));
    endpoint.registerCheck(std::make_shared<AlwaysHealthyCheck>("cache"));

    MockHttpRequest req;
    MockHttpResponse res;
    endpoint.handle(req, res);

    EXPECT_EQ(res.statusCode_, 200);
    EXPECT_NE(res.body_.find("\"status\":\"UP\""), std::string::npos);
    EXPECT_NE(res.body_.find("\"name\":\"database\""), std::string::npos);
    EXPECT_NE(res.body_.find("\"name\":\"cache\""), std::string::npos);
}

/**
 * @brief Test any check DOWN returns 503.
 * @details Verifies that when any health check fails, the response is 503 with status DOWN.
 */
TEST_F(ObservabilityHealthEndpointTest, AnyCheckDown)
{
    HealthEndpoint endpoint;
    endpoint.registerCheck(std::make_shared<AlwaysHealthyCheck>("database"));
    endpoint.registerCheck(std::make_shared<AlwaysUnhealthyCheck>("cache"));

    MockHttpRequest req;
    MockHttpResponse res;
    endpoint.handle(req, res);

    EXPECT_EQ(res.statusCode_, 503);
    EXPECT_NE(res.body_.find("\"status\":\"DOWN\""), std::string::npos);
    EXPECT_NE(res.body_.find("\"name\":\"cache\",\"status\":\"DOWN\""), std::string::npos);
}

/**
 * @brief Test empty health endpoint returns UP.
 * @details Verifies that with no checks registered, the endpoint reports UP.
 */
TEST_F(ObservabilityHealthEndpointTest, EmptyEndpoint)
{
    HealthEndpoint endpoint;

    MockHttpRequest req;
    MockHttpResponse res;
    endpoint.handle(req, res);

    EXPECT_EQ(res.statusCode_, 200);
    EXPECT_NE(res.body_.find("\"status\":\"UP\""), std::string::npos);
    EXPECT_NE(res.body_.find("\"checks\":[]"), std::string::npos);
}

/**
 * @brief Test health endpoint sets Content-Type header.
 */
TEST_F(ObservabilityHealthEndpointTest, SetsContentType)
{
    HealthEndpoint endpoint;
    endpoint.registerCheck(std::make_shared<AlwaysHealthyCheck>("db"));

    MockHttpRequest req;
    MockHttpResponse res;
    endpoint.handle(req, res);

    auto it = res.headers_.find("Content-Type");
    ASSERT_NE(it, res.headers_.end());
    EXPECT_EQ(it->second, "application/json");
}
