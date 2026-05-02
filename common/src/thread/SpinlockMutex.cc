#include "src/thread/SpinlockMutex.hpp"

#include <fmt/format.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <sstream>

namespace common::thread {
    namespace {
        // Helper function to convert thread::id to string
        auto getThreadIdString() -> std::string {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            return oss.str();
        }
    }

    auto SpinlockMutex::lock() noexcept -> void {
        // Use exponential backoff to reduce contention
        int spin_count = 0;
        while (flag_.test_and_set(std::memory_order_acquire)) {
            if (++spin_count > 100) {
                // After several attempts, yield to other threads
                std::this_thread::yield();
                spin_count = 0;
            } else {
                // On modern CPUs, pause instruction can be more efficient than yielding
#if defined(_MSC_VER) || defined(__GNUC__)
                _mm_pause(); // x86 specific pause instruction, requires <immintrin.h> indirectly
#endif
            }
        }
    }

    auto SpinlockMutex::try_lock() noexcept -> bool {
        // Attempt to acquire the lock without blocking
        return !flag_.test_and_set(std::memory_order_acquire);
    }

    template<typename Rep, typename Period>
    auto SpinlockMutex::try_lock_for(const std::chrono::duration<Rep, Period> &timeout_duration) noexcept -> bool {
        auto start_time = std::chrono::high_resolution_clock::now();
        auto end_time = start_time + timeout_duration;

        // First attempt without waiting
        if (try_lock()) {
            return true;
        }

        // Loop until timeout attempting to acquire the lock
        while (std::chrono::high_resolution_clock::now() < end_time) {
            if (try_lock()) {
                return true;
            }

            // Small delay to reduce CPU usage during polling
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }

        return false;
    }

    auto SpinlockMutex::unlock() noexcept -> void {
        flag_.clear(std::memory_order_release);
    }

    // Explicitly instantiate the template method for common duration types
    template auto SpinlockMutex::try_lock_for<>(const std::chrono::duration<int64_t, std::milli> &) noexcept -> bool;

    template auto SpinlockMutex::try_lock_for<>(const std::chrono::duration<int64_t, std::nano> &) noexcept -> bool;

    template auto SpinlockMutex::try_lock_for<>(const std::chrono::duration<double, std::milli> &) noexcept -> bool;
}
