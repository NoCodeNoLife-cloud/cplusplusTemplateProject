/**
 * @file MichaelScottQueue.hpp
 * @brief Michael-Scott lock-free MPMC queue with tagged-pointer ABA prevention
 * @details An unbounded, lock-free, multi-producer/multi-consumer FIFO queue
 *          based on the classic algorithm by Michael & Scott (PODC 1996).
 *          ABA prevention is achieved via a tagged-pointer scheme: the lower
 *          48 bits of a uintptr_t store the Node pointer (x64 user-space
 *          canonical form) and the upper 16 bits carry an ABA counter that
 *          is incremented on every successful CAS.
 *
 *          Memory management uses a concurrent free pool (Treiber stack with
 *          its own tagged-pointer ABA prevention).  Dequeued nodes are pushed
 *          to the free pool for reuse �?never freed during normal operation.
 *          Enqueue attempts to pop from the free pool first; only when the
 *          pool is empty does it allocate a new node via `new`.  This design
 *          avoids use-after-free without requiring epoch-based reclamation
 *          or hazard pointers.
 *
 *          The queue maintains a dummy head node at all times.  An empty
 *          queue is identified by head == tail && head->next == nullptr.
 *
 * @par Thread Safety
 * All public methods are thread-safe and lock-free (progress guaranteed by
 * CAS on head/tail pointers).  Multiple producers and consumers can operate
 * concurrently without blocking each other.
 *
 * @par Memory Ordering
 * - load(std::memory_order_acquire)  on head/tail loads
 * - store(std::memory_order_release)  on successful CAS writes
 * - compare_exchange_weak with acq_rel semantics on modifying CAS
 * - relaxed loads/stores on the internal atomic size counter
 *
 * @par Complexity
 * - enqueue: O(1) amortised (O(1) expected CAS retries under contention)
 * - try_dequeue: O(1) amortised
 * - size: O(1) approximate (relaxed atomic load)
 * - empty: O(1)
 *
 * Reference: Michael, M. M. & Scott, M. L. (1996). "Simple, fast, and
 *            practical non-blocking and blocking concurrent queue algorithms."
 *            Proceedings of the fifteenth annual ACM symposium on Principles
 *            of distributed computing (PODC '96).
 */

#pragma once

#include "IConcurrentQueue.hpp"

