/**
 * @file CuckooFilter.cc
 * @brief Cuckoo filter �?explicit template instantiations
 * @details Provides explicit instantiations of the CuckooFilter class template
 *          for the most commonly used element types.  This reduces compile
 *          times and symbol emissions in translation units that include the
 *          header.
 *
 * Reference: Fan et al., "Cuckoo Filter: Practically Better Than Bloom" (2014).
 */

#include "CuckooFilter.hpp"

#include <string>

namespace cppforge::data_structure
{

    // ── Explicit instantiations for common key types ───────────────────

    template class CuckooFilter<std::string>;
    template class CuckooFilter<std::string, 4, FingerprintSize<16>>;
    template class CuckooFilter<std::string, 4, FingerprintSize<32>>;

    template class CuckooFilter<int>;
    template class CuckooFilter<unsigned int>;
    template class CuckooFilter<uint64_t>;
    template class CuckooFilter<int64_t>;

} // namespace cppforge::data_structure
