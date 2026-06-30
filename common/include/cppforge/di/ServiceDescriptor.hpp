#pragma once
#include <functional>
#include <memory>
#include <typeindex>
#include "ServiceLifetime.hpp"

namespace cppforge::di
{
    /// @brief Service registration descriptor
    class ServiceDescriptor
    {
    public:
        using Factory = std::function<std::shared_ptr<void>()>;

        ServiceDescriptor(std::type_index type, Factory factory, ServiceLifetime lifetime)
            : type_(type), factory_(std::move(factory)), lifetime_(lifetime) {}

        [[nodiscard]] std::type_index type() const { return type_; }
        [[nodiscard]] const Factory& factory() const { return factory_; }
        [[nodiscard]] ServiceLifetime lifetime() const { return lifetime_; }

    private:
        std::type_index type_;
        Factory factory_;
        ServiceLifetime lifetime_;
    };
}