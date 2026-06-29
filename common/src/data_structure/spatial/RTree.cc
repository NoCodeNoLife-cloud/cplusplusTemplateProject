/**
 * @file RTree.cc
 * @brief RTree explicit template instantiations
 * @details Provides explicit instantiations of the RTree template for
 *          common payload types and dimensionalities so that downstream
 *          translation units can link against pre-compiled code without
 *          requiring the full template definition in every TU.
 *
 * To add a new instantiation, append the corresponding
 * @code
 *   template class RTree<YourType, Dims, MaxEntries>;
 * @endcode
 * below.
 */

#include <cppforge/data_structure/spatial/RTree.hpp>

namespace cppforge::data_structure::spatial
{

// ── 2-D instantiations ─────────────────────────────────────────────────

/// @cond INSTANTIATIONS

// Common 2-D payloads with default MaxEntries = 10.
template class RTree<int, 2, 10>;
template class RTree<unsigned int, 2, 10>;
template class RTree<long, 2, 10>;
template class RTree<unsigned long, 2, 10>;
template class RTree<long long, 2, 10>;
template class RTree<float, 2, 10>;
template class RTree<double, 2, 10>;
template class RTree<void*, 2, 10>;

// 2-D with larger node capacity.
template class RTree<int, 2, 32>;
template class RTree<int, 2, 64>;
template class RTree<double, 2, 32>;
template class RTree<double, 2, 64>;

// ── 3-D instantiations ─────────────────────────────────────────────────

template class RTree<int, 3, 10>;
template class RTree<double, 3, 10>;

// ── 4-D instantiations ─────────────────────────────────────────────────

template class RTree<float, 4, 10>;
template class RTree<double, 4, 10>;

/// @endcond

} // namespace cppforge::data_structure::spatial
