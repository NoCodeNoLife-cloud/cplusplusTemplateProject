/**
 * @file HyperLogLog.cc
 * @brief HyperLogLog cardinality estimator — explicit instantiations
 * @details This file provides explicit template instantiations for the most
 *          commonly used HyperLogLog precision values.
 */

#include "HyperLogLog.hpp"

namespace common::data_structure::probabilistic
{
    // Explicit instantiations for common precision values.
    // These force the compiler to generate all template member functions,
    // catching compilation errors early and reducing code-bloat in large TUs.
    template class HyperLogLog<10>; // m = 1024,  ~1KB,   ~3.2% error
    template class HyperLogLog<12>; // m = 4096,  ~4KB,   ~1.6% error
    template class HyperLogLog<14>; // m = 16384, ~16KB,  ~0.8% error (default)
    template class HyperLogLog<16>; // m = 65536, ~64KB,  ~0.4% error

} // namespace common::data_structure::probabilistic
