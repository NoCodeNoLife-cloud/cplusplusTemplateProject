/**
 * @file CountMinSketch.cc
 * @brief Count-Min Sketch probabilistic frequency estimator �?explicit instantiations
 * @details This file provides explicit template instantiations for commonly used
 *          CountMinSketch configurations and item types. Forces the compiler to
 *          generate code for these specialisations, catching compilation errors
 *          early and reducing code-bloat in large translation units.
 */

#include <cppforge/data_structure/probabilistic/CountMinSketch.hpp>

#include <string>

namespace cppforge::data_structure::probabilistic
{
    // ── Default configuration ──

    /// @brief Explicit instantiation of the default CountMinSketch
    ///        (Width=65536, Depth=5, HashStd) �?all non-template members.
    template class CountMinSketch<>;

    // ── Template member instantiations for common item types ──
    // These ensure the compiler generates add<T> and estimate<T> for the
    // most frequently used hashable types.

    // Note: parameter types must match exactly: add(const T&, uint32_t)
    // and estimate(const T&) const.
    template void CountMinSketch<>::add<std::string>(const std::string&, uint32_t);
    template void CountMinSketch<>::add<int>(const int&, uint32_t);
    template void CountMinSketch<>::add<uint32_t>(const uint32_t&, uint32_t);
    template void CountMinSketch<>::add<uint64_t>(const uint64_t&, uint32_t);

    template auto CountMinSketch<>::estimate<std::string>(const std::string&) const -> uint32_t;
    template auto CountMinSketch<>::estimate<int>(const int&) const -> uint32_t;
    template auto CountMinSketch<>::estimate<uint32_t>(const uint32_t&) const -> uint32_t;
    template auto CountMinSketch<>::estimate<uint64_t>(const uint64_t&) const -> uint32_t;

} // namespace cppforge::data_structure::probabilistic
