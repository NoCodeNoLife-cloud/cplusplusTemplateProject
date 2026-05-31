/**
 * @file AutoJoinThread.cc
 * @brief AutoJoinThread class implementation
 * @details This file contains the implementation of the AutoJoinThread class methods for Threading utilities and thread pool implementation.
 */

#include "thread/AutoJoinThread.hpp"

#include <fmt/format.h>
#include <thread>
#include <utility>
#include <sstream>

namespace common::thread
{
    AutoJoinThread::AutoJoinThread(AutoJoinThread&& other)  : thread_(std::move(other.thread_))
    {
    }

    AutoJoinThread::~AutoJoinThread()
    {
        if (thread_.joinable())
        {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
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

    auto AutoJoinThread::operator=(AutoJoinThread&& other)  -> AutoJoinThread&
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
}