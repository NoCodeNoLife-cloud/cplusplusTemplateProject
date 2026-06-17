/**
 * @file PersistentVector.hpp
 * @brief Persistent (immutable) vector backed by an RRB-Tree
 * @details A persistent vector implementation based on the Relaxed Radix-Balanced
 *          Tree (RRB-Tree).  All mutating operations return a new vector while
 *          preserving the original, enabling efficient structural sharing.
 *          Access, update, push_back, and pop_back operations are O(log n) or
 *          near O(1) in practice due to the wide branching factor.
 *
 * @par Thread Safety
 * This class is **thread-safe** for concurrent read access since all instances
 * are immutable after construction.  No external synchronisation is required
 * when sharing instances across threads.
 *
 * @par Complexity
 * - get / set:   O(log n), practically near O(1) with branching factor ~32
 * - push_back:   amortised near O(1)
 * - pop_back:    amortised near O(1)
 * - size/empty:  O(1)
 *
 * @par Reference
 * - "RRB-Trees: Efficient Immutable Vectors" by Phil Bagwell (2011)
 * - Clojure's PersistentVector implementation
 */

#pragma once

#include <cstddef>   // size_t
#include <memory>    // std::allocator, std::unique_ptr, std::shared_ptr, std::construct_at, std::destroy_n
#include <new>       // std::launder
#include <stdexcept> // std::out_of_range
#include <iterator>  // std::random_access_iterator_tag
#include <utility>   // std::forward, std::move, std::exchange
#include <vector>    // internal node storage

namespace common::data_structure::persistent
{

// ═══════════════════════════════════════════════════════════════════════════════
// PersistentVector — immutable vector with structural sharing
// ═══════════════════════════════════════════════════════════════════════════════

/// @brief A persistent (immutable) vector backed by an RRB-Tree
/// @details This class implements an immutable vector where every modification
///          returns a new vector, leaving the original unchanged.  The internal
///          RRB-Tree structure enables near O(1) access and efficient structural
///          sharing between versions.
/// @tparam T The type of elements stored in the vector
template <typename T>
class PersistentVector
{
public:
    /// @brief Type of the stored elements.
    using value_type = T;

    /// @brief Unsigned integer type for sizes and indices.
    using size_type = std::size_t;

    // -----------------------------------------------------------------------
    // Construction / Copy / Move
    // -----------------------------------------------------------------------

    /// @brief Default constructor — creates an empty vector
    PersistentVector() noexcept
        : root_(nullptr)
        , tail_([]{
            auto n = std::make_shared<Node>(true);
            n->count_ = 0;
            return n;
        }())
        , size_(0)
        , shift_(0)
    {}

    /// @brief Construct from an initializer list
    /// @param init  List of elements to copy into the vector
    /// @throws std::bad_alloc if allocation fails, or any exception thrown by
    ///         T's copy constructor.
    /// @par Complexity
    ///   O(N) — each element is appended via push_back (amortised near O(1)
    ///   per element).
    explicit PersistentVector(std::initializer_list<T> init)
        : PersistentVector()
    {
        for (const auto& v : init)
        {
            *this = std::move(push_back(v));
        }
    }

    /// @brief Copy constructor — deleted (persistent vectors are not copied;
    ///        structural sharing is implicit through NodePtr).
    PersistentVector(const PersistentVector&) = delete;

    /// @brief Copy assignment — deleted.
    auto operator=(const PersistentVector&) -> PersistentVector& = delete;

    /// @brief Move constructor
    /// @note  The moved-from vector is left in an empty state via the
    ///        noexcept emptyTail() singleton, preserving the noexcept
    ///        guarantee.
    PersistentVector(PersistentVector&& other) noexcept
        : root_(std::move(other.root_))
        , tail_(std::move(other.tail_))
        , size_(other.size_)
        , shift_(other.shift_)
    {
        other.size_  = 0;
        other.shift_ = 0;
        other.tail_  = emptyTail();
        other.root_  = nullptr;
    }

    /// @brief Move assignment
    auto operator=(PersistentVector&& other) noexcept -> PersistentVector&
    {
        if (this != &other)
        {
            root_  = std::move(other.root_);
            tail_  = std::move(other.tail_);
            size_  = other.size_;
            shift_ = other.shift_;

            other.size_  = 0;
            other.shift_ = 0;
            other.tail_  = emptyTail();
            other.root_  = nullptr;
        }
        return *this;
    }

    /// @brief Destructor — defaulted; shared_ptr releases the tree nodes.
    ~PersistentVector() = default;

    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    /// @brief Number of bits per branch level.
    static constexpr size_t BRANCH_BITS = 5;

    /// @brief Number of elements per node (2^BRANCH_BITS = 32).
    static constexpr size_t BRANCH_SIZE = 1 << BRANCH_BITS;

    /// @brief Bitmask to compute the index within a node (BRANCH_SIZE - 1).
    static constexpr size_t BRANCH_MASK = BRANCH_SIZE - 1;

    // -----------------------------------------------------------------------
    // Node types
    // -----------------------------------------------------------------------

    /// @brief Internal node of the RRB-Tree — either leaf (stores T values) or
    ///        internal (stores child NodePtrs).
    struct Node;

