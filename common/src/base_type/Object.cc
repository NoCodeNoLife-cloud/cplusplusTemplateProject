/**
 * @file Object.cc
 * @brief Object class implementation
 * @details This file contains the implementation of the Object class methods for Base type classes providing fundamental object functionality.
 */

#include "base_type/Object.hpp"

#include <memory>
#include <string>
#include <typeinfo>
#include <glog/logging.h>

namespace common::base_type
{
    const std::type_info& Object::getClass() const
    {
        return typeid(*this);
    }

    bool Object::equals(const Object& other) const
    {
        return this == &other;
    }

    size_t Object::hashCode() const
    {
        return std::hash<const void*>{}(this);
    }

    std::string Object::toString() const
    {
        return std::string{getClass().name()} + "@" + std::to_string(hashCode());
    }

    std::unique_ptr<Object> Object::clone() const
    {
        // Since Object is likely to be used as a base for polymorphic types,
        // we throw an exception to indicate that derived classes should implement this.
        DLOG(WARNING) << "clone() called on base Object class - must be implemented by derived classes";
        throw std::logic_error("clone() must be implemented by derived classes");
    }

    std::string Object::getClassName() const
    {
        return std::string{getClass().name()};
    }

    bool Object::is(const Object& other) const
    {
        return this == &other;
    }
}
