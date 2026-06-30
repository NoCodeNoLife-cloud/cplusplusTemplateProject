/// @file EventBus.hpp
/// @brief Thread-safe event bus implementation for cppforge observability.
/// @details Provides publish/subscribe messaging with thread safety.

#pragma once

#include <cppforge/observability/events/IEventBus.hpp>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace cppforge::observability::events
{

/// @brief Thread-safe event bus implementation.
/// @details Supports multiple subscribers per event with synchronous dispatch.
class EventBus : public IEventBus
{
  public:
    void publish(const std::string &event, const std::string &payload) override;
    void subscribe(const std::string &event, std::function<void(const std::string &)> handler) override;

  private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<std::function<void(const std::string &)>>> subscribers_;
};

} // namespace cppforge::observability::events
