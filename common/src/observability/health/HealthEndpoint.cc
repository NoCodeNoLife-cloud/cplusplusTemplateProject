/// @file HealthEndpoint.cc
/// @brief Implementation of the health endpoint.

#include <cppforge/observability/health/HealthEndpoint.hpp>
#include <sstream>

namespace cppforge::observability::health
{

void HealthEndpoint::registerCheck(std::shared_ptr<IHealthCheck> check)
{
    checks_.push_back(std::move(check));
}

void HealthEndpoint::handle(const http::IHttpRequest & /*req*/, http::IHttpResponse &res) const
{
    bool allUp = true;
    std::ostringstream checksJson;
    checksJson << "[";

    for (std::size_t i = 0; i < checks_.size(); ++i)
    {
        if (i > 0)
        {
            checksJson << ",";
        }
        bool isUp = checks_[i]->check();
        if (!isUp)
        {
            allUp = false;
        }
        checksJson << "{\"name\":\"" << checks_[i]->name() << "\",\"status\":\"" << (isUp ? "UP" : "DOWN") << "\"}";
    }

    checksJson << "]";

    std::ostringstream body;
    body << "{\"status\":\"" << (allUp ? "UP" : "DOWN") << "\",\"checks\":" << checksJson.str() << "}";

    res.status(static_cast<int>(allUp ? http::HttpStatus::OK : http::HttpStatus::ServiceUnavailable));
    res.header("Content-Type", "application/json");
    res.body(body.str());
}

} // namespace cppforge::observability::health