#pragma once
#include "IServiceProvider.hpp"
#include "ServiceCollection.hpp"
#include <memory>
#include <functional>

namespace cppforge::di
{
    /// @brief Application context with lifecycle management
    class ApplicationContext
    {
    public:
        explicit ApplicationContext(std::shared_ptr<IServiceProvider> provider);

        /// @brief Get the service provider
        [[nodiscard]] std::shared_ptr<IServiceProvider> getServiceProvider() const;

        /// @brief Resolve a service
        template<typename T>
        [[nodiscard]] std::shared_ptr<T> getService()
        {
            return provider_->getService<T>();
        }

        /// @brief Run the application
        void run();

        /// @brief Shutdown the application
        void shutdown();

        /// @brief Register startup callback
        void onStartup(std::function<void()> callback);

        /// @brief Register shutdown callback
        void onShutdown(std::function<void()> callback);

    private:
        std::shared_ptr<IServiceProvider> provider_;
        std::vector<std::function<void()>> startupCallbacks_;
        std::vector<std::function<void()>> shutdownCallbacks_;
        bool running_ = false;
    };
}