    /// @brief Shared pointer to an immutable node.
    using NodePtr = std::shared_ptr<const Node>;

    /// @brief Full definition of the RRB-Tree node with manual lifetime management.
    ///
    /// Storage uses a raw byte buffer with proper alignment for both T and NodePtr.
    /// The active member is determined by `leaf_`: when true, the buffer holds
    /// an array of BRANCH_SIZE elements of type T; when false, it holds an array
    /// of BRANCH_SIZE elements of type NodePtr.
    ///
    /// Construction and destruction of individual elements are performed manually
    /// via std::construct_at / std::destroy_n, enabling correct lifetime management
    /// for non-trivial types such as std::string or std::shared_ptr.
    ///
    /// @par Thread Safety
    /// Nodes are immutable after construction and safe for concurrent read access.
    struct Node final
    {
        /// @brief Size of the raw storage buffer in bytes — enough for the larger
        ///        of sizeof(T) * BRANCH_SIZE and sizeof(NodePtr) * BRANCH_SIZE.
        static constexpr size_t kStorageSize = sizeof(T) * BRANCH_SIZE > sizeof(NodePtr) * BRANCH_SIZE
                                                   ? sizeof(T) * BRANCH_SIZE
                                                   : sizeof(NodePtr) * BRANCH_SIZE;

        /// @brief Alignment required for the storage buffer — max of alignof(T)
        ///        and alignof(NodePtr).
        static constexpr size_t kStorageAlign = alignof(T) > alignof(NodePtr) ? alignof(T) : alignof(NodePtr);

        alignas(kStorageAlign) std::byte storage_[kStorageSize];
        size_t count_;
        bool   leaf_;

        /// @brief Construct a node with the given type flag.
        /// @param leaf True for leaf node (stores T), false for internal node (stores NodePtr).
        explicit Node(bool leaf) noexcept : storage_{}, count_(0), leaf_(leaf) {}

        Node(const Node&)            = delete;
        auto operator=(const Node&) -> Node& = delete;

        ~Node()
        {
            if (leaf_)
            {
                std::destroy_n(values(), count_);
            }
            else
            {
                std::destroy_n(children(), count_);
            }
        }

        /// @brief Returns a pointer to the T array (valid only when leaf_ is true).
        [[nodiscard]] auto values() noexcept -> T*
        {
            return std::launder(reinterpret_cast<T*>(storage_));
        }

        /// @brief Returns a const pointer to the T array (valid only when leaf_ is true).
        [[nodiscard]] auto values() const noexcept -> const T*
        {
            return std::launder(reinterpret_cast<const T*>(storage_));
        }

        /// @brief Returns a pointer to the NodePtr array (valid only when leaf_ is false).
        [[nodiscard]] auto children() noexcept -> NodePtr*
        {
            return std::launder(reinterpret_cast<NodePtr*>(storage_));
        }

        /// @brief Returns a const pointer to the NodePtr array (valid only when leaf_ is false).
        [[nodiscard]] auto children() const noexcept -> const NodePtr*
        {
            return std::launder(reinterpret_cast<const NodePtr*>(storage_));
        }
    };

    // -----------------------------------------------------------------------
    // Capacity
    // -----------------------------------------------------------------------

    /// @brief Returns the number of elements in the vector.
    [[nodiscard]] auto size() const noexcept -> size_type
    {
        return size_;
    }

    /// @brief Returns true if the vector contains no elements.
    [[nodiscard]] auto empty() const noexcept -> bool
    {
        return size_ == 0;
    }

    // -----------------------------------------------------------------------
    // Element access
    // -----------------------------------------------------------------------

    /// @brief Returns a const reference to the element at the given index.
    /// @param index  Zero-based index of the element to retrieve.
    /// @return Const reference to the element at @p index.
    /// @throws std::out_of_range if @p index is >= size().
    [[nodiscard]] auto get(size_type index) const -> const T&
    {
        if (index >= size_)
            throw std::out_of_range("PersistentVector::get: index out of range");
        if (index >= tailOffset(size_))
            return tail_->values()[index & BRANCH_MASK];
        return doGet(index);
    }

    /// @brief Returns a const reference to the element at the given index.
    ///        (Bounds checking is performed — throws on invalid index.)
    /// @param index  Zero-based index.
    /// @return Const reference to the element at @p index.
    /// @throws std::out_of_range if @p index is >= size().
    [[nodiscard]] auto operator[](size_type index) const -> const T&
    {
        return get(index);
    }

    /// @brief Returns a const reference to the first element.
    /// @throws std::out_of_range if the vector is empty.
    [[nodiscard]] auto front() const -> const T&
    {
        if (size_ == 0)
            throw std::out_of_range("PersistentVector::front: empty vector");
        return get(0);
    }

    /// @brief Returns a const reference to the last element.
    /// @throws std::out_of_range if the vector is empty.
    [[nodiscard]] auto back() const -> const T&
    {
        if (size_ == 0)
            throw std::out_of_range("PersistentVector::back: empty vector");
        return get(size_ - 1);
    }

