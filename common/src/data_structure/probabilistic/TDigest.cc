/**
 * @file TDigest.cc
 * @brief T-Digest approximate quantile estimation — explicit instantiations
 * @details This file provides an explicit template instantiation for the
 *          default compression parameter, forcing the compiler to generate
 *          all template member functions and catching compilation errors
 *          early.
 */

#include "TDigest.hpp"

#include <cstdint>

namespace common::data_structure::probabilistic
{
    /// @brief Explicit instantiation for the default compression (100).
    ///
    /// This produces roughly 50 centroids in steady state, offering a
    /// good balance between accuracy (~1% relative error at extremes)
    /// and memory (~1.6 KB).
    template class TDigest<100>;

} // namespace common::data_structure::probabilistic
