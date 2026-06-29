/**
 * @file SeqLock.hpp
 * @brief Sequence lock for concurrent read-write synchronisation
 * @details A seqlock is a lock-free synchronisation primitive optimised for
 *          read-mostly workloads.  Readers spin-wait on an atomic sequence
 *          counter, copying the protected data without acquiring any OS lock.
 *          Writers increment the counter (making it odd), modify the data,
 *          then increment again (restoring it to an even value).  A reader
 *          succeeds only if both reads of the counter return the same even
 *          value, guaranteeing a consistent snapshot.
 *
 *          This implementation assumes a single writer.  For multi-writer
 *          scenarios, wrap with MutexSeqLock<T> which adds a std::shared_mutex
 *          for exclusive write access.
 *
 * @par Thread Safety
 * All public methods are thread-safe.  Multiple readers can execute
 * concurrently with each other and with a single writer.  Readers are
 * wait-free in the absence of contention; they spin only when a writer
 * is active.
 *
 * @par Memory Ordering
 * - Reader: seq_.load(std::memory_order_acquire) (twice) with compiler
 *           barriers around the data copy
 * - Writer: seq_.store(std::memory_order_release) (twice) with compiler
 *           barriers around the data write
 * - compiler_barrier: std::atomic_signal_fence(std::memory_order_seq_cst)
 *                     prevents compiler reordering without emitting CPU
 *                     fence instructions
 *
 * @par Complexity
 * - load: O(1) expected retries under contention; wait-free in the absence
 *         of a concurrent writer
 * - store: O(1) uncontended
 * - try_load: O(1) single attempt, non-blocking
 *
 * @par Usage Example
 * @code
 * SeqLock<std::pair<int, double>> lock;
 * // Writer
 * lock.store({42, 3.14});
 * // Reader
 * auto snap = lock.load();
 * // RAII reader
 * if (auto guard = lock.protect(); guard->first == 42) { ... }
 * // RAII writer
 * {
 *     auto wg = lock.lock();
 *     wg->first = 100;
 *     wg->second = 2.71;
 * }
 * @endcode
 *
 * Reference: Corbet, J. (2007). "Sequence locks."
 *            https://lwn.net/Articles/219303/
 */

#pragma once

#include <atomic>
#include <concepts>
#include <cstddef>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <type_traits>
#include <utility>

// Platform-specific intrinsics for spin-wait pause instruction
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__x86_64__) || defined(__i386__) || defined(__i386)
#include <xmmintrin.h>
#endif

namespace cppforge::data_structure::concurrent
{

// ── SeqLockable concept ──────────────────────────────────────────────────

/// @brief Requirements for types that can be protected by a SeqLock.
///
/// The type must be trivially copyable so that the raw-bytes copy in the
/// reader hot path is well-defined, and copy-constructible so that the
/// RAII ReadGuard can store a snapshot.
///
/// @tparam T Candidate type to check.
template <typename T>
concept SeqLockable = std::is_trivially_copyable_v<T> && std::is_copy_constructible_v<T>;

// ── SeqLock ──────────────────────────────────────────────────────────────

/// @brief Sequence lock (seqlock) providing efficient concurrent read/write
///        access to a single copyable value.
///
/// Readers spin on an atomic sequence counter; they never acquire an OS lock.
/// Writers increment the counter, modify the data, and increment again.
/// A reader succeeds only if both reads of the counter return the same even
/// value, guaranteeing a consistent snapshot.
///
/// @par Thread Safety
/// Multiple readers and a single writer may operate concurrently.  This
/// implementation does **not** guard against multiple concurrent writers;
/// use MutexSeqLock<T> for multi-writer scenarios.
///
/// @tparam T The protected data type.  Must satisfy SeqLockable.
///
/// @par Cache-Line Isolation
/// The sequence counter and the protected data reside in separate cache lines
/// (64-byte aligned and padded) to eliminate false sharing between readers
/// (who write only to seq_ via atomic load) and the writer (who writes both
/// seq_ and data_).
///
/// @par Usage Example
/// @code
/// SeqLock<double> temp;
/// // Writer thread
/// temp.store(36.5);
/// // Reader threads
/// double t = temp.load();
/// auto guard = temp.protect();
/// // guard is a consistent snapshot
/// @endcode
template <SeqLockable T>
class alignas(64) SeqLock final
{
public:
    // ── ReadGuard ────────────────────────────────────────────────────────

