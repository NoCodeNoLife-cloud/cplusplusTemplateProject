#pragma once
#include <memory>
#include <typeindex>

namespace cppforge::di
{
    /// @brief Service resolution interface
    class IServiceProvider
    {
    public:
        virtual ~IServiceProvider() = default;

        /// @brief Resolve a service by type
        template<typename T>
        [[nodiscard]] std::shared_ptr<T> getService()
        {
            return std::static_pointer_cast<T>(resolve(typeid(T)));
        }

        /// @brief Check if a service is registered
        [[nodiscard]] virtual bool isRegistered(std::type_index type) const = 0;

    protected:
        /// @brief Internal resolution method
        [[nodiscard]] virtual std::shared_ptr<void> resolve(std::type_index type) = 0;
    };
}