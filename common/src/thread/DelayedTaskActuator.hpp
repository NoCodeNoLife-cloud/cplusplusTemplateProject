#pragma once
#include <fmt/format.h>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <cstdint>

namespace common::thread {
    /// @brief A class that executes tasks with a specified delay.
    /// @tparam ResultType The type of the result returned by the task.
    template<typename ResultType>
    class DelayedTaskActuator {
    public:
        /// @brief Schedules a task to be executed after a specified delay.
        /// @param delayMs The delay in milliseconds before the task is executed.
        /// @param task The task to be executed.
        /// @return The ID of the scheduled task.
        [[nodiscard]] auto scheduleTask(int32_t delayMs, std::function<ResultType()> task) -> int32_t;

        /// @brief Retrieves the result of a scheduled task.
        /// @param taskId The ID of the task whose result is to be retrieved.
        /// @return A future object that will hold the result of the task.
        [[nodiscard]] auto getTaskResult(int32_t taskId) -> std::future<ResultType>;

        /// @brief Checks if a task is currently pending execution.
        /// @param taskId The ID of the task to check.
        /// @return true if the task is pending, false otherwise.
        auto isTaskPending(const int32_t taskId) const -> bool;

        /// @brief Cancels a scheduled task if it hasn't started yet.
        /// @param taskId The ID of the task to cancel.
        /// @return true if the task was canceled, false if it was not found or already executed.
        [[nodiscard]] bool cancelTask(int32_t taskId);

    private:
        mutable std::mutex mutex_{};
        std::condition_variable cv_{};
        std::unordered_map<int32_t, std::future<ResultType> > results_{};
        std::unordered_map<int32_t, std::shared_ptr<bool> > pendingTasks_{}; // Track active tasks (true=pending, false=completed)
        int32_t nextTaskId_{0};
    };

    template<typename ResultType>
    auto DelayedTaskActuator<ResultType>::scheduleTask(int32_t delayMs, std::function<ResultType()> task) -> int32_t {
        if (delayMs < 0) {
            throw std::invalid_argument("DelayedTaskActuator::scheduleTask: delayMs must be non-negative");
        }

        if (!task) {
            throw std::invalid_argument("DelayedTaskActuator::scheduleTask: task function cannot be null");
        }

        std::lock_guard lock(mutex_);
        const int32_t taskId = nextTaskId_++;
        
        auto packagedTask = std::make_shared<std::packaged_task<ResultType()> >(std::move(task));
        std::future<ResultType> result = packagedTask->get_future();

        // Store pending task before starting thread to track execution state
        pendingTasks_[taskId] = std::make_shared<bool>(true);

        // Create a copyable lambda that captures the packaged task by shared_ptr
        std::thread([this, delayMs, packagedTask, taskId]() mutable {
            try {
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

                // Execute the task inside a try-catch to handle exceptions
                try {
                    (*packagedTask)();
                }
                // ReSharper disable once CppDFAUnreachableCode
                catch (...) {
                    // If task throws, set exception in packaged task
                    try {
                        throw;
                    } catch (...) {
                        packagedTask->set_exception(std::current_exception());
                    }
                }

                // Mark that execution is complete
                std::lock_guard lock1(mutex_);
                const auto it = pendingTasks_.find(taskId);
                if (it != pendingTasks_.end()) {
                    *(it->second) = false; // Mark as completed
                    pendingTasks_.erase(it);
                }
                cv_.notify_all(); // Notify waiting threads that task state changed
            } catch (...) {
                // Handle any unexpected exceptions during execution
                std::lock_guard lock1(mutex_);
                const auto it = pendingTasks_.find(taskId);
                if (it != pendingTasks_.end()) {
                    pendingTasks_.erase(it);
                }
                cv_.notify_all();
            }
        }).detach();

        results_[taskId] = std::move(result);
        return taskId;
    }

    template<typename ResultType>
    auto DelayedTaskActuator<ResultType>::getTaskResult(int32_t taskId) -> std::future<ResultType> {
        std::unique_lock lock(mutex_);

        // Wait until the task result is available or task has completed execution
        cv_.wait(lock, [this, taskId] {
            // Return true if result exists OR task was already executed (not pending anymore)
            return results_.contains(taskId) || !isTaskPending(taskId);
        });

        auto it = results_.find(taskId);
        if (it != results_.end()) {
            std::future<ResultType> result = std::move(it->second);
            results_.erase(it);
            return result;
        } else {
            // Task completed but result was already retrieved
            throw std::runtime_error("DelayedTaskActuator::getTaskResult: Task result not available, possibly already retrieved");
        }
    }

    template<typename ResultType>
    auto DelayedTaskActuator<ResultType>::isTaskPending(const int32_t taskId) const -> bool {
        std::lock_guard lock(mutex_);
        const auto it = pendingTasks_.find(taskId);
        if (it != pendingTasks_.end()) {
            return *(it->second); // Return the boolean value indicating pending status
        }
        // If not in pendingTasks_, it's either completed or doesn't exist
        return false;
    }

    template<typename ResultType>
    bool DelayedTaskActuator<ResultType>::cancelTask(int32_t taskId) {
        std::lock_guard lock(mutex_);
        auto it = results_.find(taskId);
        if (it != results_.end()) {
            // We can't actually cancel the running thread, but we can remove the result
            results_.erase(it);

            // Also remove from pending tasks if it's still there
            const auto pendingIt = pendingTasks_.find(taskId);
            if (pendingIt != pendingTasks_.end()) {
                pendingTasks_.erase(pendingIt);
            }

            return true;
        }
        return false;
    }
}
