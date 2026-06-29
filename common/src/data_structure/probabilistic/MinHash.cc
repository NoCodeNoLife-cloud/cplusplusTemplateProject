/**
 * @file MinHash.cc
 * @brief MinHash probabilistic Jaccard similarity estimator �?explicit instantiations
 * @details This file provides explicit template instantiations for commonly used
 *          MinHash configurations and item types. Forces the compiler to generate
 *          code for these specialisations, catching compilation errors early and
 *          reducing code-bloat in large translation units.
 */

#include <cppforge/data_structure/probabilistic/MinHash.hpp>

#include <string>

namespace cppforge::data_structure::probabilistic
{
    // ── Default configuration ──

    /// @brief Explicit instantiation of the default MinHash
    ///        (SignatureSize=128, MinHashDefaultHash) �?all non-template members.
    template class MinHash<>;

    // ── Template member instantiations for common item types ──
    // These ensure the compiler generates insert<T> for the most frequently
    // used hashable types.

    template void MinHash<>::insert<std::string>(const std::string&);
    template void MinHash<>::insert<int>(const int&);
    template void MinHash<>::insert<uint32_t>(const uint32_t&);
    template void MinHash<>::insert<uint64_t>(const uint64_t&);

} // namespace cppforge::data_structure::probabilistic
