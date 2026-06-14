/**
 * @file IACAutomaton.hpp
 * @brief Abstract interface for Aho-Corasick automaton (multi-pattern string matching)
 * @details Defines the IACAutomaton interface and the MatchResult structure.  An
 *          Aho-Corasick automaton pre-processes a set of pattern strings into a
 *          finite-state machine (Trie + fail links) and can then scan an input
 *          text in O(n + m) time to find all occurrences of any pattern, where
 *          n is the text length and m is the total number of matches.
 *
 * Reference: Aho, A.V. and Corasick, M.J., "Efficient string matching: an aid to
 *            bibliographic search", Communications of the ACM, 18(6):333–340, 1975.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace common::data_structure::string
{
    /// @brief Describes a single match found by the automaton.
    struct MatchResult
    {
        std::size_t position;      ///< Byte offset of match start in the scanned text.
        std::size_t length;        ///< Length of the matched pattern (bytes).
        std::size_t patternIndex;  ///< Index of the matched pattern (insertion order).
    };

    /// @brief Abstract interface for an Aho-Corasick automaton.
    ///
    /// Typical usage:
    /// @code
    ///   std::unique_ptr<IACAutomaton> ac = std::make_unique<AhoCorasick>();
    ///   ac->insert("he");
    ///   ac->insert("she");
    ///   ac->insert("his");
    ///   ac->insert("hers");
    ///   ac->build();
    ///
    ///   const auto results = ac->match("ushers");
    ///   for (const auto& r : results)
    ///   {
    ///       // r.position == 1, r.length == 3, pattern == "she"
    ///       // r.position == 2, r.length == 2, pattern == "he"
    ///   }
    /// @endcode
    ///
    /// @par Thread Safety
    /// Implementations are **not** required to be thread-safe unless explicitly
    /// documented.
    ///
    /// @par Lifecycle
    /// 1. Call @c insert() zero or more times to register patterns.
    /// 2. Call @c build() once to construct the internal fail links.
    /// 3. Call @c match() any number of times.
    /// 4. Do **not** insert new patterns after @c build().
    class IACAutomaton
    {
    public:
        virtual ~IACAutomaton() = default;

        // ── Pattern registration ────────────────────────────────────────

        /// @brief Inserts a pattern into the automaton's dictionary.
        ///
        /// Patterns can be inserted in any order.  Each pattern receives a
        /// unique index equal to its insertion order (0-based).  Duplicate
        /// patterns are stored as separate entries with distinct indices.
        ///
        /// @param pattern The pattern string to insert.
        /// @pre  @c build() has **not** been called yet.
        /// @post @c patternCount() is incremented by 1.
        /// @throws std::invalid_argument if @p pattern is empty.
        virtual void insert(std::string_view pattern) = 0;

        /// @brief Builds the fail-link table and prepares the automaton for
        ///        matching.
        ///
        /// After this call the automaton transitions from "construction" mode
        /// to "matching" mode.  No further patterns may be inserted.
        ///
        /// It is safe (but redundant) to call @c build() multiple times.
        virtual void build() = 0;

        // ── Queries ─────────────────────────────────────────────────────

        /// @brief Returns the number of patterns registered in the automaton.
        /// @return Pattern count (also equals the number of @c insert() calls).
        [[nodiscard]] virtual std::size_t patternCount() const noexcept = 0;

        /// @brief Scans the input @p text and returns all matches.
        ///
        /// Matches may overlap and are reported as they are encountered
        /// during the left-to-right scan (i.e. in position order).
        /// Within the same position, matches are reported in no particular
        /// order.
        ///
        /// @param  text The text to scan.
        /// @return A vector of MatchResult describing every occurrence of
        ///         any registered pattern in @p text.
        /// @pre    @c build() has been called at least once.
        [[nodiscard]] virtual std::vector<MatchResult>
        match(std::string_view text) const = 0;

        /// @brief Retrieves the pattern string by its insertion index.
        /// @param  index The 0-based pattern index.
        /// @return The pattern string.
        /// @throws std::out_of_range if @p index is out of bounds.
        [[nodiscard]] virtual std::string_view
        getPattern(std::size_t index) const = 0;
    };

} // namespace common::data_structure::string
