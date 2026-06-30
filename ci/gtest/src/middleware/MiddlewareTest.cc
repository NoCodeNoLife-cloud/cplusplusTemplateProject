/**
 * @file MiddlewareTest.cc
 * @brief Unit tests for the middleware chain and router.
 * @details Tests cover middleware execution order, short-circuit behavior,
 *          and route matching by method and path.
 */

#include <string>
#include <vector>
#include <gtest/gtest.h>

#include <cppforge/http/IHttpRequest.hpp>
#include <cppforge/http/IHttpResponse.hpp>
#include <cppforge/middleware/IMiddleware.hpp>
#include <cppforge/middleware/MiddlewareChain.hpp>
#include <cppforge/middleware/Router.hpp>

using namespace cppforge::http;
using namespace cppforge::middleware;

namespace
{

/// @brief Mock HTTP request for testing.
class MockHttpRequest : public IHttpRequest
{
  public:
    MockHttpRequest(std::string method, std::string path) : method_(std::move(method)), path_(std::move(path)) {}

    std::string method() const override { return method_; }
    std::string path() const override { return path_; }
    std::string header(const std::string & /*key*/) const override { return ""; }
    std::string body() const override { return ""; }
    std::string queryParam(const std::string & /*key*/) const override { return ""; }

  private:
    std::string method_;
    std::string path_;
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

/// @brief Middleware that records its execution order.
class RecordingMiddleware : public IMiddleware
{
  public:
    explicit RecordingMiddleware(std::string name, std::vector<std::string> &log) : name_(std::move(name)), log_(log) {}

    void handle(const IHttpRequest & /*req*/, IHttpResponse & /*res*/, std::function<void()> next) override
    {
        log_.push_back(name_);
        next();
    }

  private:
    std::string name_;
    std::vector<std::string> &log_;
};

/// @brief Middleware that short-circuits the chain (does not call next).
class ShortCircuitMiddleware : public IMiddleware
{
  public:
    void handle(const IHttpRequest & /*req*/, IHttpResponse &res, std::function<void()> /*next*/) override
    {
        res.status(403);
        res.body("Blocked");
    }
};

} // namespace

/// @brief Test fixture for middleware tests.
class MiddlewareMiddlewareChainTest : public testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test middleware chain executes in order.
 * @details Verifies that middleware are called in the order they were added.
 */
TEST_F(MiddlewareMiddlewareChainTest, ExecutesInOrder)
{
    std::vector<std::string> log;
    MiddlewareChain chain;
    chain.add(std::make_shared<RecordingMiddleware>("A", log));
    chain.add(std::make_shared<RecordingMiddleware>("B", log));
    chain.add(std::make_shared<RecordingMiddleware>("C", log));

    MockHttpRequest req("GET", "/test");
    MockHttpResponse res;
    chain.execute(req, res);

    ASSERT_EQ(log.size(), 3u);
    EXPECT_EQ(log[0], "A");
    EXPECT_EQ(log[1], "B");
    EXPECT_EQ(log[2], "C");
}

/**
 * @brief Test middleware chain short-circuit behavior.
 * @details Verifies that a middleware that does not call next() stops the chain.
 */
TEST_F(MiddlewareMiddlewareChainTest, ShortCircuit)
{
    std::vector<std::string> log;
    MiddlewareChain chain;
    chain.add(std::make_shared<RecordingMiddleware>("A", log));
    chain.add(std::make_shared<ShortCircuitMiddleware>());
    chain.add(std::make_shared<RecordingMiddleware>("C", log));

    MockHttpRequest req("GET", "/test");
    MockHttpResponse res;
    chain.execute(req, res);

    ASSERT_EQ(log.size(), 1u);
    EXPECT_EQ(log[0], "A");
    EXPECT_EQ(res.statusCode_, 403);
    EXPECT_EQ(res.body_, "Blocked");
}

/**
 * @brief Test empty middleware chain.
 * @details Verifies that an empty chain executes without error.
 */
TEST_F(MiddlewareMiddlewareChainTest, EmptyChain)
{
    MiddlewareChain chain;
    MockHttpRequest req("GET", "/test");
    MockHttpResponse res;
    EXPECT_NO_THROW(chain.execute(req, res));
    EXPECT_EQ(chain.size(), 0u);
}

/// @brief Test fixture for router tests.
class MiddlewareRouterTest : public testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test router matches GET route.
 * @details Verifies that a registered GET handler is invoked for matching requests.
 */
TEST_F(MiddlewareRouterTest, MatchesGetRoute)
{
    Router router;
    bool called = false;
    router.get("/hello", [&](const IHttpRequest &, IHttpResponse &res)
               {
                   called = true;
                   res.status(200);
                   res.body("Hello");
               });

    MockHttpRequest req("GET", "/hello");
    MockHttpResponse res;
    router.handle(req, res);

    EXPECT_TRUE(called);
    EXPECT_EQ(res.statusCode_, 200);
    EXPECT_EQ(res.body_, "Hello");
}

/**
 * @brief Test router matches POST route.
 * @details Verifies that a registered POST handler is invoked for matching requests.
 */
TEST_F(MiddlewareRouterTest, MatchesPostRoute)
{
    Router router;
    bool called = false;
    router.post("/data", [&](const IHttpRequest &, IHttpResponse &res)
                {
                    called = true;
                    res.status(201);
                });

    MockHttpRequest req("POST", "/data");
    MockHttpResponse res;
    router.handle(req, res);

    EXPECT_TRUE(called);
    EXPECT_EQ(res.statusCode_, 201);
}

/**
 * @brief Test router returns 404 for unmatched route.
 * @details Verifies that unmatched requests get a 404 response.
 */
TEST_F(MiddlewareRouterTest, Returns404ForUnmatched)
{
    Router router;
    router.get("/hello", [](const IHttpRequest &, IHttpResponse &res)
               { res.status(200); });

    MockHttpRequest req("GET", "/notfound");
    MockHttpResponse res;
    router.handle(req, res);

    EXPECT_EQ(res.statusCode_, 404);
    EXPECT_EQ(res.body_, "Not Found");
}

/**
 * @brief Test router method mismatch returns 404.
 * @details Verifies that a GET request to a POST-only route returns 404.
 */
TEST_F(MiddlewareRouterTest, MethodMismatch)
{
    Router router;
    router.post("/data", [](const IHttpRequest &, IHttpResponse &res)
                { res.status(201); });

    MockHttpRequest req("GET", "/data");
    MockHttpResponse res;
    router.handle(req, res);

    EXPECT_EQ(res.statusCode_, 404);
}

/**
 * @brief Test router chaining.
 * @details Verifies that route registration methods return a reference for chaining.
 */
TEST_F(MiddlewareRouterTest, Chaining)
{
    Router router;
    auto &ref = router.get("/a", [](const IHttpRequest &, IHttpResponse &) {}).post("/b", [](const IHttpRequest &, IHttpResponse &) {});
    EXPECT_EQ(&ref, &router);
}
