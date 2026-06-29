/**
 * @file IAopAspect.hpp
 * @brief Interface for AOP aspect implementations
 * @details Defines the contract for aspect-oriented programming aspects:
 *          before/after/around advice methods that can be woven around
 *          target function invocations.  Implementations provide cross-cutting
 *          concerns like logging, profiling, and transaction management.
 */

#pragma once
#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

namespace cppforge::interface::aop
{
    /// @brief Interface class for Aspect-Oriented Programming (AOP) functionalities.
    /// This class provides a template-based interface for implementing AOP concepts
    /// such as before/after method execution advice, exception handling, and result processing.
    /// @tparam Derived The derived class that implements the specific AOP behavior.
    /// The derived class can override the virtual methods to customize the AOP behavior.
    template <typename Derived>
    class IAopAspect
    {
    public:
        /// @brief Execute the function with the given arguments
        /// @param func Function to be executed
        /// @param args Arguments to be passed to the function
        /// @return The result of the function
        template <typename Func, typename... Args>
            requires std::invocable<Func, Args...>
        [[nodiscard]] decltype(auto) exec(Func&& func, Args&&... args);

        virtual ~IAopAspect() = default;

    protected:
        /// @brief Function to be executed before the function call
        /// @details This method is called before the target function is invoked.
        /// Derived classes can override this to implement pre-execution logic.
        virtual void onEntry() = 0;

        /// @brief Function to be executed after the function call
        /// @details This method is called after the target function is successfully invoked.
        /// Derived classes can override this to implement post-execution logic.
        virtual void onExit() = 0;

        /// @brief Function to be executed when an exception is thrown
        /// @details This method is called when an exception is caught during function execution.
        /// Derived classes can override this to implement exception handling logic.
        /// @param e Exception pointer containing the caught exception
        virtual void onException(std::exception_ptr e) = 0;

        /// @brief Function to handle the result
        /// @tparam T The type of the result
        /// @param result The result to be handled
        /// @return The processed result
        /// @details This method is called to process the result of the function execution.
        /// Derived classes can override this to implement result processing logic.
        /// @note Default implementation returns by value to avoid dangling references
        template <typename T>
        auto handleResult(T&& result) -> std::decay_t<T>
        {
            return std::forward<T>(result);
        }
    };

    template <typename Derived>
    template <typename Func, typename... Args>
        requires std::invocable<Func, Args...>
    decltype(auto) IAopAspect<Derived>::exec(Func&& func, Args&&... args)
    {
        // Execute the pre-execution logic
        static_cast<Derived*>(this)->onEntry();
        try
        {
            if constexpr (std::is_void_v<std::invoke_result_t<Func, Args...>>)
            {
                // For void return types, invoke function and execute post-execution logic
                std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
                static_cast<Derived*>(this)->onExit();
            }
            else
            {
                // For non-void return types, capture result, execute post-execution logic,
                // and process the result through handleResult
                auto result = std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
                static_cast<Derived*>(this)->onExit();
                return static_cast<Derived*>(this)->handleResult(std::move(result));
            }
        }
        catch (...)
        {
            static_cast<Derived*>(this)->onException(std::current_exception());
            throw;
        }
    }
}
