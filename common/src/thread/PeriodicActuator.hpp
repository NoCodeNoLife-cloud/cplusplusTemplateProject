#pragma once
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <boost/asio.hpp>

#include "src/interface/ITimerTask.hpp"

namespace common::thread {
    /// @brief A utility class that executes a given task periodically using Boost.Asio.
    /// @details This class uses a steady_timer from Boost.Asio to schedule and execute tasks at regular intervals.
    ///          The task to be executed must implement the ITimerTask interface.
    class PeriodicActuator {
    public:
        /// @brief Construct a PeriodicActuator with a task and interval
        /// @param task The task to execute periodically
        /// @param interval The interval between executions
        // ReSharper disable once CppDFATimeOver
        explicit PeriodicActuator(std::shared_ptr<interfaces::ITimerTask> task, std::chrono::milliseconds interval);

        /// @brief Destructor that stops the actuator if running
        ~PeriodicActuator();

        /// @brief Start the periodic actuator to begin executing the task at specified intervals.
        /// @details This function creates a background thread to run the io_context and starts the periodic execution.
        ///          The task will then be rescheduled automatically based on the configured interval until stopped.
        auto start() -> void;

        /// @brief Stop the periodic actuator gracefully.
        /// @details This function stops the io_context, allowing the background thread to finish.
        ///          Any pending tasks will be cancelled.
        auto stop() -> void;

        /// @brief Check if the actuator is currently running.
        /// @return true if the actuator is running, false otherwise.
        [[nodiscard]] auto isRunning() const -> bool;

    private:
        boost::asio::io_context ioContext_{};
        std::shared_ptr<interfaces::ITimerTask> task_{};
        boost::asio::steady_timer timer_;
        std::chrono::milliseconds interval_{};
        std::thread workerThread_{};
        std::atomic<bool> isRunning_{false};

        /// @brief Schedule the next execution of the task
        auto scheduleNext() -> void;
    };
}
