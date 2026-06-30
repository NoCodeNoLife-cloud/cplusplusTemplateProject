/// @file IHttpServer.hpp
/// @brief HTTP server interface for cppforge framework.
/// @details Defines the contract for an HTTP server implementation.

#pragma once

#include <cstdint>
#include <functional>

namespace cppforge::http
{

class IHttpRequest;
class IHttpResponse;

/// @brief Abstract HTTP server interface.
/// @details Provides start/stop lifecycle and request handler registration.
class IHttpServer
{
  public:
    virtual ~IHttpServer() = default;

    /// @brief Start the server on the given port.
    /// @param port The port number to listen on.
    virtual void start(uint16_t port) = 0;

    /// @brief Stop the server.
    virtual void stop() = 0;

    /// @brief Set the request handler function.
    /// @param handler Callback invoked for each incoming request.
    virtual void setHandler(std::function<void(const IHttpRequest &, IHttpResponse &)> handler) = 0;
};

} // namespace cppforge::http
