#include "src/base_type/Object.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <format>
#include <typeinfo>
#include <memory>
#include <string>

namespace common::base_type {
    auto Object::getClass() const noexcept -> const std::type_info & {
        return typeid(*this);
    }

    auto Object::equals(const Object &other) const -> bool {
        const bool result = this == &other;
        DLOG(INFO) << fmt::format("Object::equals - comparing {} with {}, result: {}", 
            getClassName(), other.getClassName(), result);
        return result;
    }

    auto Object::hashCode() const noexcept -> size_t {
        return std::hash<const void *>{}(this);
    }

    auto Object::toString() const -> std::string {
        return std::string{getClass().name()} + "@" + std::to_string(hashCode());
    }

    auto Object::clone() const -> std::unique_ptr<Object> {
        DLOG(ERROR) << fmt::format("clone() called on base Object class ({}). Derived classes must override this method.", getClassName());
        // Since Object is likely to be used as a base for polymorphic types, 
        // we throw an exception to indicate that derived classes should implement this.
        throw std::logic_error("clone() must be implemented by derived classes");
    }

    auto Object::isInstance(const std::type_info &target_type) const noexcept -> bool {
        const bool result = target_type == typeid(*this);
        DLOG(INFO) << fmt::format("Object::isInstance - checking if {} is instance of {}, result: {}", 
            getClassName(), target_type.name(), result);
        return result;
    }

    auto Object::getClassName() const -> std::string {
        return std::string{getClass().name()};
    }

    auto Object::is(const Object &other) const noexcept -> bool {
        const bool result = this == &other;
        DLOG(INFO) << fmt::format("Object::is - reference comparison between {} instances, result: {}", 
            getClassName(), result);
        return result;
    }
}
