/// @file IEventBus.hpp
/// @brief Event bus interface for cppforge observability.
/// @details Defines the contract for a publish/subscribe event system.

#pragma once

#include <functional>
#include <string>

namespace cppforge::observability::events
{

/// @brief Abstract event bus interface.
/// @details Provides publish/subscribe messaging for named events.
class IEventBus
{
  public:
    virtual ~IEventBus() = default;

    /// @brief Publish an event with a payload.
    /// @param event The event name.
    /// @param payload The event payload.
    virtual void publish(const std::string &event, const std::string &payload) = 0;

    /// @brief Subscribe to an event.
    /// @param event The event name.
    /// @param handler The callback invoked when the event is published.
    virtual void subscribe(const std::string &event, std::function<void(const std::string &)> handler) = 0;
};

} // namespace cppforge::observability::events
