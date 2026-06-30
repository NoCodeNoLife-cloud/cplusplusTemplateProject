/// @file EventBus.cc
/// @brief Implementation of the thread-safe event bus.

#include <cppforge/observability/events/EventBus.hpp>

namespace cppforge::observability::events
{

void EventBus::publish(const std::string &event, const std::string &payload)
{
    std::vector<std::function<void(const std::string &)>> handlers;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscribers_.find(event);
        if (it != subscribers_.end())
        {
            handlers = it->second;
        }
    }
    for (const auto &handler : handlers)
    {
        handler(payload);
    }
}

void EventBus::subscribe(const std::string &event, std::function<void(const std::string &)> handler)
{
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_[event].push_back(std::move(handler));
}

} // namespace cppforge::observability::events
