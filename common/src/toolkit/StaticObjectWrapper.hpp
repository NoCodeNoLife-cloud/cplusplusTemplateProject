/**
 * @file StaticObjectWrapper.hpp
 * @brief StaticObjectWrapper class declaration
 * @details This header defines the StaticObjectWrapper class that provides functionality for General utility toolkits for strings, arrays, and other operations.
 */

#pragma once
#include <memory>
#include <mutex>
#include <stdexcept>
#include <type_traits>

namespace common::toolkit
{
    /// @brief A wrapper class for static object initialization and management.
    /// Ensures thread-safe initialization of static objects with lazy initialization support.
    /// @tparam T The type of object to wrap
    template <typename T>
    class StaticObjectWrapper
    {
    public:
        StaticObjectWrapper() = delete;

        StaticObjectWrapper(const StaticObjectWrapper&) = delete;

        StaticObjectWrapper& operator=(const StaticObjectWrapper&) = delete;

        /// @brief Initialize the static object with provided arguments
        /// @tparam Args Types of arguments to forward to T's constructor
        /// @param args Arguments to forward to T's constructor
        template <typename... Args>
        static void init(Args&&... args) ;

        /// @brief Get a mutable reference to the static object
        /// @return Reference to the static object
        /// @throws std::runtime_error if object was not initialized and is not default constructible
        [[nodiscard]] static T& get();

        /// @brief Destroy the static object if it exists
        static void destroy() ;

        /// @brief Check if the static object has been initialized
        /// @return true if initialized, false otherwise
        [[nodiscard]] static bool isInitialized();

    private:
        static inline std::unique_ptr<T> instance_ = nullptr;
        static inline std::once_flag init_flag_;

        /// @brief Construct the object with provided arguments
        /// @tparam Args Types of arguments to forward to T's constructor
        /// @param args Arguments to forward to T's constructor
        template <typename... Args>
        static void construct(Args&&... args);
    };

    template <typename T>
    template <typename... Args>
    void StaticObjectWrapper<T>::init(Args&&... args)
    {
        std::call_once(init_flag_, construct<Args...>, std::forward<Args>(args)...);
    }

    template <typename T>
    T& StaticObjectWrapper<T>::get()
    {
        if constexpr (std::is_default_constructible_v<T>)
        {
            std::call_once(init_flag_, []
            {
                instance_ = std::make_unique<T>();
            });
        }

        if (!instance_)
        {
            throw std::runtime_error("StaticObjectWrapper::get: Object not initialized. Call init() with required parameters before first use.");
        }
        return *instance_;
    }

    template <typename T>
    void StaticObjectWrapper<T>::destroy()
    {
        instance_.reset(); // Automatically deletes and sets to nullptr
    }

    template <typename T>
    bool StaticObjectWrapper<T>::isInitialized()
    {
        return instance_ != nullptr;
    }

    template <typename T>
    template <typename... Args>
    void StaticObjectWrapper<T>::construct(Args&&... args)
    {
        instance_ = std::make_unique<T>(std::forward<Args>(args)...);
    }
}