    // -----------------------------------------------------------------------
    // Iterators
    // -----------------------------------------------------------------------

    /// @brief Random-access const iterator for PersistentVector.
    /// @details Dereferencing is O(log_32 n). All other operations are O(1).
    ///          This is the only iterator type (mutable iteration is unsupported
    ///          due to immutability).
    class const_iterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const T*;
        using reference         = const T&;

        /// @brief Default constructor.
        const_iterator() noexcept = default;

        /// @brief Dereference — returns a const reference to the element at the
        ///        current position.
        [[nodiscard]] auto operator*() const noexcept -> reference
        { return (*vector_)[index_]; }

        /// @brief Arrow operator — returns a const pointer to the element.
        [[nodiscard]] auto operator->() const noexcept -> pointer
        { return &(*vector_)[index_]; }

        /// @brief Random-access subscript.
        [[nodiscard]] auto operator[](difference_type n) const noexcept -> reference
        { return (*vector_)[index_ + static_cast<size_type>(n)]; }

        // -- Increment / Decrement ---------------------------------------------

        auto operator++() noexcept -> const_iterator&
        { ++index_; return *this; }

        auto operator++(int) noexcept -> const_iterator
        { auto tmp = *this; ++*this; return tmp; }

        auto operator--() noexcept -> const_iterator&
        { --index_; return *this; }

        auto operator--(int) noexcept -> const_iterator
        { auto tmp = *this; --*this; return tmp; }

        // -- Compound assignment -----------------------------------------------

        auto operator+=(difference_type n) noexcept -> const_iterator&
        { index_ += static_cast<size_type>(n); return *this; }

        auto operator-=(difference_type n) noexcept -> const_iterator&
        { index_ -= static_cast<size_type>(n); return *this; }

        // -- Iterator arithmetic (friend) --------------------------------------

        [[nodiscard]] friend auto operator+(const const_iterator& it, difference_type n) noexcept -> const_iterator
        { return const_iterator(it.vector_, it.index_ + static_cast<size_type>(n)); }

        [[nodiscard]] friend auto operator+(difference_type n, const const_iterator& it) noexcept -> const_iterator
        { return const_iterator(it.vector_, it.index_ + static_cast<size_type>(n)); }

        [[nodiscard]] friend auto operator-(const const_iterator& it, difference_type n) noexcept -> const_iterator
        { return const_iterator(it.vector_, it.index_ - static_cast<size_type>(n)); }

        [[nodiscard]] friend auto operator-(const const_iterator& lhs, const const_iterator& rhs) noexcept -> difference_type
        { return static_cast<difference_type>(lhs.index_) - static_cast<difference_type>(rhs.index_); }

        // -- Comparison (friend) -----------------------------------------------

        [[nodiscard]] friend auto operator==(const const_iterator& lhs, const const_iterator& rhs) noexcept -> bool
        { return lhs.index_ == rhs.index_; }

        [[nodiscard]] friend auto operator!=(const const_iterator& lhs, const const_iterator& rhs) noexcept -> bool
        { return lhs.index_ != rhs.index_; }

        [[nodiscard]] friend auto operator<(const const_iterator& lhs, const const_iterator& rhs) noexcept -> bool
        { return lhs.index_ < rhs.index_; }

        [[nodiscard]] friend auto operator<=(const const_iterator& lhs, const const_iterator& rhs) noexcept -> bool
        { return lhs.index_ <= rhs.index_; }

        [[nodiscard]] friend auto operator>(const const_iterator& lhs, const const_iterator& rhs) noexcept -> bool
        { return lhs.index_ > rhs.index_; }

        [[nodiscard]] friend auto operator>=(const const_iterator& lhs, const const_iterator& rhs) noexcept -> bool
        { return lhs.index_ >= rhs.index_; }

    private:
        friend class PersistentVector;

        /// @brief Private constructor — only PersistentVector may create iterators.
        const_iterator(const PersistentVector* vec, size_type index) noexcept
            : vector_(vec)
            , index_(index)
        {}

        const PersistentVector* vector_ = nullptr;
        size_type               index_  = 0;
    };

    // -- begin / end ---------------------------------------------------------

    /// @brief Returns a const iterator to the first element.
    [[nodiscard]] auto begin() const noexcept -> const_iterator
    { return const_iterator(this, 0); }

    /// @brief Returns a const iterator past the last element.
    [[nodiscard]] auto end() const noexcept -> const_iterator
    { return const_iterator(this, size_); }

    /// @brief Returns a const iterator to the first element (same as begin()).
    [[nodiscard]] auto cbegin() const noexcept -> const_iterator
    { return begin(); }

    /// @brief Returns a const iterator past the last element (same as end()).
    [[nodiscard]] auto cend() const noexcept -> const_iterator
    { return end(); }

    // -----------------------------------------------------------------------
    // Persistent modification operations — return a new vector
    // -----------------------------------------------------------------------

