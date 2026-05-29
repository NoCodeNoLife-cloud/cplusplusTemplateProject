/**
 * @file AutoJoinThread.hpp
 * @brief AutoJoinThread class declaration
 * @details This header defines the AutoJoinThread class that provides functionality for Threading utilities and thread pool implementation.
 */

#pragma once

#include <thread>
#include <utility>

namespace common::thread
{
    /// @brief A wrapper class for std::thread that automatically joins the thread in its destructor.
    class AutoJoinThread
    {
    public:
        /// @brief Disable copy construction
        AutoJoinThread(const AutoJoinThread&) = delete;

        /// @brief Disable copy assignment
        AutoJoinThread& operator=(const AutoJoinThread&) = delete;

        /// @brief Move constructor
        /// @param other The source object to move from
        AutoJoinThread(AutoJoinThread&& other) noexcept;

        /// @brief Move assignment operator
        /// @param other The source object to move from
        /// @return Reference to this object
        auto operator=(AutoJoinThread&& other) noexcept -> AutoJoinThread&;

        /// @brief Template constructor that accepts any callable object and its arguments
        /// @tparam Function The type of the callable object
        /// @tparam Args The types of the arguments to pass to the callable object
        /// @param f The callable object
        /// @param args The arguments to pass to the callable object
        template <typename Function, typename... Args>
        explicit AutoJoinThread(Function&& f, Args&&... args) : thread_(std::forward<Function>(f), std::forward<Args>(args)...)
        {
        }

        /// @brief Destructor that automatically joins the thread if it's joinable
        ~AutoJoinThread();

        /// @brief Detaches the thread from the object
        auto detach() -> void;

        /// @brief Joins the thread with the calling thread
        auto join() -> void;

        /// @brief Checks if the thread is joinable
        /// @return true if the thread is joinable, false otherwise
        [[nodiscard]] auto joinable() const noexcept -> bool;

        /// @brief Swaps this thread with another AutoJoinThread object
        /// @param other The other AutoJoinThread object to swap with
        auto swap(AutoJoinThread& other) noexcept -> void;

        /// @brief Gets the native handle of the underlying thread
        /// @return The native handle of the thread
        [[nodiscard]] auto native_handle() -> std::thread::native_handle_type;

    private:
        std::thread thread_; ///< The underlying thread object
    };
}
