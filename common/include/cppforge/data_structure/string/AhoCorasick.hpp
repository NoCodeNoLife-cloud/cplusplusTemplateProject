/**
 * @file AhoCorasick.hpp
 * @brief Aho-Corasick automaton �?completed transition table (zero-backtracking)
 * @details A production-grade Aho-Corasick multi-pattern string matcher.
 *          Uses a Struct-of-Arrays (SoA) layout for cache-friendly traversal.
 *          During @c build() the goto table is completed into @c trans_ so that
 *          every (state, byte) pair has a valid successor, eliminating fail-link
 *          backtracking during the hot match path (O(1) per character).
 *
 * @par Internal Layout
 * - @c goto_   : flat 256-way transition table (with NO_STATE sentinels).
 * - @c trans_  : completed transition table (every entry valid, no backtracking).
 * - @c fail_   : per-state fail link (dictionary suffix link).
 * - @c output_ : per-state pattern index lists used during construction,
 *                flattened into @c outputData_ / @c outputPtr_ / @c outputCnt_
 *                after @c build().
 *
 * @par Thread Safety
 * - @c insert() and @c build() are **not** thread-safe; callers must
 *   serialise construction.
 * - @c match() and @c matchEach() are **read-only** after @c build() has
 *   completed and are safe for concurrent invocation from multiple threads.
 *
 * @par Usage Example
 * @code
 *   AhoCorasick ac;
 *   ac.insert("he");
 *   ac.insert("she");
 *   ac.insert("his");
 *   ac.insert("hers");
 *   ac.build();
 *
 *   const auto results = ac.match("ushers");
 *   // results: [ {1,3,1} ("she"), {2,2,0} ("he") ]
 * @endcode
 */

#pragma once

#include <cppforge/data_structure/string/IACAutomaton.hpp>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cppforge::data_structure::string
{
    /// @brief Aho-Corasick automaton with completed transition table (zero-backtracking).
    class AhoCorasick final : public IACAutomaton
    {
    public:
        /// @brief Number of entries per state in the flat goto table.
        static constexpr std::size_t ALPHABET_SIZE = 256;

        /// @brief Sentinel value indicating "no state".
        static constexpr int32_t NO_STATE = -1;

        // ── Construction / destruction ───────────────────────────────────

        /// @brief Constructs an empty automaton with only the root state.
        AhoCorasick();

        /// @brief Default destructor.
        ~AhoCorasick() override = default;

        // ── Copy semantics (deleted �?vectors are too large) ─────────────

        AhoCorasick(const AhoCorasick&) = delete;
        auto operator=(const AhoCorasick&) -> AhoCorasick& = delete;

        // ── Move semantics (default �?flat vectors move efficiently) ─────

        AhoCorasick(AhoCorasick&&) = default;
        auto operator=(AhoCorasick&&) -> AhoCorasick& = default;

        // ── IACAutomaton interface ───────────────────────────────────────

        void insert(std::string_view pattern) override;

        void build() override;

        [[nodiscard]] std::size_t patternCount() const noexcept override;

        [[nodiscard]] std::vector<MatchResult>
        match(std::string_view text) const override;

        /// @brief Scans @p text and invokes @p callback for each match.
        ///
        /// Zero-allocation hot path: the callback is invoked inline during
        /// scanning, avoiding the cost of building a result vector.  Matches
        /// are reported in position order (left-to-right scan); within the
        /// same position the order is unspecified.
        ///
        /// @tparam Callback  Invocable with <tt>const MatchResult&</tt>.
        /// @param  text  Byte sequence to scan.
        /// @param  cb    Callback invoked for each match.
        /// @throws std::runtime_error if build() has not been called.
        template <typename Callback>
        void matchEach(std::string_view text, Callback&& cb) const
        {
            if (!built_)
            {
                throw std::runtime_error(
                    "AhoCorasick::matchEach: build() must be called first");
            }

            auto state = int32_t{0};
            const auto* trans = trans_.data();
            const auto* outCnt = outputCnt_.data();
            const auto* outPtr = outputPtr_.data();
            const auto* outData = outputData_.data();

            for (std::size_t i = 0; i < text.size(); ++i)
            {
                const auto c = static_cast<uint8_t>(text[i]);
                state = trans[static_cast<std::size_t>(state) * ALPHABET_SIZE + c];

                if (const auto cnt = outCnt[state]; cnt > 0)
                {
                    const auto off = outPtr[state];
                    for (int32_t j = 0; j < cnt; ++j)
                    {
                        const auto patIdx = static_cast<std::size_t>(outData[off + j]);
                        const auto& pat = patterns_[patIdx];
                        cb(MatchResult{
                            i - pat.size() + 1,
                            pat.size(),
                            patIdx
                        });
                    }
                }
            }
        }

        [[nodiscard]] std::string_view
        getPattern(std::size_t index) const override;

    private:
        // ══════════════════════════════════════════════════════════════════
        //  SoA storage (Struct-of-Arrays)
        // ══════════════════════════════════════════════════════════════════

        /// @brief Flat 256-way goto table.  Index: state × 256 + byte.
        ///        Unfilled transitions store @c NO_STATE (-1).
        std::vector<int32_t> goto_;

        /// @brief Completed transition table (zero-backtracking).
        ///        Same layout as goto_ but every (state, byte) pair has a valid
        ///        successor (no NO_STATE).  Populated during build() after BFS.
        std::vector<int32_t> trans_;

        /// @brief Per-state fail link.  size = numStates.
        std::vector<int32_t> fail_;

        /// @brief Flattened pattern-index output; each state's output list
        ///        occupies a contiguous slice.
        ///        Only valid after @c build().
        std::vector<int32_t> outputData_;

        /// @brief Start offset into @c outputData_ for each state.
        ///        size = numStates.
        std::vector<int32_t> outputPtr_;

        /// @brief Number of pattern entries for each state.
        ///        size = numStates.
        std::vector<int32_t> outputCnt_;

        /// @brief Original pattern strings, indexed by insertion order.
        std::vector<std::string> patterns_;

        /// @brief Whether @c build() has completed (transition table ready).
        bool built_ = false;

        // ── Temporary per-state output (used during insertion / build,
        //    flattened into SoA arrays after build) ─────────────────────

        /// @brief Per-state vector of pattern indices.
        ///        Used during @c insert() and @c build() for easy merge.
        ///        Cleared after flattening in @c build().
        std::vector<std::vector<int32_t>> output_;

        // ══════════════════════════════════════════════════════════════════
        //  Internal helpers
        // ══════════════════════════════════════════════════════════════════

        /// @brief Allocates a new state and extends all SoA vectors.
        /// @return The index of the newly allocated state.
        [[nodiscard]] auto addState() -> std::size_t;

        /// @brief Merges pattern indices from @p failState into @p state's
        ///        output list, avoiding duplicates.
        /// @param state      The destination state.
        /// @param failState  The source state (typically @c fail_[state]).
        void mergeOutput(std::size_t state, std::size_t failState);

        /// @brief Flattens the temporary @c output_ into the SoA arrays
        ///        (@c outputData_, @c outputPtr_, @c outputCnt_).
        void flattenOutput();
    };

} // namespace cppforge::data_structure::string