    /// @brief Returns a new vector with @p value appended at the end.
    /// @param value  The element to append (copy-constructed into the result).
    /// @return A new PersistentVector containing all original elements followed
    ///         by @p value.
    /// @throws std::bad_alloc if allocation fails, or any exception thrown by
    ///         T's copy constructor.
    /// @par Complexity
    ///   Amortised near O(1) — the tail block absorbs most appends; bulk
    ///   flushes into the RRB-tree occur only every BRANCH_SIZE-th append.
    [[nodiscard]] auto push_back(const T& value) const -> PersistentVector
        { return pushBackImpl(value); }

    /// @brief Returns a new vector with @p value appended at the end (move
    ///        version).
    /// @param value  The element to append (move-constructed into the result).
    /// @return A new PersistentVector containing all original elements followed
    ///         by @p value.
    /// @throws std::bad_alloc if allocation fails, or any exception thrown by
    ///         T's move constructor.
    [[nodiscard]] auto push_back(T&& value) const -> PersistentVector
        { return pushBackImpl(std::move(value)); }

    /// @brief Returns a new vector with the element at @p index replaced by @p value
    ///        (copy version).
    /// @param index  Zero-based index of the element to replace.
    /// @param value  The new value to place at @p index.
    /// @return A new PersistentVector with the element at @p index replaced.
    /// @throws std::out_of_range if @p index is >= size().
    /// @par Complexity
    ///   O(log n) — path-copying traverses O(log n) nodes.
    [[nodiscard]] auto update(size_type index, const T& value) const -> PersistentVector
    {
        if (index >= size_)
            throw std::out_of_range("PersistentVector::update: index out of range");

        // ── Case 1: index in tail block ────────────────────────────────────
        if (index >= tailOffset(size_))
        {
            auto newTail = std::make_shared<Node>(true);
            {
                size_type count = 0;
                try {
                    for (; count < tail_->count_; ++count)
                    {
                        if (count == (index & BRANCH_MASK))
                            std::construct_at(&newTail->values()[count], value);
                        else
                            std::construct_at(&newTail->values()[count], tail_->values()[count]);
                    }
                } catch (...) {
                    std::destroy_n(newTail->values(), count);
                    throw;
                }
                newTail->count_ = tail_->count_;
            }
            return PersistentVector(root_, std::move(newTail), size_, shift_);
        }

        // ── Case 2: index in tree — path-copy ──────────────────────────────
        auto newRoot = doUpdate(root_, shift_, index, value);
        return PersistentVector(std::move(newRoot), tail_, size_, shift_);
    }

    /// @brief Returns a new vector with the element at @p index replaced by @p value
    ///        (move version).
    /// @param index  Zero-based index of the element to replace.
    /// @param value  The new value to move into @p index.
    /// @return A new PersistentVector with the element at @p index replaced.
    /// @throws std::out_of_range if @p index is >= size().
    /// @par Complexity
    ///   O(log n) — path-copying traverses O(log n) nodes.
    [[nodiscard]] auto update(size_type index, T&& value) const -> PersistentVector
    {
        if (index >= size_)
            throw std::out_of_range("PersistentVector::update: index out of range");

        // ── Case 1: index in tail block ────────────────────────────────────
        if (index >= tailOffset(size_))
        {
            auto newTail = std::make_shared<Node>(true);
            {
                size_type count = 0;
                try {
                    for (; count < tail_->count_; ++count)
                    {
                        if (count == (index & BRANCH_MASK))
                            std::construct_at(&newTail->values()[count], std::move(value));
                        else
                            std::construct_at(&newTail->values()[count], tail_->values()[count]);
                    }
                } catch (...) {
                    std::destroy_n(newTail->values(), count);
                    throw;
                }
                newTail->count_ = tail_->count_;
            }
            return PersistentVector(root_, std::move(newTail), size_, shift_);
        }

        // ── Case 2: index in tree — path-copy ──────────────────────────────
        auto newRoot = doUpdate(root_, shift_, index, std::move(value));
        return PersistentVector(std::move(newRoot), tail_, size_, shift_);
    }

    /// @brief Returns a new vector with the last element removed.
    /// @return A new PersistentVector containing all elements except the last.
    /// @throws std::out_of_range if the vector is empty.
    /// @par Complexity
    ///   O(log n) — path-copying traverses O(log n) nodes.
    [[nodiscard]] auto pop_back() const -> PersistentVector
    {
        if (size_ == 0)
            throw std::out_of_range("PersistentVector::pop_back: empty vector");

        // ── Case 1: tail has more than 1 element ───────────────────────────
        if (tail_->count_ > 1)
        {
            auto newTail = std::make_shared<Node>(true);
            {
                size_type count = 0;
                try {
                    const auto newCount = tail_->count_ - 1;
                    for (; count < newCount; ++count)
                        std::construct_at(&newTail->values()[count], tail_->values()[count]);
                } catch (...) {
                    std::destroy_n(newTail->values(), count);
                    throw;
                }
                newTail->count_ = tail_->count_ - 1;
            }
            return PersistentVector(root_, std::move(newTail), size_ - 1, shift_);
        }

        // ── Case 2: single element — result is empty ───────────────────────
        if (size_ == 1)
        {
            return PersistentVector();
        }

        // ── Case 3: tail has exactly 1 element — pop leaf from tree ────────
        auto [newRoot, poppedLeaf] = doPopBack(root_, shift_);

        // Adjust shift — collapse single-child internal nodes when possible.
        auto newShift = shift_;
        while (newRoot && !newRoot->leaf_ && newRoot->count_ == 1 && newShift > BRANCH_BITS)
        {
            newRoot = newRoot->children()[0];
            newShift -= BRANCH_BITS;
        }
        if (!newRoot || newRoot->count_ == 0)
        {
            newRoot  = nullptr;
            newShift = 0;
        }

        // poppedLeaf becomes the new tail
        return PersistentVector(std::move(newRoot), std::move(poppedLeaf), size_ - 1, newShift);
    }

