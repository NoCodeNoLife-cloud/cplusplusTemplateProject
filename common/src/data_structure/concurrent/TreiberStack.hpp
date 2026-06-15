/**
 * @file TreiberStack.hpp
 * @brief Lock-free LIFO stack (Treiber stack) with tagged-pointer ABA prevention
 * @details An unbounded, lock-free, multi-producer/multi-consumer LIFO stack
 *          based on the classic algorithm by R. K. Treiber (IBM 1986).
 *          ABA prevention is achieved via a tagged-pointer scheme: the lower
 *          48 bits of a uintptr_t store the Node pointer (x64 user-space
 *          canonical form) and the upper 16 bits carry an ABA counter that
 *          is incremented on every successful CAS.
 *
 *          Memory management uses a concurrent free pool (itself a Treiber
 *          stack with tagged-pointer ABA prevention).  Popped nodes are pushed
 *          to the free pool for reuse — never freed during normal operation.
 *          Push attempts to pop from the free pool first; only when the pool
 *          is empty does it allocate a new node via `new`.  This design avoids
 *          use-after-free without requiring epoch-based reclamation or hazard
 *          pointers.
 *
 * @par Thread Safety
 * All public methods are thread-safe and lock-free (progress guaranteed by
 * CAS on the top pointer).  Multiple producers and consumers can operate
 * concurrently without blocking each other.
 *
 * @par Memory Ordering
 * - load(std::memory_order_acquire)  on top_ loads
 * - store(std::memory_order_release) on successful CAS writes
 * - compare_exchange_weak with acq_rel semantics on modifying CAS
 * - relaxed loads/stores on the internal atomic size counter
 *
 * @par Complexity
 * - push: O(1) expected CAS retries under contention
 * - try_pop: O(1) expected CAS retries under contention
 * - size: O(1) approximate (relaxed atomic load)
 * - empty: O(1)
 *
 * Reference: Treiber, R. K. (1986). "Systems Programming: Coping with
 *            Parallelism." IBM Thomas J. Watson Research Center,
 *            Technical Report RJ 5118.
 */

#pragma once

#include "IConcurrentStack.hpp"

