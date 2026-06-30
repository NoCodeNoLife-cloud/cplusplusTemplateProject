/// @file Router.cc
/// @brief Implementation of the HTTP router.

#include <cppforge/middleware/Router.hpp>

namespace cppforge::middleware
{

Router &Router::get(const std::string &path, Handler handler)
{
    routes_.push_back({"GET", path, std::move(handler)});
    return *this;
}

Router &Router::post(const std::string &path, Handler handler)
{
    routes_.push_back({"POST", path, std::move(handler)});
    return *this;
}

Router &Router::put(const std::string &path, Handler handler)
{
    routes_.push_back({"PUT", path, std::move(handler)});
    return *this;
}

Router &Router::del(const std::string &path, Handler handler)
{
    routes_.push_back({"DELETE", path, std::move(handler)});
    return *this;
}

void Router::handle(const http::IHttpRequest &req, http::IHttpResponse &res) const
{
    const auto &method = req.method();
    const auto &path = req.path();

    for (const auto &route : routes_)
    {
        if (route.method == method && route.path == path)
        {
            route.handler(req, res);
            return;
        }
    }

    res.status(static_cast<int>(http::HttpStatus::NotFound));
    res.body("Not Found");
}

} // namespace cppforge::middleware