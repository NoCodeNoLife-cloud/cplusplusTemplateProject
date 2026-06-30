#include <cppforge/di/ServiceProvider.hpp>
#include <stdexcept>

namespace cppforge::di
{
    ServiceProvider::ServiceProvider(std::vector<ServiceDescriptor> descriptors)
        : descriptors_(std::move(descriptors))
    {
    }

    bool ServiceProvider::isRegistered(std::type_index type) const
    {
        for (const auto& desc : descriptors_)
        {
            if (desc.type() == type)
                return true;
        }
        return false;
    }

    std::shared_ptr<void> ServiceProvider::resolve(std::type_index type)
    {
        // Find descriptor
        ServiceDescriptor* descriptor = nullptr;
        for (auto& desc : descriptors_)
        {
            if (desc.type() == type)
            {
                descriptor = &desc;
                break;
            }
        }

        if (!descriptor)
        {
            throw std::runtime_error("Service not registered: " + std::string(type.name()));
        }

        // Handle based on lifetime
        switch (descriptor->lifetime())
        {
        case ServiceLifetime::Singleton:
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = singletons_.find(type);
            if (it != singletons_.end())
            {
                return it->second;
            }
            auto instance = descriptor->factory()();
            singletons_[type] = instance;
            return instance;
        }
        case ServiceLifetime::Scoped:
            throw std::logic_error("Scoped lifetime not yet supported. Use Singleton or Transient.");
        case ServiceLifetime::Transient:
            return descriptor->factory()();
        default:
            throw std::runtime_error("Unknown service lifetime");
        }
    }
}