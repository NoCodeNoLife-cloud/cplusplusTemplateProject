#pragma once
#include "IServiceCollection.hpp"
#include "ServiceProvider.hpp"
#include <unordered_map>
#include <vector>

namespace cppforge::di
{
    /// @brief Service collection implementation
    class ServiceCollection : public IServiceCollection
    {
    public:
        std::shared_ptr<IServiceProvider> buildServiceProvider() override;

    protected:
        void addDescriptor(std::type_index type, ServiceDescriptor::Factory factory, ServiceLifetime lifetime) override;

    private:
        std::vector<ServiceDescriptor> descriptors_;
    };
}