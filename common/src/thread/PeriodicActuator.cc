#include "src/thread/PeriodicActuator.hpp"

#include <fmt/format.h>
#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <stdexcept>

#include "src/interface/ITimerTask.hpp"

namespace common::thread {
    PeriodicActuator::PeriodicActuator(std::shared_ptr<interfaces::ITimerTask> task, const std::chrono::milliseconds interval) : task_(std::move(task)), timer_(ioContext_), interval_(interval) {
        if (!task_) {
throw std::invalid_argument("PeriodicActuator::PeriodicActuator: task cannot be null");
        }

        if (interval_.count() <= 0) {
throw std::invalid_argument("PeriodicActuator::PeriodicActuator: interval must be positive");
        }
}

    PeriodicActuator::~PeriodicActuator() {
        if (isRunning()) {
            stop();
        }
    }

    auto PeriodicActuator::start() -> void {
        if (isRunning()) {
throw std::runtime_error("PeriodicActuator::start: Actuator is already running");
        }
isRunning_ = true;
        scheduleNext();

        workerThread_ = std::thread([this]() {
            try {
                ioContext_.run();
} catch (...) {
                // Log the exception or handle it appropriately
isRunning_ = false;
                throw; // Re-throw to be handled by thread management
            }
        });
}

    auto PeriodicActuator::stop() -> void {
        if (!isRunning()) {
return; // Already stopped
        }
isRunning_ = false;

        // Stop the io_context gracefully
        ioContext_.stop();

        // Wait for the worker thread to finish
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
}

    auto PeriodicActuator::isRunning() const -> bool {
        return isRunning_;
    }

    auto PeriodicActuator::scheduleNext() -> void {
        if (!isRunning()) {
            return; // Don't schedule if not running
        }

        timer_.expires_after(interval_);
        timer_.async_wait([this](const boost::system::error_code &ec) {
            if (!ec && isRunning()) // Only proceed if no error and still running
            {
                try {
                    task_->execute();
                } catch ([[maybe_unused]] const std::exception &e) {
                    // Handle exception from task execution - could log or rethrow based on requirements
                    // For now, we continue with the next scheduled execution
} catch (...) {
                    // Catch any other exceptions to prevent the scheduler from stopping
}

                if (isRunning()) {
                    scheduleNext(); // Schedule the next execution if still running
                }
            }
        });
    }
}
