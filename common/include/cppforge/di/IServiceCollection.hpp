#pragma once
#include <memory>
#include "ServiceDescriptor.hpp"

namespace cppforge::di
{
    class IServiceProvider;

    /// @brief Service registration interface
    class IServiceCollection
    {
    public:
        virtual ~IServiceCollection() = default;

        /// @brief Register a service with factory function
        template<typename TService, typename TImpl>
        void addSingleton()
        {
            addDescriptor(typeid(TService), []() -> std::shared_ptr<void> {
                return std::make_shared<TImpl>();
            }, ServiceLifetime::Singleton);
        }

        /// @brief Register a service with factory function
        template<typename TService>
        void addSingleton(std::function<std::shared_ptr<TService>()> factory)
        {
            addDescriptor(typeid(TService), [factory]() -> std::shared_ptr<void> {
                return factory();
            }, ServiceLifetime::Singleton);
        }

        /// @brief Register a scoped service
        template<typename TService, typename TImpl>
        void addScoped()
        {
            addDescriptor(typeid(TService), []() -> std::shared_ptr<void> {
                return std::make_shared<TImpl>();
            }, ServiceLifetime::Scoped);
        }

        /// @brief Register a transient service
        template<typename TService, typename TImpl>
        void addTransient()
        {
            addDescriptor(typeid(TService), []() -> std::shared_ptr<void> {
                return std::make_shared<TImpl>();
            }, ServiceLifetime::Transient);
        }

        /// @brief Build the service provider
        [[nodiscard]] virtual std::shared_ptr<IServiceProvider> buildServiceProvider() = 0;

    protected:
        virtual void addDescriptor(std::type_index type, ServiceDescriptor::Factory factory, ServiceLifetime lifetime) = 0;
    };
}