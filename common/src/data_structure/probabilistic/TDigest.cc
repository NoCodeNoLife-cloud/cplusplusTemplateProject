/**
 * @file TDigest.cc
 * @brief T-Digest approximate quantile estimation â€?explicit instantiations
 * @details This file provides an explicit template instantiation for the
 *          default compression parameter, forcing the compiler to generate
 *          all template member functions and catching compilation errors
 *          early.
 */

#include <cppforge/data_structure/probabilistic/TDigest.hpp>

#include <cstdint>

namespace cppforge::data_structure::probabilistic
{
    /// @brief Explicit instantiation for the default compression (100).
    ///
    /// This produces roughly 50 centroids in steady state, offering a
    /// good balance between accuracy (~1% relative error at extremes)
    /// and memory (~1.6 KB).
    template class TDigest<100>;

} // namespace cppforge::data_structure::probabilistic
