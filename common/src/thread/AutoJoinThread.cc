#include "src/thread/AutoJoinThread.hpp"

#include <fmt/format.h>
#include <thread>
#include <utility>
#include <sstream>

namespace common::thread {
    AutoJoinThread::AutoJoinThread(AutoJoinThread &&other) noexcept : thread_(std::move(other.thread_)) {
    }

    AutoJoinThread::~AutoJoinThread() {
        if (thread_.joinable()) {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
thread_.join();
        }
    }

    auto AutoJoinThread::detach() -> void {
        if (thread_.joinable()) {
thread_.detach();
        }
    }

    auto AutoJoinThread::join() -> void {
        if (thread_.joinable()) {
thread_.join();
        }
    }

    auto AutoJoinThread::joinable() const noexcept -> bool {
        return thread_.joinable();
    }

    auto AutoJoinThread::swap(AutoJoinThread &other) noexcept -> void {
        thread_.swap(other.thread_);
    }

    auto AutoJoinThread::native_handle() -> std::thread::native_handle_type {
        return thread_.native_handle();
    }

    auto AutoJoinThread::operator=(AutoJoinThread &&other) noexcept -> AutoJoinThread & {
        if (this != &other) {
            if (thread_.joinable()) {
thread_.join();
            }
            thread_ = std::move(other.thread_);
        }
        return *this;
    }
}
