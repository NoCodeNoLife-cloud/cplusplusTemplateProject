#include <cppforge/di/ApplicationContext.hpp>
#include <stdexcept>

namespace cppforge::di
{
    ApplicationContext::ApplicationContext(std::shared_ptr<IServiceProvider> provider)
        : provider_(std::move(provider))
    {
    }

    std::shared_ptr<IServiceProvider> ApplicationContext::getServiceProvider() const
    {
        return provider_;
    }

    void ApplicationContext::run()
    {
        if (running_) return;
        running_ = true;
        for (const auto& callback : startupCallbacks_)
        {
            callback();
        }
    }

    void ApplicationContext::shutdown()
    {
        if (!running_) return;
        running_ = false;
        for (const auto& callback : shutdownCallbacks_)
        {
            callback();
        }
    }

    void ApplicationContext::onStartup(std::function<void()> callback)
    {
        if (running_) throw std::logic_error("Cannot register startup callbacks after run()");
        startupCallbacks_.push_back(std::move(callback));
    }

    void ApplicationContext::onShutdown(std::function<void()> callback)
    {
        if (running_) throw std::logic_error("Cannot register shutdown callbacks after run()");
        shutdownCallbacks_.push_back(std::move(callback));
    }
}