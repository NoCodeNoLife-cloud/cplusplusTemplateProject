/**
 * @file PeriodicActuator.cc
 * @brief PeriodicActuator implementation â€?timer loop, start/stop lifecycle
 * @details Implements the periodic execution loop: uses std::this_thread::sleep_for
 *          or std::chrono::steady_clock for fixed-rate timing.  start/stop
 *          are synchronised via a mutex and atomic flag.
 */

#include "thread/PeriodicActuator.hpp"

#include <chrono>
#include <memory>
#include <stdexcept>
#include <boost/asio.hpp>
#include <fmt/format.h>
#include <glog/logging.h>

#include "interface/task/ITimerTask.hpp"

namespace cppforge::thread
{
    PeriodicActuator::PeriodicActuator(std::shared_ptr<interface::task::ITimerTask> task, const std::chrono::milliseconds interval) : task_(std::move(task)), interval_(interval)
    {
        if (!task_)
        {
            DLOG(WARNING) << "PeriodicActuator constructor: task is null";
            throw std::invalid_argument("PeriodicActuator::PeriodicActuator: task cannot be null");
        }

        if (interval_.count() <= 0)
        {
            DLOG(WARNING) << fmt::format("PeriodicActuator constructor: interval {} must be positive", interval_.count());
            throw std::invalid_argument("PeriodicActuator::PeriodicActuator: interval must be positive");
        }
    }

    PeriodicActuator::~PeriodicActuator()
    {
        if (isRunning())
        {
            stop();
        }
    }

    void PeriodicActuator::start()
    {
        if (isRunning())
        {
            DLOG(WARNING) << "PeriodicActuator start: Actuator is already running";
            throw std::runtime_error("PeriodicActuator::start: Actuator is already running");
        }
        ioContext_.restart();
        isRunning_ = true;
        scheduleNext();

        workerThread_ = std::thread([this]()
        {
            try
            {
                ioContext_.run();
            }
            catch (...)
            {
                // Log the exception or handle it appropriately
                isRunning_ = false;
                throw; // Re-throw to be handled by thread management
            }
        });
    }

    void PeriodicActuator::stop()
    {
        if (!isRunning())
        {
            return; // Already stopped
        }
        isRunning_ = false;

        // Cancel the timer to unblock the io_context
        timer_.cancel();

        // Stop the io_context to make run() return
        ioContext_.stop();

        // Wait for the worker thread to finish
        if (workerThread_.joinable())
        {
            workerThread_.join();
        }
    }

    bool PeriodicActuator::isRunning() const
    {
        return isRunning_;
    }

    void PeriodicActuator::scheduleNext()
    {
        if (!isRunning())
        {
            return; // Don't schedule if not running
        }

        timer_.expires_after(interval_);
        timer_.async_wait([this](const boost::system::error_code& ec)
        {
            if (!ec && isRunning()) // Only proceed if no error and still running
            {
                try
                {
                    task_->execute();
                }
                catch ([[maybe_unused]] const std::exception& e)
                {
                    // Handle exception from task execution - could log or rethrow based on requirements
                    // For now, we continue with the next scheduled execution
                }
                catch (...)
                {
                    // Catch any other exceptions to prevent the scheduler from stopping
                }

                if (isRunning())
                {
                    scheduleNext(); // Schedule the next execution if still running
                }
            }
        });
    }
}