    // -----------------------------------------------------------------------
    // Bulk operations
    // -----------------------------------------------------------------------

    /// @brief Returns a new vector containing elements in the range [first, last).
    /// @param first  Zero-based index of the first element to include.
    /// @param last   Zero-based index past the last element to include.
    /// @return A new PersistentVector with elements [first, last) from this vector.
    /// @throws std::out_of_range if @p first > @p last or @p last > size().
    /// @par Complexity
    ///   O((last - first) * log n) — each element is retrieved via O(log n) tree
    ///   traversal and appended via amortised near-O(1) push_back.
    /// @par Exception Safety
    ///   Strong guarantee — a local vector is built incrementally; if any
    ///   operation throws, the local vector is destroyed and *this is unchanged.
    [[nodiscard]] auto sub_vector(size_type first, size_type last) const -> PersistentVector
    {
        if (first > last || last > size_)
            throw std::out_of_range("PersistentVector::sub_vector: invalid range");

        auto result = PersistentVector();
        for (size_type i = first; i < last; ++i)
            result = std::move(result.push_back(get(i)));
        return result;
    }

    /// @brief Returns a new vector concatenating this vector with @p other.
    /// @details Constructs the result by copying the internal root and tail
    ///          pointers (shallow structural sharing), then appends each element
    ///          of @p other via push_back.
    /// @param other  The vector whose elements are appended after this vector's.
    /// @return A new PersistentVector with the concatenated result.
    /// @throws std::bad_alloc if allocation fails, or any exception thrown by
    ///         T's copy constructor.
    /// @par Complexity
    ///   O(m * log n) where m = other.size() — each element from @p other is
    ///   retrieved and appended via amortised near-O(1) push_back.
    /// @par Exception Safety
    ///   Strong guarantee — a local vector is built incrementally; if any
    ///   operation throws, the local vector is destroyed and *this is unchanged.
    [[nodiscard]] auto concat(const PersistentVector& other) const -> PersistentVector
    {
        // Shallow-copy *this — share the tree/tail nodes via shared_ptr.
        auto result = PersistentVector(root_, tail_, size_, shift_);
        for (const auto& elem : other)
            result = std::move(result.push_back(elem));
        return result;
    }

private:
    // ───────────────────────────────────────────────────────────────────────
    // Data members
    // ───────────────────────────────────────────────────────────────────────

    NodePtr   root_;   ///< Root of the internal RRB-tree (nullptr when empty).
    NodePtr   tail_;   ///< Tail leaf-block (0 … BRANCH_SIZE elements).
    size_type size_;   ///< Total number of elements in the vector.
    size_type shift_;  ///< Bits of tree depth (0 = no tree / tree is flat).

    // ───────────────────────────────────────────────────────────────────────
    // Empty tail singleton (used by move operations)
    // ───────────────────────────────────────────────────────────────────────

    /// @brief Returns a shared empty tail node for moved-from vectors.
    /// @details This static singleton avoids heap allocation in the move
    ///          constructor and move-assignment operator, preserving their
    ///          noexcept guarantee.
    /// @return A shared pointer to a leaf node with count_ == 0.
    static auto emptyTail() noexcept -> NodePtr
    {
        static const auto node = []{
            auto n = std::make_shared<Node>(true);
            n->count_ = 0;
            return n;
        }();
        return node;
    }

    // ───────────────────────────────────────────────────────────────────────
    // Internal constructor (used by persistent operations)
    // ───────────────────────────────────────────────────────────────────────

    /// @brief Creates a vector from fully-formed internal state.
    /// @param root   Root node of the tree (may be nullptr).
    /// @param tail   Tail leaf block.
    /// @param size   Total element count.
    /// @param shift  Tree depth in bits.
    PersistentVector(NodePtr root, NodePtr tail, size_type size, size_type shift) noexcept
        : root_(std::move(root))
        , tail_(std::move(tail))
        , size_(size)
        , shift_(shift)
    {}

    // ───────────────────────────────────────────────────────────────────────
    // Helper methods
    // ───────────────────────────────────────────────────────────────────────

