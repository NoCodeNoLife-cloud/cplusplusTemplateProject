/// @file ConfigWatcher.hpp
/// @brief Configuration file hot-reload watcher for cppforge observability.
/// @details Monitors a file for changes and invokes a callback when modifications are detected.

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace cppforge::observability::config
{

/// @brief Watches a configuration file for changes and triggers a callback.
/// @details Polls the file at a configurable interval and invokes the registered
///          callback with the new file content when changes are detected.
class ConfigWatcher
{
  public:
    /// @brief Construct a config watcher for the given file.
    /// @param filepath Path to the configuration file to watch.
    explicit ConfigWatcher(std::string filepath);

    /// @brief Destructor. Stops the watcher if running.
    ~ConfigWatcher();

    // Non-copyable, non-movable
    ConfigWatcher(const ConfigWatcher &) = delete;
    ConfigWatcher &operator=(const ConfigWatcher &) = delete;
    ConfigWatcher(ConfigWatcher &&) = delete;
    ConfigWatcher &operator=(ConfigWatcher &&) = delete;

    /// @brief Start watching the file for changes.
    /// @param interval Polling interval (default 1000ms).
    void start(std::chrono::milliseconds interval = std::chrono::milliseconds(1000));

    /// @brief Stop watching the file.
    void stop();

    /// @brief Register a callback for file changes.
    /// @param callback Invoked with the new file content when a change is detected.
    /// @note Thread-safe. Can be called before or after start().
    void onChange(std::function<void(const std::string &)> callback);

  private:
    std::string filepath_;
    std::chrono::milliseconds interval_;
    std::function<void(const std::string &)> callback_;
    mutable std::mutex callbackMutex_;  ///< Protects callback_ access
    std::atomic<bool> running_{false};
    std::thread watchThread_;
    std::string lastContent_;

    /// @brief The main watch loop that polls for file changes.
    void watchLoop();

    /// @brief Read the current file content.
    /// @return The file content as a string.
    std::string readFile() const;
};

} // namespace cppforge::observability::config