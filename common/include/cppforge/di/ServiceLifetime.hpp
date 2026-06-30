#pragma once

namespace cppforge::di
{
    /// @brief Service lifetime management
    enum class ServiceLifetime
    {
        Singleton,  ///< One instance for the entire application
        Scoped,     ///< One instance per scope
        Transient   ///< New instance every time
    };
}