    /// @brief Common implementation for push_back (copy and move versions).
    /// @tparam U  Deduced forwarding reference type (T const& or T&&).
    /// @param value  The element to append.
    /// @return A new PersistentVector with @p value appended.
    template <typename U>
    [[nodiscard]] auto pushBackImpl(U&& value) const -> PersistentVector
    {
        // ── Case 1: tail has room ─────────────────────────────────────────
        if (tail_->count_ < BRANCH_SIZE)
        {
            auto newTail = std::make_shared<Node>(true);
            size_type count = 0;
            try {
                for (; count < tail_->count_; ++count)
                    std::construct_at(&newTail->values()[count], tail_->values()[count]);
                std::construct_at(&newTail->values()[count], std::forward<U>(value));
                ++count;
            } catch (...) {
                std::destroy_n(newTail->values(), count);
                throw;
            }
            newTail->count_ = count;

            return PersistentVector(root_, std::move(newTail), size_ + 1, shift_);
        }

        // ── Case 2: tail is full — flush it into the tree ─────────────────
        // 2a. Create a full leaf (32 elements) from the current tail.
        auto newLeaf = std::make_shared<Node>(true);
        size_type leafCount = 0;
        try {
            for (; leafCount < BRANCH_SIZE; ++leafCount)
                std::construct_at(&newLeaf->values()[leafCount], tail_->values()[leafCount]);
        } catch (...) {
            std::destroy_n(newLeaf->values(), leafCount);
            throw;
        }
        newLeaf->count_ = BRANCH_SIZE;

        // 2b. Insert the new leaf into the tree.
        const auto leafIdx = tailOffset(size_);

        NodePtr    newRoot;
        size_type  newShift;

        if (!root_)
        {
            // First leaf in the tree — create a path (internal → leaf).
            newRoot  = newPath(std::move(newLeaf), BRANCH_BITS, leafIdx);
            newShift = BRANCH_BITS;
        }
        else
        {
            // Incorporate the leaf into the existing tree, elevating the root
            // if the index range requires an extra level.
            auto workingRoot  = root_;
            auto workingShift = shift_;

            while ((leafIdx >> workingShift) >= BRANCH_SIZE)
            {
                auto elevated        = std::make_shared<Node>(false);
                std::construct_at(elevated->children() + 0, std::move(workingRoot));
                elevated->count_     = 1;
                workingRoot          = std::move(elevated);
                workingShift        += BRANCH_BITS;
            }

            newRoot  = pushLeaf(workingRoot, workingShift, std::move(newLeaf), leafIdx);
            newShift = workingShift;
        }

        // 2c. Create a brand-new tail containing only the new value.
        auto newTail = std::make_shared<Node>(true);
        std::construct_at(&newTail->values()[0], std::forward<U>(value));
        newTail->count_ = 1;

        return PersistentVector(std::move(newRoot), std::move(newTail), size_ + 1, newShift);
    }

    /// @brief Returns the start index of the tail block.
    ///
    /// Elements with index < tailOffset() are stored in the RRB-tree;
    /// elements with index >= tailOffset() are stored in the tail block.
    ///
    /// @param sz  Current size of the vector.
    /// @return Index of the first element that resides in the tail.
    [[nodiscard]] static auto tailOffset(size_type sz) noexcept -> size_type
    {
        if (sz == 0) return 0;
        return ((sz - 1) / BRANCH_SIZE) * BRANCH_SIZE;
    }

    /// @brief Retrieves an element from the RRB-tree (precondition:
    ///        index < tailOffset(size_)).
    /// @return Const reference to the element at @p index.
    auto doGet(size_type index) const -> const T&
    {
        auto node  = root_;
        auto level = shift_;
        while (level > 0)
        {
            const auto childIdx = (index >> level) & BRANCH_MASK;
            node = node->children()[childIdx];
            level -= BRANCH_BITS;
        }
        const auto leafIdx = index & BRANCH_MASK;
        return node->values()[leafIdx];
    }

    /// @brief Creates a chain of internal nodes from the given shift down to
    ///        a leaf, placing the leaf at the correct position determined by
    ///        @p idx.
    /// @param leaf  The leaf node to place at the bottom of the path.
    /// @param shift  Current bit-depth of the path being created.
    /// @param idx   Global index that determines the path's child indices.
    /// @return The root of the newly created path.
    static auto newPath(NodePtr leaf, size_type shift, size_type idx) -> NodePtr
    {
        if (shift == 0) return leaf;
        auto internal        = std::make_shared<Node>(false);
        const auto childIdx  = (idx >> shift) & BRANCH_MASK;
        auto* const dest     = internal->children();
        auto child           = newPath(std::move(leaf), shift - BRANCH_BITS, idx);
        std::construct_at(dest + childIdx, std::move(child));
        internal->count_     = 1;
        return internal;
    }