    /// @brief RAII guard that captures a consistent snapshot of the
    ///        protected data.
    ///
    /// The snapshot is taken at construction time via a full seqlock read
    /// (spin-waiting until consistency is achieved).  The snapshot remains
    /// valid for the lifetime of the guard regardless of concurrent writes.
    ///
    /// @par Usage Example
    /// @code
    /// SeqLock<Config> cfg;
    /// // ...
    /// if (auto g = cfg.protect(); g->timeout > 0) {
    ///     process(g->timeout);
    /// }
    /// @endcode
    class ReadGuard final
    {
    public:
        /// @brief Takes a consistent snapshot of @p lock's protected data.
        /// @param lock The seqlock to read from.
        explicit ReadGuard(const SeqLock& lock)
            : lock_(lock)
        {
            snapshot_ = lock_.load();
        }

        ReadGuard(const ReadGuard&) = delete;
        auto operator=(const ReadGuard&) -> ReadGuard& = delete;
        ReadGuard(ReadGuard&&) = delete;
        auto operator=(ReadGuard&&) -> ReadGuard& = delete;

        /// @brief Returns a const reference to the captured snapshot.
        /// @return Const reference to the snapshot.
        [[nodiscard]] auto operator*() const -> const T& { return snapshot_; }

        /// @brief Provides pointer access to the captured snapshot.
        /// @return Const pointer to the snapshot.
        [[nodiscard]] auto operator->() const -> const T* { return &snapshot_; }

    private:
        const SeqLock& lock_;
        T               snapshot_;
    };

    // ── WriteGuard ───────────────────────────────────────────────────────

    /// @brief RAII guard that provides exclusive write access to the
    ///        protected data.
    ///
    /// Acquires the write lock (increments the sequence counter to odd) at
    /// construction and releases it (increments to next even) at destruction.
    ///
    /// @par Usage Example
    /// @code
    /// SeqLock<Metrics> m;
    /// {
    ///     auto g = m.lock();
    ///     g->count += 1;
    ///     g->total += val;
    /// }
    /// @endcode
    class WriteGuard final
    {
    public:
        /// @brief Acquires the write lock on @p lock.
        /// @param lock The seqlock to write to.
        explicit WriteGuard(SeqLock& lock) noexcept
            : lock_(lock)
        {
            lock_.acquire_write_lock();
            lock_.compiler_barrier();  // prevents data writes from being hoisted above the seq store
        }

        /// @brief Releases the write lock.
        ~WriteGuard() noexcept
        {
            lock_.compiler_barrier();  // prevents data writes from being sunk below the seq store
            lock_.release_write_lock();
        }

        WriteGuard(const WriteGuard&) = delete;
        auto operator=(const WriteGuard&) -> WriteGuard& = delete;
        WriteGuard(WriteGuard&&) = delete;
        auto operator=(WriteGuard&&) -> WriteGuard& = delete;

        /// @brief Returns a mutable reference to the protected data.
        /// @return Mutable reference to the data.
        [[nodiscard]] auto operator*() -> T& { return lock_.data_; }

        /// @brief Provides pointer access to the protected data.
        /// @return Mutable pointer to the data.
        [[nodiscard]] auto operator->() -> T* { return &lock_.data_; }

    private:
        SeqLock& lock_;
    };

    // ── Construction / destruction ───────────────────────────────────────

    /// @brief Default constructor �?value-initialises the protected data.
    SeqLock() {}

    /// @brief Constructs the seqlock with an initial value.
    /// @param initial The initial value for the protected data.
    explicit SeqLock(const T& initial);

    /// @brief Destructor.
    ~SeqLock() noexcept = default;

    // Disable copy (atomic members are non-copyable).
    SeqLock(const SeqLock&) = delete;
    auto operator=(const SeqLock&) -> SeqLock& = delete;

