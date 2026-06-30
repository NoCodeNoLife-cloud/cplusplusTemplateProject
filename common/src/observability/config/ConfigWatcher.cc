/// @file ConfigWatcher.cc
/// @brief Implementation of the configuration file watcher.

#include <cppforge/observability/config/ConfigWatcher.hpp>
#include <fstream>
#include <sstream>

namespace cppforge::observability::config
{

ConfigWatcher::ConfigWatcher(std::string filepath) : filepath_(std::move(filepath)), interval_(1000)
{
}

ConfigWatcher::~ConfigWatcher()
{
    stop();
}

void ConfigWatcher::start(std::chrono::milliseconds interval)
{
    if (running_.exchange(true))
    {
        return; // Already running
    }
    interval_ = interval;
    lastContent_ = readFile();
    watchThread_ = std::thread(&ConfigWatcher::watchLoop, this);
}

void ConfigWatcher::stop()
{
    if (running_.exchange(false))
    {
        if (watchThread_.joinable())
        {
            watchThread_.join();
        }
    }
}

void ConfigWatcher::onChange(std::function<void(const std::string &)> callback)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callback_ = std::move(callback);
}

void ConfigWatcher::watchLoop()
{
    while (running_.load())
    {
        std::this_thread::sleep_for(interval_);
        if (!running_.load())
        {
            break;
        }
        std::string currentContent = readFile();
        if (currentContent != lastContent_)
        {
            lastContent_ = currentContent;
            std::function<void(const std::string &)> callbackCopy;
            {
                std::lock_guard<std::mutex> lock(callbackMutex_);
                callbackCopy = callback_;
            }
            if (callbackCopy)
            {
                callbackCopy(currentContent);
            }
        }
    }
}

std::string ConfigWatcher::readFile() const
{
    std::ifstream file(filepath_);
    if (!file.is_open())
    {
        return {};
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

} // namespace cppforge::observability::config