#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace common::data_structure::concurrent
{
    /// @brief Lock-free LIFO stack with tagged-pointer ABA protection.
    ///
    /// Implements the classic Treiber unbounded linked-list stack.
    /// All operations are lock-free (no mutex, no blocking).  Nodes are
    /// recycled via an internal lock-free free pool to eliminate dynamic
    /// allocation on every operation.
    ///
    /// @tparam T Element type.  Must satisfy std::movable.
    ///
    /// @par Thread Safety
    /// All public methods are thread-safe and lock-free.
    ///
    /// @par Memory Reclamation
    /// Popped nodes are pushed to a lock-free free pool (Treiber stack with
    /// tagged-pointer ABA prevention).  Push operations reuse nodes from
    /// the pool.  The free pool eliminates most dynamic allocations during
    /// steady-state operation.  All nodes are freed in the destructor.
    ///
    /// @par Usage Example
    /// @code
    /// TreiberStack<int> s;
    /// s.push(1);
    /// s.push(2);
    /// if (auto val = s.try_pop()) {
    ///     // *val == 2
    /// }
    /// @endcode
    template <std::movable T>
    class TreiberStack final : public IConcurrentStack<T>
    {
    public:
        // ── Construction / destruction ─────────────────────────────────

        /// @brief Constructs an empty stack.
        TreiberStack() = default;

        /// @brief Destructor — drains the stack and free pool, deletes all
        ///        remaining nodes.
        ~TreiberStack() noexcept override;

        // Disable copy (non-copyable due to atomic members and the node pool).
        TreiberStack(const TreiberStack&) = delete;
        auto operator=(const TreiberStack&) -> TreiberStack& = delete;

        // ── Move ───────────────────────────────────────────────────────

        /// @brief Move constructor — transfers all nodes from @p other.
        /// @param other The stack to move from.  Left in a valid empty state.
        TreiberStack(TreiberStack&& other) noexcept;

        /// @brief Move assignment — releases current resources, then
        ///        transfers ownership of @p other's nodes.
        /// @param other The stack to move from.
        /// @return Reference to this stack.
        auto operator=(TreiberStack&& other) noexcept -> TreiberStack&;

        // ── IConcurrentStack interface ─────────────────────────────────

        /// @brief Pushes a value onto the top of the stack.
        ///
        /// Allocates or recycles a node, then links it into the stack using
        /// CAS on the top pointer.
        /// @param value The value to push.
        /// @throws std::exception if the value constructor throws.
        void push(T value) override;

        /// @brief Attempts to pop a value from the top of the stack.
        ///
        /// Uses CAS on the top pointer to atomically remove the head node.
        /// The removed node is pushed to the free pool for reuse.
        /// @return The popped value if the stack was non-empty, or
        ///         std::nullopt if empty.
        [[nodiscard]] auto try_pop() -> std::optional<T> override;

        /// @brief Returns the approximate number of elements in the stack.
        ///
        /// The count is maintained by an atomic counter that is incremented
        /// on successful push and decremented on successful try_pop.
        /// Under concurrent operations the returned value may momentarily
        /// lag behind the true count.
        /// @return Approximate element count.
        [[nodiscard]] auto size() const -> std::size_t override;

        /// @brief Checks whether the stack is empty.
        /// @return true if size() == 0.
        [[nodiscard]] auto empty() const -> bool override;

    private:
        // ── Tagged-pointer constants ───────────────────────────────────

        // On x64, user-space pointers occupy only the lower 48 bits
        // (canonical form).  The upper 16 bits are usable as an ABA counter.
        static_assert(sizeof(uintptr_t) == 8,
                      "TreiberStack requires 64-bit platform");
        static constexpr uintptr_t kPtrMask  = 0x0000FFFFFFFFFFFFULL;
        static constexpr uintptr_t kTagShift = 48;

        // ── Internal types ─────────────────────────────────────────────

        /// @brief Stack / free-pool node with in-place value storage.
        struct Node
        {
            /// @brief The element value.  Empty (std::nullopt) for recycled
            ///        nodes; engaged for nodes that hold a real value.
            std::optional<T> value;

            /// @brief Tagged pointer to the next node (stack) or the next
            ///        free node (free pool).
            std::atomic<uintptr_t> next{0};
        };

        // ── Packed-pointer helpers ─────────────────────────────────────

        /// @brief Packs a Node pointer and a 16-bit tag into a uintptr_t.
        [[nodiscard]] static auto pack_ptr(Node* ptr, uint16_t tag) -> uintptr_t
        {
            return reinterpret_cast<uintptr_t>(ptr)
                 | (static_cast<uintptr_t>(tag) << kTagShift);
        }

        /// @brief Extracts the Node pointer from a packed uintptr_t.
        [[nodiscard]] static auto unpack_ptr(uintptr_t tagged) -> Node*
        {
            return reinterpret_cast<Node*>(tagged & kPtrMask);
        }

        /// @brief Extracts the 16-bit tag from a packed uintptr_t.
        [[nodiscard]] static auto unpack_tag(uintptr_t tagged) -> uint16_t
        {
            return static_cast<uint16_t>(tagged >> kTagShift);
        }

        // ── Free-pool operations ───────────────────────────────────────

        /// @brief Pops a node from the lock-free free pool.
        /// @return A recycled node whose value has been destroyed and whose
        ///         `next` field has been reset to 0, or nullptr if the pool
        ///         is empty.
        [[nodiscard]] auto pop_free_node() -> Node*;

        /// @brief Pushes a node onto the lock-free free pool.
        /// @param node The node to recycle.  Its `next` field is repurposed
        ///             for the free list.  The stored value is NOT reset
        ///             (the optional stays engaged to avoid concurrent
        ///             dereference races).
        void push_free_node(Node* node) noexcept;

        // ── Resource cleanup ──────────────────────────────────────────

        /// @brief Destroys all stack nodes and free-pool nodes.
        void destroy_all_nodes() noexcept;

        // ── Member data ────────────────────────────────────────────────

        // The stack top pointer (nullptr when empty).
        std::atomic<uintptr_t> top_{0};

        // The free-pool (Treiber stack) head pointer.
        std::atomic<uintptr_t> free_head_{0};

        // Approximate element count.
        std::atomic<std::size_t> size_{0};
    };

    // ══════════════════════════════════════════════════════════════════════
    //  Implementation
    // ══════════════════════════════════════════════════════════════════════

    // ── Construction / destruction ───────────────────────────────────────

    template <std::movable T>
    TreiberStack<T>::~TreiberStack() noexcept
    {
        destroy_all_nodes();
    }

    // ── Move ─────────────────────────────────────────────────────────────

    template <std::movable T>
    TreiberStack<T>::TreiberStack(TreiberStack&& other) noexcept
        : top_(other.top_.load(std::memory_order_relaxed))
        , free_head_(other.free_head_.load(std::memory_order_relaxed))
        , size_(other.size_.load(std::memory_order_relaxed))
    {
        // Reset other to a valid empty state.
        other.top_.store(0, std::memory_order_relaxed);
        other.free_head_.store(0, std::memory_order_relaxed);
        other.size_.store(0, std::memory_order_relaxed);
    }

    template <std::movable T>
    auto TreiberStack<T>::operator=(TreiberStack&& other) noexcept
        -> TreiberStack&
    {
        if (this != &other)
        {
            // Release our own resources.
            destroy_all_nodes();

            // Steal other's data.
            top_.store(other.top_.load(std::memory_order_relaxed),
                       std::memory_order_relaxed);
            free_head_.store(other.free_head_.load(std::memory_order_relaxed),
                             std::memory_order_relaxed);
            size_.store(other.size_.load(std::memory_order_relaxed),
                        std::memory_order_relaxed);

            // Reset other to a valid empty state.
            other.top_.store(0, std::memory_order_relaxed);
            other.free_head_.store(0, std::memory_order_relaxed);
            other.size_.store(0, std::memory_order_relaxed);
        }
        return *this;
    }

    // ── Free-pool implementation (Treiber stack) ─────────────────────────

    template <std::movable T>
    auto TreiberStack<T>::pop_free_node() -> Node*
    {
        uintptr_t old_head = free_head_.load(std::memory_order_acquire);
        while (true)
        {
            Node* head_node = unpack_ptr(old_head);
            if (head_node == nullptr)
            {
                return nullptr;  // Pool empty.
            }

            // Read the next pointer from the head node.
            // Safety: A CAS failure discards this read and retries, so a stale
            // value is harmless.  If the CAS succeeds, we own this node
            // exclusively and no other thread can modify its `next` pointer,
            // making the relaxed load sufficient.
            uintptr_t next = head_node->next.load(std::memory_order_relaxed);

            // CAS free_head_ from old_head to next, incrementing the tag.
            uintptr_t desired = pack_ptr(unpack_ptr(next),
                                         static_cast<uint16_t>(unpack_tag(old_head) + 1));
            if (free_head_.compare_exchange_weak(old_head, desired,
                                                 std::memory_order_acq_rel,
                                                 std::memory_order_relaxed))
            {
                // Successfully popped the node.  Reset its next field
                // (which was the free-list chain pointer) to 0 so it is
                // ready for stack insertion.
                head_node->next.store(0, std::memory_order_relaxed);
                // The optional is still engaged (moved-from) — push will
                // replace it via emplace() when the node is reused.
                return head_node;
            }
            // CAS failed — retry with the updated old_head.
        }
    }

    template <std::movable T>
    void TreiberStack<T>::push_free_node(Node* node) noexcept
    {
        // Link the node into the free list.  The optional value is not reset
        // here — it remains engaged (moved-from) to avoid races where another
        // thread holds a pointer to this node and may dereference the optional.
        // push() will replace the value via emplace() when the node is reused.

        uintptr_t old_head = free_head_.load(std::memory_order_relaxed);
        while (true)
        {
            // Link the node into the free list.
            node->next.store(old_head, std::memory_order_relaxed);

            uintptr_t desired = pack_ptr(node,
                                         static_cast<uint16_t>(unpack_tag(old_head) + 1));
            if (free_head_.compare_exchange_weak(old_head, desired,
                                                 std::memory_order_release,
                                                 std::memory_order_relaxed))
            {
                return;  // Successfully pushed.
            }
            // CAS failed — reload old_head and retry.
        }
    }

    // ── Push ─────────────────────────────────────────────────────────────

    template <std::movable T>
    void TreiberStack<T>::push(T value)
    {
        // Obtain a node from the free pool or allocate a fresh one.
        Node* node = pop_free_node();
        if (node == nullptr)
        {
            node = new Node();
        }

        // Construct the value in the node.
        // If the constructor throws, return the node to the free pool
        // (or add a newly allocated node to the pool for later reuse).
        try
        {
            node->value.emplace(std::move(value));
        }
        catch (...)
        {
            // push_free_node calls node->value.reset() which is a no-op when
            // the optional is empty (the constructor never completed).
            push_free_node(node);
            throw;
        }

        // Treiber push protocol.
        uintptr_t old_top = top_.load(std::memory_order_acquire);
        while (true)
        {
            // Link the new node to the current top.
            // Tag on `next` is ignored (always 0 when used as a raw pointer).
            node->next.store(old_top, std::memory_order_relaxed);

            uintptr_t desired = pack_ptr(
                node,
                static_cast<uint16_t>(unpack_tag(old_top) + 1));

            if (top_.compare_exchange_weak(old_top, desired,
                                           std::memory_order_release,
                                           std::memory_order_relaxed))
            {
                // Successfully linked the new node.
                size_.fetch_add(1, std::memory_order_relaxed);
                return;
            }
            // CAS failed — reload old_top and retry.
        }
    }

    // ── Pop ──────────────────────────────────────────────────────────────

    template <std::movable T>
    auto TreiberStack<T>::try_pop() -> std::optional<T>
    {
        uintptr_t old_top = top_.load(std::memory_order_acquire);
        while (true)
        {
            Node* top_node = unpack_ptr(old_top);
            if (top_node == nullptr)
            {
                return std::nullopt;  // Stack empty.
            }

            // Read the next pointer from the top node.
            uintptr_t next = top_node->next.load(std::memory_order_relaxed);

            uintptr_t desired = pack_ptr(
                unpack_ptr(next),
                static_cast<uint16_t>(unpack_tag(old_top) + 1));

            if constexpr (std::is_copy_constructible_v<T>)
            {
                // For copyable types: copy the value before CAS.
                // If CAS fails, the copy is discarded and the node retains
                // its value for the next pop attempt.

                // Defensive check: if the value was concurrently emplaced
                // (emplace temporarily empties the optional), retry — top_
                // must have changed since the node was recycled.
                if (!top_node->value.has_value())
                {
                    // Reload top_ and retry.
                    old_top = top_.load(std::memory_order_acquire);
                    continue;
                }

                T extracted_value = *top_node->value;

                if (top_.compare_exchange_weak(old_top, desired,
                                               std::memory_order_release,
                                               std::memory_order_relaxed))
                {
                    // CAS succeeded — we own the node exclusively.
                    // Recycle the node in the free pool.
                    push_free_node(top_node);

                    size_.fetch_sub(1, std::memory_order_relaxed);
                    return extracted_value;
                }
                // CAS failed — discard the copy, retry the loop.
            }
            else
            {
                // For move-only types: CAS first, then move the value
                // (the only safe sequence for non-copyable types).
                if (top_.compare_exchange_weak(old_top, desired,
                                               std::memory_order_release,
                                               std::memory_order_relaxed))
                {
                    // CAS succeeded — we own the node exclusively.
                    // (Defensive has_value check for robustness.)
                    if (!top_node->value.has_value())
                    {
                        // This should not happen on a correctly-operated
                        // stack, but we defensively push the node back
                        // and retry.
                        push_free_node(top_node);
                        old_top = top_.load(std::memory_order_acquire);
                        continue;
                    }

                    T extracted_value = std::move(*top_node->value);

                    // Recycle the node in the free pool.
                    push_free_node(top_node);

                    size_.fetch_sub(1, std::memory_order_relaxed);
                    return extracted_value;
                }
                // CAS failed — retry the loop.
            }
        }
    }

    // ── Size / empty ─────────────────────────────────────────────────────

    template <std::movable T>
    auto TreiberStack<T>::size() const -> std::size_t
    {
        return size_.load(std::memory_order_relaxed);
    }

    template <std::movable T>
    auto TreiberStack<T>::empty() const -> bool
    {
        return size_.load(std::memory_order_relaxed) == 0;
    }

    // ── Resource cleanup ─────────────────────────────────────────────────

    template <std::movable T>
    void TreiberStack<T>::destroy_all_nodes() noexcept
    {
        // Walk the stack from top, following next until null.
        // We unpack first and check for nullptr because the sentinel may
        // be a tagged null pointer (pack_ptr(nullptr, tag)) which is non-zero.
        {
            uintptr_t cur_tagged = top_.load(std::memory_order_relaxed);
            while (true)
            {
                Node* cur = unpack_ptr(cur_tagged);
                if (cur == nullptr)
                {
                    break;
                }
                // Save next before destroying (next field may be modified
                // if this node was also in the free pool — but since we
                // are single-threaded here, this is safe).
                uintptr_t next_tagged = cur->next.load(std::memory_order_relaxed);
                cur->value.reset();
                delete cur;
                cur_tagged = next_tagged;
            }
        }

        // Walk the free pool, deleting all remaining nodes.
        // Use the same nullptr-check pattern as the stack walk above,
        // because the free pool's chain-terminating sentinel may be a
        // tagged null pointer (pack_ptr(nullptr, tag)), which is non-zero.
        {
            uintptr_t cur_tagged = free_head_.load(std::memory_order_relaxed);
            while (true)
            {
                Node* cur = unpack_ptr(cur_tagged);
                if (cur == nullptr)
                {
                    break;
                }
                uintptr_t next_tagged = cur->next.load(std::memory_order_relaxed);
                // Reset the (moved-from) value before deletion.
                // push_free_node no longer resets the value (see the
                // free-pool design rationale), so we must destroy it here.
                cur->value.reset();
                delete cur;
                cur_tagged = next_tagged;
            }
        }
    }

} // namespace common::data_structure::concurrent
