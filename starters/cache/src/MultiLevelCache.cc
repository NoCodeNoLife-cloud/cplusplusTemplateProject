/**
 * @file MultiLevelCache.cc
 * @brief Explicit template instantiations for MultiLevelCache
 * @details Provides explicit instantiations for common key/value types to
 *          ensure linkage when used across translation units.
 */

#include <cppforge/starter/cache/MultiLevelCache.hpp>

namespace cppforge::starter::cache
{
    template class MultiLevelCache<std::string, std::string>;
}
