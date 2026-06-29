/**
 * @file IBaseEstimator.hpp
 * @brief Abstract interface for cardinality estimators (HyperLogLog, etc.)
 * @details Defines the common contract for all probabilistic cardinality
 *          estimators: insert, estimate, merge, clear, and memoryUsage.
 *          Implementations trade exact accuracy for sub-linear memory usage.
 *          The canonical implementation is HyperLogLog.
 *
 * @par Thread Safety
 * Implementations must document their own thread-safety guarantees.
 * Unless specified otherwise, implementations are not thread-safe.
 */

#pragma once

#include <cstdint>
#include <vector>

namespace cppforge::data_structure::probabilistic
{
    /// @brief Abstract interface for cardinality estimators.
    ///
    /// Provides a common contract for probabilistic data structures that estimate
    /// the number of distinct elements (cardinality) in a multi-set.
    /// Implementations trade exact accuracy for significantly reduced memory usage.
    class IBaseEstimator
    {
    public:
        virtual ~IBaseEstimator() = default;

        /// @brief Inserts a raw byte sequence into the estimator.
        /// @param data Pointer to the data bytes.
        /// @param length Number of bytes to insert.
        virtual void insert(const void* data, std::size_t length) = 0;

        /// @brief Returns the current estimated cardinality.
        /// @return Estimated number of distinct elements inserted so far.
        [[nodiscard]] virtual uint64_t estimate() const = 0;

        /// @brief Merges the state of another estimator into this one.
        ///
        /// After merging, this estimator reflects the union of all distinct
        /// elements observed by both instances. Implementations should verify
        /// that the other estimator is of a compatible type/precision.
        /// @param other The other estimator to merge from.
        virtual void merge(const IBaseEstimator& other) = 0;

        /// @brief Resets the estimator to its initial empty state.
        virtual void clear() = 0;

        /// @brief Returns the approximate memory usage of the estimator.
        /// @return Memory usage in bytes.
        [[nodiscard]] virtual uint64_t memoryUsage() const = 0;
    };

} // namespace cppforge::data_structure::probabilistic