    /// @brief Move constructor �?transfers the sequence counter and data.
    /// @param other The seqlock to move from.  Left in a valid empty state.
    SeqLock(SeqLock&& other) noexcept;

    /// @brief Move assignment.
    /// @param other The seqlock to move from.
    /// @return Reference to this seqlock.
    auto operator=(SeqLock&& other) noexcept -> SeqLock&;

    // ── Reader API ───────────────────────────────────────────────────────

    /// @brief Spins until a consistent snapshot of the protected data can be
    ///        obtained, then returns it.
    ///
    /// Repeatedly reads the sequence counter, copies the data (protected by
    /// compiler barriers), and verifies consistency.  If a writer is active
    /// or became active during the copy, retries.
    /// @return A consistent copy of the protected data.
    [[nodiscard]] auto load() const -> T;

    /// @brief Single non-blocking attempt to obtain a consistent snapshot.
    ///
    /// Unlike load(), this method never spins.  It returns std::nullopt if
    /// a writer is active or if the data was modified during the copy.
    /// @return A consistent snapshot if one could be obtained in a single
    ///         attempt, or std::nullopt otherwise.
    [[nodiscard]] auto try_load() const -> std::optional<T>;

    /// @brief Returns an RAII ReadGuard containing a consistent snapshot.
    ///
    /// The guard holds the snapshot for its lifetime.  Unlike load(), this
    /// avoids an extra move/copy if the caller needs to access the data
    /// multiple times.
    /// @return ReadGuard whose operator* / operator-> return the snapshot.
    [[nodiscard]] auto protect() const -> ReadGuard;

    // ── Writer API ───────────────────────────────────────────────────────

    /// @brief Exclusive write: acquires the write lock, stores @p value, and
    ///        releases the lock.
    ///
    /// Blocks until any concurrent readers finish their current attempt.
    /// @param value The new value to store.
    void store(const T& value);

    /// @brief Returns an RAII WriteGuard for exclusive write access.
    ///
    /// The guard holds the write lock for its lifetime.  Changes made via
    /// the guard's operator* / operator-> become visible to readers when the
    /// guard is destroyed.
    /// @return WriteGuard that provides mutable access to the data.
    [[nodiscard]] auto lock() -> WriteGuard;

private:
    // ── Internal helpers ─────────────────────────────────────────────────

    /// @brief Compiler-level memory barrier (no CPU fence).
    ///
    /// Prevents the compiler from reordering memory accesses across this
    /// point.  Implementation note: std::atomic_signal_fence with seq_cst
    /// ordering is sufficient because the hardware already provides
    /// cache coherence; only compiler reordering must be constrained.
    static void compiler_barrier() noexcept;

    /// @brief Acquires the write lock by incrementing the sequence counter
    ///        to an odd value.
    ///
    /// Single-writer assumption: no CAS or arbitration is performed.
    /// @pre No concurrent writer (caller must enforce single-writer or
    ///      use MutexSeqLock for multi-writer scenarios).
    void acquire_write_lock() noexcept;

    /// @brief Releases the write lock by incrementing the sequence counter
    ///        to the next even value.
    ///
    /// Simultaneously advances the version number so that in-flight readers
    /// detect a change and retry.
    void release_write_lock() noexcept;

    /// @brief Spin-wait hint for the reader retry loop.
    ///
    /// Executes a short busy-wait using platform-specific pause/yield
    /// instructions (PAUSE on x86, YIELD on ARM64).  After a fixed number
    /// of iterations, yields the remaining timeslice to the OS scheduler.
    static void spin_wait() noexcept;

    // ── Constants ────────────────────────────────────────────────────────

    /// Typical L1/L2 cache-line size on x86 and ARM64.
    static constexpr std::size_t kCacheLineSize = 64;

    // ── Member data ──────────────────────────────────────────────────────

    // The sequence counter lives in the first cache line.
    // Even value = free / no writer active; odd value = writer active.
    std::atomic<std::size_t> seq_{0};

    // Padding: pushes data_ into a separate cache line to eliminate false
    // sharing between readers (who read seq_) and writers (who write both
    // seq_ and data_).
    unsigned char padding_[kCacheLineSize - sizeof(std::atomic<std::size_t>)];