    /// @brief Inserts a leaf node into an existing tree (path-copying).
    /// @param root   The root of the tree to insert into.
    /// @param shift  Bit-depth of @p root.
    /// @param leaf   The leaf node to insert.
    /// @param idx    Global index that determines the leaf's position.
    /// @return The new tree root with the leaf inserted (path-copied).
    static auto pushLeaf(NodePtr root, size_type shift, NodePtr leaf, size_type idx) -> NodePtr
    {
        auto newRoot      = std::make_shared<Node>(false);
        auto* const dest  = newRoot->children();
        // Copy all existing child pointers (construct into raw storage).
        {
            size_type count = 0;
            try {
                for (; count < root->count_; ++count)
                    std::construct_at(dest + count, root->children()[count]);
            } catch (...) {
                std::destroy_n(dest, count);
                throw;
            }
            newRoot->count_ = root->count_;
        }

        const auto childIdx = (idx >> shift) & BRANCH_MASK;

        // If childIdx < root->count_, the slot was already constructed above
        // (copied from root).  We must destroy the old value before replacing it
        // to maintain correct shared_ptr reference counts and avoid UB for
        // non-trivial types.
        const auto slotNeedsDestroy = childIdx < root->count_;

        if (shift == BRANCH_BITS)
        {
            // Direct children are leaves — place the new leaf directly.
            if (slotNeedsDestroy)
                std::destroy_at(dest + childIdx);
            std::construct_at(dest + childIdx, std::move(leaf));
        }
        else
        {
            // Recurse into the existing sub-tree, or create a new path if
            // the child slot is empty.
            const auto& existing = root->children()[childIdx];
            if (existing)
            {
                auto child = pushLeaf(existing, shift - BRANCH_BITS, std::move(leaf), idx);
                if (slotNeedsDestroy)
                    std::destroy_at(dest + childIdx);
                std::construct_at(dest + childIdx, std::move(child));
            }
            else
            {
                // childIdx >= root->count_ (no existing child), so the slot
                // was *not* constructed above — safe to construct directly.
                auto child = newPath(std::move(leaf), shift - BRANCH_BITS, idx);
                std::construct_at(dest + childIdx, std::move(child));
            }
        }

        // Expand the child-count if we're inserting beyond the previous max.
        if (childIdx >= root->count_)
            newRoot->count_ = childIdx + 1;

        return newRoot;
    }

    // ───────────────────────────────────────────────────────────────────────
    // doUpdate — recursive path-copy for update()
    // ───────────────────────────────────────────────────────────────────────

    /// @brief Recursively copies the path from root to the target leaf,
    ///        replacing the leaf value at @p index with a copy of @p value.
    /// @param node   Current node to copy (leaf or internal).
    /// @param level  Current bit-depth of @p node.
    /// @param index  Global index of the element being updated.
    /// @param value  The new value to place at @p index.
    /// @return A new node representing the updated subtree root.
    static auto doUpdate(NodePtr node, size_type level, size_type index, const T& value) -> NodePtr
    {
        auto newNode = std::make_shared<Node>(node->leaf_);

        if (node->leaf_)
        {
            // Leaf node: copy all values, replacing the target slot.
            size_type count = 0;
            try {
                for (; count < node->count_; ++count)
                {
                    if (count == (index & BRANCH_MASK))
                        std::construct_at(&newNode->values()[count], value);
                    else
                        std::construct_at(&newNode->values()[count], node->values()[count]);
                }
            } catch (...) {
                std::destroy_n(newNode->values(), count);
                throw;
            }
            newNode->count_ = node->count_;
            return newNode;
        }

        // Internal node: determine which child leads to the target index.
        {
            const auto childIdx = (index >> level) & BRANCH_MASK;
            size_type count = 0;
            try {
                for (; count < node->count_; ++count)
                {
                    if (count == childIdx)
                        std::construct_at(&newNode->children()[count],
                            doUpdate(node->children()[count], level - BRANCH_BITS, index, value));
                    else
                        std::construct_at(&newNode->children()[count], node->children()[count]);
                }
            } catch (...) {
                std::destroy_n(newNode->children(), count);
                throw;
            }
            newNode->count_ = node->count_;
        }
        return newNode;
    }

    /// @brief Recursively copies the path from root to the target leaf,
    ///        replacing the leaf value at @p index by moving @p value in.
    /// @param node   Current node to copy (leaf or internal).
    /// @param level  Current bit-depth of @p node.
    /// @param index  Global index of the element being updated.
    /// @param value  The new value to move into @p index.
    /// @return A new node representing the updated subtree root.
    static auto doUpdate(NodePtr node, size_type level, size_type index, T&& value) -> NodePtr
    {
        auto newNode = std::make_shared<Node>(node->leaf_);

        if (node->leaf_)
        {
            size_type count = 0;
            try {
                for (; count < node->count_; ++count)
                {
                    if (count == (index & BRANCH_MASK))
                        std::construct_at(&newNode->values()[count], std::move(value));
                    else
                        std::construct_at(&newNode->values()[count], node->values()[count]);
                }
            } catch (...) {
                std::destroy_n(newNode->values(), count);
                throw;
            }
            newNode->count_ = node->count_;
            return newNode;
        }

        // Internal node: determine which child leads to the target index.
        {
            const auto childIdx = (index >> level) & BRANCH_MASK;
            size_type count = 0;
            try {
                for (; count < node->count_; ++count)
                {
                    if (count == childIdx)
                        std::construct_at(&newNode->children()[count],
                            doUpdate(node->children()[count], level - BRANCH_BITS, index, std::move(value)));
                    else
                        std::construct_at(&newNode->children()[count], node->children()[count]);
                }
            } catch (...) {
                std::destroy_n(newNode->children(), count);
                throw;
            }
            newNode->count_ = node->count_;
        }
        return newNode;
    }

    // ───────────────────────────────────────────────────────────────────────
    // doPopBack — recursive path-copy for pop_back()
    // ───────────────────────────────────────────────────────────────────────

