#include "src/thread/PeriodicActuator.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <stdexcept>

#include "src/thread/interface/ITimerTask.hpp"

namespace common::thread {
    PeriodicActuator::PeriodicActuator(std::shared_ptr<interfaces::ITimerTask> task, const std::chrono::milliseconds interval) : task_(std::move(task)), timer_(ioContext_), interval_(interval) {
        if (!task_) {
            DLOG(ERROR) << "PeriodicActuator initialization failed - task is null";
            throw std::invalid_argument("PeriodicActuator::PeriodicActuator: task cannot be null");
        }

        if (interval_.count() <= 0) {
            DLOG(ERROR) << fmt::format("PeriodicActuator initialization failed - invalid interval: {}ms", interval_.count());
            throw std::invalid_argument("PeriodicActuator::PeriodicActuator: interval must be positive");
        }

        DLOG(INFO) << fmt::format("PeriodicActuator initialized - interval={}ms", interval_.count());
    }

    PeriodicActuator::~PeriodicActuator() {
        if (isRunning()) {
            stop();
        }
    }

    auto PeriodicActuator::start() -> void {
        if (isRunning()) {
            DLOG(WARNING) << "PeriodicActuator start failed - actuator is already running";
            throw std::runtime_error("PeriodicActuator::start: Actuator is already running");
        }

        DLOG(INFO) << fmt::format("PeriodicActuator starting - interval={}ms", interval_.count());
        isRunning_ = true;
        scheduleNext();

        workerThread_ = std::thread([this]() {
            try {
                ioContext_.run();
                DLOG(INFO) << "PeriodicActuator worker thread completed";
            } catch (...) {
                // Log the exception or handle it appropriately
                DLOG(ERROR) << "PeriodicActuator worker thread exception occurred";
                isRunning_ = false;
                throw; // Re-throw to be handled by thread management
            }
        });

        DLOG(INFO) << "PeriodicActuator started successfully";
    }

    auto PeriodicActuator::stop() -> void {
        if (!isRunning()) {
            DLOG(INFO) << "PeriodicActuator stop called - already stopped";
            return; // Already stopped
        }

        DLOG(INFO) << "PeriodicActuator stopping...";
        isRunning_ = false;

        // Stop the io_context gracefully
        ioContext_.stop();

        // Wait for the worker thread to finish
        if (workerThread_.joinable()) {
            workerThread_.join();
        }

        DLOG(INFO) << "PeriodicActuator stopped successfully";
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
                    DLOG(ERROR) << fmt::format("PeriodicActuator task execution exception: {}", e.what());
                } catch (...) {
                    // Catch any other exceptions to prevent the scheduler from stopping
                    DLOG(ERROR) << "PeriodicActuator task execution unknown exception";
                }

                if (isRunning()) {
                    scheduleNext(); // Schedule the next execution if still running
                }
            }
        });
    }
}
