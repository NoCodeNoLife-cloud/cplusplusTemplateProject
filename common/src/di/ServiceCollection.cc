#include <cppforge/di/ServiceCollection.hpp>

namespace cppforge::di
{
    void ServiceCollection::addDescriptor(std::type_index type, ServiceDescriptor::Factory factory, ServiceLifetime lifetime)
    {
        descriptors_.emplace_back(type, std::move(factory), lifetime);
    }

    std::shared_ptr<IServiceProvider> ServiceCollection::buildServiceProvider()
    {
        return std::make_shared<ServiceProvider>(std::move(descriptors_));
    }
}