#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace cppforge::data_structure::concurrent
{
    /// @brief Lock-free MPMC queue with tagged-pointer ABA protection.
    ///
    /// Implements the classic Michael-Scott unbounded linked-list queue.
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
    /// Dequeued nodes are pushed to a lock-free free pool (Treiber stack with
    /// tagged-pointer ABA prevention).  Enqueue operations reuse nodes from
    /// the pool.  The free pool eliminates most dynamic allocations during
    /// steady-state operation.  All nodes are freed in the destructor.
    ///
    /// @par Usage Example
    /// @code
    /// MichaelScottQueue<int> q;
    /// q.enqueue(1);
    /// q.enqueue(2);
    /// if (auto val = q.try_dequeue()) {
    ///     // *val == 1
    /// }
    /// @endcode
    template <std::movable T>
    class MichaelScottQueue final : public IConcurrentQueue<T>
    {
    public:
        // ── Construction / destruction ─────────────────────────────────

        /// @brief Constructs an empty queue with an initial dummy head node.
        MichaelScottQueue();

        /// @brief Destructor �?drains the queue and free pool, deletes all
        ///        remaining nodes.
        ~MichaelScottQueue() noexcept override;

        // Disable copy (non-copyable due to atomic members and the node pool).
        MichaelScottQueue(const MichaelScottQueue&) = delete;
        auto operator=(const MichaelScottQueue&) -> MichaelScottQueue& = delete;

        // ── Move ───────────────────────────────────────────────────────

        /// @brief Move constructor �?transfers all nodes from @p other.
        /// @param other The queue to move from.  Left in a valid empty state.
        MichaelScottQueue(MichaelScottQueue&& other) noexcept;

        /// @brief Move assignment �?releases current resources, then
        ///        transfers ownership of @p other's nodes.
        /// @param other The queue to move from.
        /// @return Reference to this queue.
        auto operator=(MichaelScottQueue&& other) noexcept -> MichaelScottQueue&;

        // ── IConcurrentQueue interface ─────────────────────────────────

        /// @brief Enqueues a value at the tail.
        ///
        /// Allocates or recycles a node, then links it into the queue using
        /// the standard MS enqueue protocol with CAS on tail->next and tail.
        /// @param value The value to enqueue.
        void enqueue(T value) override;

        /// @brief Attempts to dequeue a value from the head.
        ///
        /// Uses the standard MS dequeue protocol with CAS on head and
        /// helping for lagging tail.  The removed old-dummy node is pushed
        /// to the free pool for reuse.
        /// @return The dequeued value if the queue was non-empty, or
        ///         std::nullopt if empty.
        [[nodiscard]] auto try_dequeue() -> std::optional<T> override;

        /// @brief Returns the approximate number of elements in the queue.
        ///
        /// The count is maintained by an atomic counter that is incremented
        /// on successful enqueue and decremented on successful dequeue.
        /// Under concurrent operations the returned value may momentarily
        /// lag behind the true count.
        /// @return Approximate element count.
        [[nodiscard]] auto size() const -> std::size_t override;

        /// @brief Checks whether the queue is empty.
        /// @return true if size() == 0.
        [[nodiscard]] auto empty() const -> bool override;

    private:
        // ── Tagged-pointer constants ───────────────────────────────────

        // On x64, user-space pointers occupy only the lower 48 bits
        // (canonical form).  The upper 16 bits are usable as an ABA counter.
        static_assert(sizeof(uintptr_t) == 8,
                      "MichaelScottQueue requires 64-bit platform");
        static constexpr uintptr_t kPtrMask  = 0x0000FFFFFFFFFFFFULL;
        static constexpr uintptr_t kTagShift = 48;

        // ── Internal types ─────────────────────────────────────────────

        /// @brief Queue / free-pool node with in-place value storage.
        struct Node
        {
            /// @brief The element value.  Empty (std::nullopt) for dummy nodes
            ///        and recycled nodes; engaged for nodes that hold a real
            ///        value.
            std::optional<T> value;

            /// @brief Tagged pointer to the next node (queue) or the next
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

        /// @brief Destroys all queue nodes and free-pool nodes.
        void destroy_all_nodes() noexcept;

        // ── Member data ────────────────────────────────────────────────

        // The queue head pointer (always points to the dummy node).
        std::atomic<uintptr_t> head_;

        // The queue tail pointer (points to the last node, initially dummy).
        std::atomic<uintptr_t> tail_;

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
    MichaelScottQueue<T>::MichaelScottQueue()
    {
        auto* dummy    = new Node();
        auto  packed   = pack_ptr(dummy, 0);
        head_.store(packed, std::memory_order_relaxed);
        tail_.store(packed, std::memory_order_relaxed);
    }

    template <std::movable T>
    MichaelScottQueue<T>::~MichaelScottQueue() noexcept
    {
        destroy_all_nodes();
    }

    // ── Move ─────────────────────────────────────────────────────────────

    template <std::movable T>
    MichaelScottQueue<T>::MichaelScottQueue(MichaelScottQueue&& other) noexcept
        : head_(other.head_.load(std::memory_order_relaxed))
        , tail_(other.tail_.load(std::memory_order_relaxed))
        , free_head_(other.free_head_.load(std::memory_order_relaxed))
        , size_(other.size_.load(std::memory_order_relaxed))
    {
        // Reset other to a valid empty state with a fresh dummy node.
        auto* dummy   = new Node();
        auto  packed  = pack_ptr(dummy, 0);
        other.head_.store(packed, std::memory_order_relaxed);
        other.tail_.store(packed, std::memory_order_relaxed);
        other.free_head_.store(0, std::memory_order_relaxed);
        other.size_.store(0, std::memory_order_relaxed);
    }

    template <std::movable T>
    auto MichaelScottQueue<T>::operator=(MichaelScottQueue&& other) noexcept
        -> MichaelScottQueue&
    {
        if (this != &other)
        {
            // Release our own resources.
            destroy_all_nodes();

            // Steal other's data.
            head_.store(other.head_.load(std::memory_order_relaxed),
                        std::memory_order_relaxed);
            tail_.store(other.tail_.load(std::memory_order_relaxed),
                        std::memory_order_relaxed);
            free_head_.store(other.free_head_.load(std::memory_order_relaxed),
                             std::memory_order_relaxed);
            size_.store(other.size_.load(std::memory_order_relaxed),
                        std::memory_order_relaxed);

            // Reset other to a valid empty state.
            auto* dummy  = new Node();
            auto  packed = pack_ptr(dummy, 0);
            other.head_.store(packed, std::memory_order_relaxed);
            other.tail_.store(packed, std::memory_order_relaxed);
            other.free_head_.store(0, std::memory_order_relaxed);
            other.size_.store(0, std::memory_order_relaxed);
        }
        return *this;
    }

    // ── Free-pool implementation (Treiber stack) ─────────────────────────

    template <std::movable T>
    auto MichaelScottQueue<T>::pop_free_node() -> Node*
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
            // Safety: only we can pop this node (CAS ensures exclusivity),
            // so the relaxed load is sufficient.
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
                // ready for queue insertion.
                head_node->next.store(0, std::memory_order_relaxed);
                // The optional is still engaged (moved-from) �?enqueue will
                // replace it via emplace() when the node is reused.
                return head_node;
            }
            // CAS failed �?retry with the updated old_head.
        }
    }

    template <std::movable T>
    void MichaelScottQueue<T>::push_free_node(Node* node) noexcept
    {
        // Link the node into the free list.  The optional value is not reset
        // here �?it remains engaged (moved-from) to avoid races where another
        // thread holds a pointer to this node and may dereference the optional.
        // enqueue() will replace the value via emplace() when the node is
        // reused.

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
            // CAS failed �?reload old_head and retry.
        }
    }

    // ── Enqueue ──────────────────────────────────────────────────────────

    template <std::movable T>
    void MichaelScottQueue<T>::enqueue(T value)
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

        // Michael-Scott enqueue protocol.
        while (true)
        {
            uintptr_t tail_tagged = tail_.load(std::memory_order_acquire);
            Node*     tail_node   = unpack_ptr(tail_tagged);
            uintptr_t next_tagged = tail_node->next.load(std::memory_order_acquire);
            Node*     next_node   = unpack_ptr(next_tagged);

            // Consistency check: if tail hasn't changed, proceed.
            if (tail_tagged == tail_.load(std::memory_order_relaxed))
            {
                if (next_node == nullptr)
                {
                    // Tail is at the last node �?try to append.
                    // Tag on `next` is ignored (always 0 when null).
                    uintptr_t expected = 0;
                    uintptr_t desired  = pack_ptr(node, 0);
                    if (tail_node->next.compare_exchange_weak(expected, desired,
                                                              std::memory_order_release,
                                                              std::memory_order_relaxed))
                    {
                        // Successfully linked the new node.
                        size_.fetch_add(1, std::memory_order_relaxed);

                        // Try to advance tail (may fail �?another thread
                        // may help or the next enqueue will fix it).
                        auto new_tail = pack_ptr(node,
                                                 static_cast<uint16_t>(unpack_tag(tail_tagged) + 1));
                        tail_.compare_exchange_weak(tail_tagged, new_tail,
                                                    std::memory_order_release,
                                                    std::memory_order_relaxed);
                        return;
                    }
                }
                else
                {
                    // Tail is lagging �?help advance it.
                    auto new_tail = pack_ptr(next_node,
                                             static_cast<uint16_t>(unpack_tag(tail_tagged) + 1));
                    tail_.compare_exchange_weak(tail_tagged, new_tail,
                                                std::memory_order_release,
                                                std::memory_order_relaxed);
                }
            }
        }
    }

    // ── Dequeue ──────────────────────────────────────────────────────────

    template <std::movable T>
    auto MichaelScottQueue<T>::try_dequeue() -> std::optional<T>
    {
        while (true)
        {
            uintptr_t head_tagged = head_.load(std::memory_order_acquire);
            Node*     head_node   = unpack_ptr(head_tagged);
            uintptr_t tail_tagged = tail_.load(std::memory_order_acquire);
            uintptr_t next_tagged = head_node->next.load(std::memory_order_acquire);
            Node*     next_node   = unpack_ptr(next_tagged);

            // Consistency check.
            if (head_tagged == head_.load(std::memory_order_relaxed))
            {
                if (head_node == unpack_ptr(tail_tagged))
                {
                    // Queue may be empty.
                    if (next_node == nullptr)
                    {
                        return std::nullopt;  // Empty.
                    }

                    // Tail is lagging �?help advance it.
                    auto new_tail = pack_ptr(next_node,
                                             static_cast<uint16_t>(unpack_tag(tail_tagged) + 1));
                    tail_.compare_exchange_weak(tail_tagged, new_tail,
                                                std::memory_order_release,
                                                std::memory_order_relaxed);
                }
                else
                {
                    // Advance head to next_node (which becomes the new dummy).
                    auto new_head = pack_ptr(next_node,
                                             static_cast<uint16_t>(unpack_tag(head_tagged) + 1));

                    if constexpr (std::is_copy_constructible_v<T>)
                    {
                        // For copyable types: copy the value before CAS.
                        // If CAS fails, the copy is discarded and the node
                        // retains its value for the next dequeue attempt.

                        // Defensive check: if the value was concurrently
                        // emplaced (emplace temporarily empties the optional),
                        // retry �?head must have changed since the node was
                        // recycled.
                        if (!next_node->value.has_value())
                        {
                            continue;
                        }

                        T extracted_value = *next_node->value;

                        if (head_.compare_exchange_weak(head_tagged, new_head,
                                                        std::memory_order_release,
                                                        std::memory_order_relaxed))
                        {
                            // CAS succeeded �?we own the node exclusively.
                            // Recycle the old dummy in the free pool.
                            push_free_node(head_node);

                            size_.fetch_sub(1, std::memory_order_relaxed);
                            return extracted_value;
                        }
                        // CAS failed �?discard the copy, retry the loop.
                    }
                    else
                    {
                        // For move-only types: CAS first, then move the value
                        // (the only safe sequence for non-copyable types).
                        if (head_.compare_exchange_weak(head_tagged, new_head,
                                                        std::memory_order_release,
                                                        std::memory_order_relaxed))
                        {
                            // CAS succeeded �?we own the node exclusively.
                            // (Defensive has_value check for robustness.)
                            if (!next_node->value.has_value())
                            {
                                continue;
                            }

                            T extracted_value = std::move(*next_node->value);

                            // Recycle the old dummy in the free pool.
                            push_free_node(head_node);

                            size_.fetch_sub(1, std::memory_order_relaxed);
                            return extracted_value;
                        }
                        // CAS failed �?retry the loop.
                    }
                }
            }
        }
    }

    // ── Size / empty ─────────────────────────────────────────────────────

    template <std::movable T>
    auto MichaelScottQueue<T>::size() const -> std::size_t
    {
        return size_.load(std::memory_order_relaxed);
    }

    template <std::movable T>
    auto MichaelScottQueue<T>::empty() const -> bool
    {
        return size_.load(std::memory_order_relaxed) == 0;
    }

    // ── Resource cleanup ─────────────────────────────────────────────────

    template <std::movable T>
    void MichaelScottQueue<T>::destroy_all_nodes() noexcept
    {
        // Walk the queue from head, following next until null.
        // We unpack first and check for nullptr because the sentinel may
        // be a tagged null pointer (pack_ptr(nullptr, tag)) which is non-zero.
        {
            uintptr_t cur_tagged = head_.load(std::memory_order_relaxed);
            while (true)
            {
                Node* cur = unpack_ptr(cur_tagged);
                if (cur == nullptr)
                {
                    break;
                }
                // Save next before destroying (next field may be modified
                // if this node was also in the free pool �?but since we
                // are single-threaded here, this is safe).
                uintptr_t next_tagged = cur->next.load(std::memory_order_relaxed);
                cur->value.reset();
                delete cur;
                cur_tagged = next_tagged;
            }
        }

        // Walk the free pool, deleting all remaining nodes.
        // Use the same nullptr-check pattern as the queue walk above,
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

} // namespace cppforge::data_structure::concurrent