    /// @brief Recursively removes the last leaf from the tree through path-copying.
    /// @param node   Current node to copy.
    /// @param level  Current bit-depth of @p node.
    /// @return A pair of {newRoot, poppedLeaf}.  newRoot is nullptr if the
    ///         entire subtree was removed; poppedLeaf is the leaf node that
    ///         was the last in the tree (to become the new tail).
    static auto doPopBack(NodePtr node, size_type level) -> std::pair<NodePtr, NodePtr>
    {
        if (node->leaf_)
        {
            // This leaf is the last leaf — detach it.
            return {nullptr, node};
        }

        // Find the last child and recurse.
        const auto lastIdx = node->count_ - 1;
        auto [newChild, popped] = doPopBack(node->children()[lastIdx], level - BRANCH_BITS);

        if (!newChild)
        {
            // The child subtree was entirely removed.
            if (lastIdx == 0)
            {
                // This was the only child — this node is also removed.
                return {nullptr, std::move(popped)};
            }

            // Create a new node without the last child.
            auto newNode = std::make_shared<Node>(false);
            {
                size_type count = 0;
                try {
                    for (; count < lastIdx; ++count)
                        std::construct_at(&newNode->children()[count], node->children()[count]);
                } catch (...) {
                    std::destroy_n(newNode->children(), count);
                    throw;
                }
                newNode->count_ = lastIdx;
            }
            return {std::move(newNode), std::move(popped)};
        }

        // Child was modified but not removed — copy all children, replacing
        // the last one with the modified child.
        auto newNode = std::make_shared<Node>(false);
        {
            size_type count = 0;
            try {
                for (; count < node->count_; ++count)
                {
                    if (count == lastIdx)
                        std::construct_at(&newNode->children()[count], std::move(newChild));
                    else
                        std::construct_at(&newNode->children()[count], node->children()[count]);
                }
            } catch (...) {
                std::destroy_n(newNode->children(), count);
                throw;
            }
            newNode->count_ = node->count_;
        }
        return {std::move(newNode), std::move(popped)};
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
// Factory functions (internal — detail namespace)
// ═══════════════════════════════════════════════════════════════════════════════

namespace detail
{

/// @brief Creates a leaf node by copying elements from an iterator range.
/// @tparam T Element type stored in the vector.
/// @tparam Iter Input iterator type (must yield values convertible to T).
/// @param first Start of the input range.
/// @param last End of the input range.
/// @return A shared pointer to the newly created leaf node.
/// @throws std::bad_alloc if allocation fails, or any exception thrown by
///         T's copy constructor.
/// @par Exception Safety
/// Strong guarantee — if construction of any element fails, all previously
/// constructed elements are destroyed and the raw node memory is freed.
template <typename T, typename Iter>
[[nodiscard]] auto makeLeaf(Iter first, Iter last) -> typename PersistentVector<T>::NodePtr
{
    using Node   = typename PersistentVector<T>::Node;
    using NodePtr = typename PersistentVector<T>::NodePtr;

    auto* const raw  = new Node(true);
    auto* const dest = raw->values();
    size_t      count = 0;

    try
    {
        for (; first != last; ++first, ++count)
        {
            std::construct_at(dest + count, *first);
        }
    }
    catch (...)
    {
        std::destroy_n(dest, count);
        delete raw;
        throw;
    }

    raw->count_ = count;
    return std::shared_ptr<const Node>(raw);
}

/// @brief Creates a leaf node from an initializer list.
/// @tparam T Element type.
/// @param init Initializer list of elements to copy.
/// @return A shared pointer to the newly created leaf node.
template <typename T>
[[nodiscard]] auto makeLeaf(std::initializer_list<T> init) -> typename PersistentVector<T>::NodePtr
{
    return makeLeaf<T>(init.begin(), init.end());
}

/// @brief Creates an internal node by copying child pointers from an iterator range.
/// @tparam T Element type stored in the vector.
/// @tparam Iter Input iterator type (must yield NodePtr values).
/// @param first Start of the input range.
/// @param last End of the input range.
/// @return A shared pointer to the newly created internal node.
/// @throws std::bad_alloc if allocation fails, or any exception thrown by
///         NodePtr's copy constructor.
/// @par Exception Safety
/// Strong guarantee — if construction of any element fails, all previously
/// constructed elements are destroyed and the raw node memory is freed.
template <typename T, typename Iter>
[[nodiscard]] auto makeInternal(Iter first, Iter last) -> typename PersistentVector<T>::NodePtr
{
    using Node   = typename PersistentVector<T>::Node;
    using NodePtr = typename PersistentVector<T>::NodePtr;

    auto* const raw  = new Node(false);
    auto* const dest = raw->children();
    size_t      count = 0;

    try
    {
        for (; first != last; ++first, ++count)
        {
            std::construct_at(dest + count, *first);
        }
    }
    catch (...)
    {
        std::destroy_n(dest, count);
        delete raw;
        throw;
    }

    raw->count_ = count;
    return std::shared_ptr<const Node>(raw);
}

}  // namespace detail

} // namespace common::data_structure::persistent