    // The protected data starts at a cache-line boundary (offset 64).
    T data_{};
};

// ══════════════════════════════════════════════════════════════════════════
//  SeqLock Implementation
// ══════════════════════════════════════════════════════════════════════════

// ── Construction / destruction ───────────────────────────────────────────

template <SeqLockable T>
SeqLock<T>::SeqLock(const T& initial)
    : data_(initial)
{
}

// ── Move ─────────────────────────────────────────────────────────────────

template <SeqLockable T>
SeqLock<T>::SeqLock(SeqLock&& other) noexcept
    : seq_(other.seq_.load(std::memory_order_relaxed))
    , data_(std::move(other.data_))
{
    other.seq_.store(0, std::memory_order_relaxed);
    other.data_ = T{};
}

template <SeqLockable T>
auto SeqLock<T>::operator=(SeqLock&& other) noexcept -> SeqLock&
{
    if (this != &other)
    {
        seq_.store(other.seq_.load(std::memory_order_relaxed),
                   std::memory_order_relaxed);
        data_ = std::move(other.data_);
        other.seq_.store(0, std::memory_order_relaxed);
        other.data_ = T{};
    }
    return *this;
}

// ── Internal helpers ─────────────────────────────────────────────────────

template <SeqLockable T>
void SeqLock<T>::compiler_barrier() noexcept
{
    std::atomic_signal_fence(std::memory_order_seq_cst);
}

template <SeqLockable T>
void SeqLock<T>::acquire_write_lock() noexcept
{
    // Single-writer assumption: increment seq from even to odd.
    // memory_order_release ensures all preceding writes are visible
    // to readers that subsequently load seq with memory_order_acquire.
    auto seq = seq_.load(std::memory_order_relaxed);
    seq_.store(seq + 1, std::memory_order_release);
}

template <SeqLockable T>
void SeqLock<T>::release_write_lock() noexcept
{
    // Increment seq from odd back to even (and advance the version).
    auto seq = seq_.load(std::memory_order_relaxed);
    seq_.store(seq + 1, std::memory_order_release);
}

template <SeqLockable T>
void SeqLock<T>::spin_wait() noexcept
{
    static constexpr int kSpinIterations = 100;

    for (int i = 0; i < kSpinIterations; ++i)
    {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
        _mm_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
        #if defined(_MSC_VER)
        __yield();
        #else
        __asm__ __volatile__("yield" ::: "memory");
        #endif
#endif
    }

    // Back off to OS scheduler to avoid starving other threads.
    std::this_thread::yield();
}

// ── Reader API ───────────────────────────────────────────────────────────

template <SeqLockable T>
auto SeqLock<T>::load() const -> T
{
    T           copy;
    std::size_t seq0;
    std::size_t seq1;

    for (;;)
    {
        // R1 �?read the sequence counter (acquire ensures subsequent
        //       loads are not hoisted before this point).
        seq0 = seq_.load(std::memory_order_acquire);

        // Writer is active �?spin-wait, then retry.
        if (seq0 & 1)
        {
            spin_wait();
            continue;
        }

        // Compiler barrier: prevents the compiler from moving the
        // data_ copy (R2) before the seq_ load (R1).
        compiler_barrier();

        // R2 �?copy the protected data.
        copy = data_;

        // Compiler barrier: prevents the compiler from moving the
        // data_ copy (R2) after the seq_ re-read (R3).
        compiler_barrier();

        // R3 �?re-read the sequence counter.
        seq1 = seq_.load(std::memory_order_acquire);

        // If both reads match and the value is even, the snapshot is
        // consistent.
        if (seq0 == seq1)
        {
            return copy;
        }

        // Mismatch (a writer modified seq between R1 and R3) �?retry.
    }
}

template <SeqLockable T>
auto SeqLock<T>::try_load() const -> std::optional<T>
{
    T           copy;
    std::size_t seq0;

    seq0 = seq_.load(std::memory_order_acquire);

    if (seq0 & 1)
    {
        // Writer is active �?cannot obtain a consistent snapshot
        // without spinning.
        return std::nullopt;
    }

    compiler_barrier();
    copy = data_;
    compiler_barrier();

    std::size_t seq1 = seq_.load(std::memory_order_acquire);

    if (seq0 == seq1)
    {
        return copy;
    }

    // Writer modified seq during the copy �?inconsistent.
    return std::nullopt;
}

template <SeqLockable T>
auto SeqLock<T>::protect() const -> ReadGuard
{
    return ReadGuard(*this);
}

// ── Writer API ───────────────────────────────────────────────────────────

template <SeqLockable T>
void SeqLock<T>::store(const T& value)
{
    acquire_write_lock();

    // Compiler barrier: prevents the data write from moving before
    // the acquire_write_lock store.
    compiler_barrier();
    data_ = value;
    // Compiler barrier: prevents the data write from moving after
    // the release_write_lock store.
    compiler_barrier();

    release_write_lock();
}

template <SeqLockable T>
auto SeqLock<T>::lock() -> WriteGuard
{
    return WriteGuard(*this);
}

// ── MutexSeqLock (multi-writer safe) ─────────────────────────────────────

/// @brief SeqLock wrapper that supports multiple concurrent writers via a
///        std::shared_mutex.
///
/// Provides the same seqlock semantics but serialises writers with a mutex.
/// Readers still benefit from the seqlock's fast path: they acquire only a
/// shared (read) lock on the mutex, allowing concurrent reads.
///
/// @tparam T The protected data type.  Must satisfy SeqLockable.
///
/// @par Thread Safety
/// All public methods are thread-safe.  Multiple readers and multiple
/// writers can operate concurrently.  Write-write exclusion is guaranteed
/// by std::shared_mutex; read-write exclusion is handled by SeqLock.
///
/// @par Usage Example
/// @code
/// MutexSeqLock<Config> cfg;
/// // Multiple writers
/// cfg.store(Config{ ... });
/// // Multiple readers
/// Config c = cfg.load();
/// @endcode
template <SeqLockable T>
class MutexSeqLock final
{
public:
    // ── Construction / destruction ───────────────────────────────────────

