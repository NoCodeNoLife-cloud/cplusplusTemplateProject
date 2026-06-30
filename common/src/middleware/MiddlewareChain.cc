/// @file MiddlewareChain.cc
/// @brief Implementation of the middleware chain.

#include <cppforge/middleware/MiddlewareChain.hpp>

namespace cppforge::middleware
{

void MiddlewareChain::add(std::shared_ptr<IMiddleware> middleware)
{
    middlewares_.push_back(std::move(middleware));
}

void MiddlewareChain::execute(const http::IHttpRequest &req, http::IHttpResponse &res) const
{
    std::size_t index = 0;
    std::function<void()> next;
    next = [&]()
    {
        if (index < middlewares_.size())
        {
            auto current = middlewares_[index++];
            current->handle(req, res, next);
        }
    };
    next();
}

std::size_t MiddlewareChain::size() const
{
    return middlewares_.size();
}

} // namespace cppforge::middleware
