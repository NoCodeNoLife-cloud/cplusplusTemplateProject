/**
 * @file AutoJoinThread.cc
 * @brief AutoJoinThread implementation â€?RAII join wrapper
 * @details Implements the RAII thread wrapper: destructor calls join() if the
 *          thread is joinable, ensuring clean thread lifecycle management.
 */

#include "thread/AutoJoinThread.hpp"
#include <thread>
#include <utility>

namespace cppforge::thread
{
    AutoJoinThread::AutoJoinThread(AutoJoinThread&& other) noexcept : thread_(std::move(other.thread_))
    {
    }

    auto AutoJoinThread::operator=(AutoJoinThread&& other) noexcept -> AutoJoinThread&
    {
        if (this != &other)
        {
            if (thread_.joinable())
            {
                thread_.join();
            }
            thread_ = std::move(other.thread_);
        }
        return *this;
    }

    AutoJoinThread::~AutoJoinThread()
    {
        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    void AutoJoinThread::detach()
    {
        if (thread_.joinable())
        {
            thread_.detach();
        }
    }

    void AutoJoinThread::join()
    {
        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    bool AutoJoinThread::joinable() const
    {
        return thread_.joinable();
    }

    void AutoJoinThread::swap(AutoJoinThread& other)
    {
        thread_.swap(other.thread_);
    }

    std::thread::native_handle_type AutoJoinThread::native_handle()
    {
        return thread_.native_handle();
    }
}
