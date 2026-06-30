#pragma once
#include "IServiceProvider.hpp"
#include "ServiceDescriptor.hpp"
#include <unordered_map>
#include <vector>
#include <mutex>

namespace cppforge::di
{
    /// @brief Service provider implementation with lifetime management
    class ServiceProvider : public IServiceProvider
    {
    public:
        explicit ServiceProvider(std::vector<ServiceDescriptor> descriptors);

        [[nodiscard]] bool isRegistered(std::type_index type) const override;

    protected:
        [[nodiscard]] std::shared_ptr<void> resolve(std::type_index type) override;

    private:
        std::vector<ServiceDescriptor> descriptors_;
        std::unordered_map<std::type_index, std::shared_ptr<void>> singletons_;
        mutable std::mutex mutex_;
    };
}