    /// @brief Default constructor.
    MutexSeqLock() = default;

    /// @brief Constructs with an initial value.
    /// @param initial The initial value for the protected data.
    explicit MutexSeqLock(const T& initial)
        : impl_(initial)
    {
    }

    // Disable copy (mutex is non-copyable).
    MutexSeqLock(const MutexSeqLock&) = delete;
    auto operator=(const MutexSeqLock&) -> MutexSeqLock& = delete;

    /// @brief Destructor.
    ~MutexSeqLock() = default;

    // ── Reader / Writer API ──────────────────────────────────────────────

    /// @brief Exclusive write (multi-writer safe).
    ///
    /// Acquires the internal mutex exclusively, then delegates to SeqLock.
    /// @param value The new value to store.
    void store(const T& value)
    {
        std::unique_lock lock(mtx_);
        impl_.store(value);
    }

    /// @brief Shared read �?returns a consistent snapshot.
    ///
    /// Acquires the internal mutex in shared mode, then delegates to SeqLock.
    /// @return A consistent copy of the protected data.
    [[nodiscard]] auto load() const -> T
    {
        std::shared_lock lock(mtx_);
        return impl_.load();
    }

    /// @brief Non-blocking single-attempt read.
    ///
    /// Attempts to acquire the shared lock without blocking.  If the lock
    /// cannot be acquired or the seqlock snapshot is inconsistent, returns
    /// std::nullopt.
    /// @return A consistent snapshot if one could be obtained immediately,
    ///         or std::nullopt otherwise.
    [[nodiscard]] auto try_load() const -> std::optional<T>
    {
        if (!mtx_.try_lock_shared())
        {
            return std::nullopt;
        }
        std::shared_lock lock(mtx_, std::adopt_lock);
        return impl_.try_load();
    }

private:
    // ── Member data ──────────────────────────────────────────────────────

    mutable std::shared_mutex mtx_;
    SeqLock<T>                impl_;
};

} // namespace cppforge::data_structure::concurrent
