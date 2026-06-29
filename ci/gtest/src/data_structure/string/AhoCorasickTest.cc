/**
 * @file AhoCorasickTest.cc
 * @brief Unit tests for the Aho-Corasick automaton (Task 1 �?fail-link backtracking)
 * @details Comprehensive test suite covering:
 *          - Construction and empty-state behaviour
 *          - Insertion and pattern counting
 *          - Single-pattern matching (beginning, middle, end, multiple, absent)
 *          - Multi-pattern matching (disjoint, shared-prefix, prefix-contained, classic example)
 *          - Overlapping and nested patterns
 *          - Boundary and exception cases (empty pattern/text, non-ASCII, build idempotency)
 *          - Polymorphic usage through the IACAutomaton interface
 *          - Thread safety (concurrent match/matchEach, mixed usage)
 *          - Edge cases (insert-after-build state unchanged, build-twice stability)
 *
 * @par Framework
 *   Google Test (gtest), using TEST_F with a shared fixture.
 *
 * @par Reference
 *   Aho, A.V. and Corasick, M.J., "Efficient string matching: an aid to
 *   bibliographic search", Communications of the ACM, 18(6):333�?40, 1975.
 */

#include <gtest/gtest.h>

#include "data_structure/string/AhoCorasick.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

using namespace cppforge::data_structure::string;

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

/// @brief Test fixture for AhoCorasick tests.
///
/// Provides common type aliases and a helper that inserts patterns,
/// builds the automaton, and returns the sorted match results as a
/// std::set of (position, length, patternIndex) tuples for easy
/// set-based assertion.
class AhoCorasickTest : public testing::Test
{
protected:
    /// Convenience alias for the type under test.
    using ACA = AhoCorasick;

    /// Shorthand: (position, length, patternIndex)
    using MatchTriple = std::tuple<std::size_t, std::size_t, std::size_t>;

    /// @brief Inserts patterns, builds, matches, and returns a sorted set.
    /// @param ac     The automaton (non-const because insert/build are mutating).
    /// @param text   The text to scan.
    /// @return A set of (position, length, patternIndex) for easy assertion.
    [[nodiscard]] static auto
    buildAndMatch(ACA& ac, std::string_view text) -> std::set<MatchTriple>
    {
        ac.build();
        const auto results = ac.match(text);
        std::set<MatchTriple> triples;
        for (const auto& r : results)
        {
            triples.emplace(r.position, r.length, r.patternIndex);
        }
        return triples;
    }

    /// @brief Inserts a list of patterns into the automaton.
    static void insertPatterns(ACA& ac, std::initializer_list<std::string_view> patterns)
    {
        for (const auto& p : patterns)
        {
            ac.insert(p);
        }
    }

    /// @brief Calls matchEach() and collects results into a vector.
    /// @param ac   The built automaton.
    /// @param text The text to scan.
    /// @return A vector of MatchResult collected via the callback.
    [[nodiscard]] static auto
    collectMatchEach(const ACA& ac, std::string_view text) -> std::vector<MatchResult>
    {
        std::vector<MatchResult> results;
        ac.matchEach(text, [&](const MatchResult& m) { results.push_back(m); });
        return results;
    }

    /// @brief Asserts that matchEach() and match() produce identical match sets.
    ///
    /// Both result sets are converted to std::set<MatchTriple> for an
    /// order-independent comparison.
    ///
    /// @param ac   The built automaton.
    /// @param text The text to scan.
    static void
    assertMatchEachEqualsMatch(const ACA& ac, std::string_view text)
    {
        const auto cbResults = collectMatchEach(ac, text);
        const auto directResults = ac.match(text);

        std::set<MatchTriple> cbSet;
        for (const auto& r : cbResults)
        {
            cbSet.emplace(r.position, r.length, r.patternIndex);
        }

        std::set<MatchTriple> directSet;
        for (const auto& r : directResults)
        {
            directSet.emplace(r.position, r.length, r.patternIndex);
        }

        ASSERT_EQ(cbSet, directSet);
    }
};

// ══════════════════════════════════════════════════════════════════════════
//  1. Construction & empty-state behaviour  (5 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Default-constructed automaton has zero patterns.
 * @details Verifies the post-condition of the default constructor.
 */
TEST_F(AhoCorasickTest, Constructor_Default_Empty)
{
    const ACA ac;
    EXPECT_EQ(ac.patternCount(), 0U);
}

/**
 * @brief Calling match() before build() throws std::runtime_error.
 * @details The automaton must be built before it can be used for matching.
 */
TEST_F(AhoCorasickTest, Constructor_MatchBeforeBuild_Throws)
{
    const ACA ac;
    EXPECT_THROW(static_cast<void>(ac.match("text")), std::runtime_error);
}

/**
 * @brief Empty automaton (build then match) returns an empty result.
 * @details After build() with no patterns inserted, match() always yields
 *          an empty vector.
 */
TEST_F(AhoCorasickTest, Constructor_EmptyAutomatonMatch_ReturnsEmpty)
{
    ACA ac;
    ac.build();
    const auto results = ac.match("any text whatsoever");
    EXPECT_TRUE(results.empty());
}

/**
 * @brief getPattern() with an out-of-range index throws std::out_of_range.
 * @details Throws via std::vector::at() when the index is >= patternCount().
 */
TEST_F(AhoCorasickTest, Constructor_GetPatternOutOfRange_Throws)
{
    const ACA ac;
    EXPECT_THROW(static_cast<void>(ac.getPattern(0)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(ac.getPattern(42)), std::out_of_range);
}

/**
 * @brief Calling build() with no patterns inserted does not throw or crash.
 * @details build() must be safe to call even when the dictionary is empty.
 */
TEST_F(AhoCorasickTest, Constructor_BuildWithoutInsert_Succeeds)
{
    ACA ac;
    EXPECT_NO_THROW(ac.build());
    // After build, match should still be callable and return empty.
    const auto results = ac.match("");
    EXPECT_TRUE(results.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Insertion & pattern counting  (4 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Inserting a single pattern yields patternCount() == 1.
 */
TEST_F(AhoCorasickTest, Insert_SinglePattern_CountIsOne)
{
    ACA ac;
    EXPECT_EQ(ac.patternCount(), 0U);
    ac.insert("hello");
    EXPECT_EQ(ac.patternCount(), 1U);
}

/**
 * @brief Inserting multiple patterns increments count correctly.
 */
TEST_F(AhoCorasickTest, Insert_MultiplePatterns_CountCorrect)
{
    ACA ac;
    insertPatterns(ac, {"alpha", "beta", "gamma", "delta"});
    EXPECT_EQ(ac.patternCount(), 4U);
}

/**
 * @brief Duplicate patterns are stored as separate entries with distinct indices.
 * @details The interface specification states that duplicate patterns receive
 *          distinct indices equal to their insertion order.
 */
TEST_F(AhoCorasickTest, Insert_DuplicatePattern_DistinctIndices)
{
    ACA ac;
    ac.insert("dup");
    ac.insert("dup");
    ac.insert("dup");
    EXPECT_EQ(ac.patternCount(), 3U);
    EXPECT_EQ(ac.getPattern(0), "dup");
    EXPECT_EQ(ac.getPattern(1), "dup");
    EXPECT_EQ(ac.getPattern(2), "dup");
}

/**
 * @brief Insert after build() throws std::runtime_error.
 * @details Once the automaton transitions to matching mode, no further
 *          patterns may be added.
 */
TEST_F(AhoCorasickTest, Insert_AfterBuild_Throws)
{
    ACA ac;
    ac.insert("before");
    ac.build();
    EXPECT_THROW(ac.insert("after"), std::runtime_error);
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Single-pattern matching  (5 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Pattern located at the beginning of the text is found.
 */
TEST_F(AhoCorasickTest, SinglePattern_AtBeginning)
{
    ACA ac;
    ac.insert("start");
    const auto triples = buildAndMatch(ac, "start of the text");

    ASSERT_EQ(triples.size(), 1U);
    EXPECT_TRUE(triples.contains({0, 5, 0}));
}

/**
 * @brief Pattern located in the middle of the text is found.
 */
TEST_F(AhoCorasickTest, SinglePattern_InMiddle)
{
    ACA ac;
    ac.insert("middle");
    const auto triples = buildAndMatch(ac, "the middle part");

    ASSERT_EQ(triples.size(), 1U);
    EXPECT_TRUE(triples.contains({4, 6, 0}));
}

/**
 * @brief Pattern located at the very end of the text is found.
 */
TEST_F(AhoCorasickTest, SinglePattern_AtEnd)
{
    ACA ac;
    ac.insert("end");
    const auto triples = buildAndMatch(ac, "this is the end");

    ASSERT_EQ(triples.size(), 1U);
    EXPECT_TRUE(triples.contains({12, 3, 0}));
}

/**
 * @brief Pattern appears multiple times in the text; all occurrences found.
 */
TEST_F(AhoCorasickTest, SinglePattern_MultipleOccurrences)
{
    ACA ac;
    ac.insert("aa");
    const auto triples = buildAndMatch(ac, "aaaa");

    // "aaaa" contains "aa" at positions 0, 1, 2
    ASSERT_EQ(triples.size(), 3U);
    EXPECT_TRUE(triples.contains({0, 2, 0}));
    EXPECT_TRUE(triples.contains({1, 2, 0}));
    EXPECT_TRUE(triples.contains({2, 2, 0}));
}

/**
 * @brief Pattern does not occur in the text; result is empty.
 */
TEST_F(AhoCorasickTest, SinglePattern_NotFound_ReturnsEmpty)
{
    ACA ac;
    ac.insert("xyz");
    const auto triples = buildAndMatch(ac, "there is no match here");
    EXPECT_TRUE(triples.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  4. Multi-pattern matching  (5 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Multiple disjoint (non-overlapping) patterns are all found.
 */
TEST_F(AhoCorasickTest, MultiplePatterns_Disjoint)
{
    ACA ac;
    insertPatterns(ac, {"abc", "xyz", "123"});
    const auto triples = buildAndMatch(ac, "---abc---xyz---123---");

    ASSERT_EQ(triples.size(), 3U);
    EXPECT_TRUE(triples.contains({3, 3, 0}));   // "abc" at pos 3
    EXPECT_TRUE(triples.contains({9, 3, 1}));   // "xyz" at pos 9
    EXPECT_TRUE(triples.contains({15, 3, 2}));  // "123" at pos 15
}

/**
 * @brief Patterns that share a common prefix match correctly.
 * @details e.g. "ab", "abc", "abcd" all share the prefix "ab".
 */
TEST_F(AhoCorasickTest, MultiplePatterns_SharedPrefix)
{
    ACA ac;
    insertPatterns(ac, {"ab", "abc", "abcd"});
    const auto triples = buildAndMatch(ac, "abcd");

    ASSERT_EQ(triples.size(), 3U);
    EXPECT_TRUE(triples.contains({0, 2, 0}));   // "ab"
    EXPECT_TRUE(triples.contains({0, 3, 1}));   // "abc"
    EXPECT_TRUE(triples.contains({0, 4, 2}));   // "abcd"
}

/**
 * @brief One pattern is a strict prefix of another; both are found.
 */
TEST_F(AhoCorasickTest, MultiplePatterns_OneIsPrefix)
{
    ACA ac;
    insertPatterns(ac, {"a", "aa", "aaa"});
    const auto triples = buildAndMatch(ac, "aaa");

    ASSERT_EQ(triples.size(), 6U);
    // Positions for "a":   0, 1, 2
    // Positions for "aa":  0, 1
    // Positions for "aaa": 0
    EXPECT_TRUE(triples.contains({0, 1, 0}));
    EXPECT_TRUE(triples.contains({1, 1, 0}));
    EXPECT_TRUE(triples.contains({2, 1, 0}));
    EXPECT_TRUE(triples.contains({0, 2, 1}));
    EXPECT_TRUE(triples.contains({1, 2, 1}));
    EXPECT_TRUE(triples.contains({0, 3, 2}));
}

/**
 * @brief Classic AC example from the original paper.
 * @details Patterns: {he, she, his, hers}. Text: "ushers".
 *          Expected matches: she@1, he@2, hers@2.
 */
TEST_F(AhoCorasickTest, MultiplePatterns_ClassicExample)
{
    ACA ac;
    insertPatterns(ac, {"he", "she", "his", "hers"});
    const auto triples = buildAndMatch(ac, "ushers");

    ASSERT_EQ(triples.size(), 3U);
    EXPECT_TRUE(triples.contains({1, 3, 1}));  // "she"
    EXPECT_TRUE(triples.contains({2, 2, 0}));  // "he"
    EXPECT_TRUE(triples.contains({2, 4, 3}));  // "hers"
}

/**
 * @brief Each pattern appears multiple times in the text.
 */
TEST_F(AhoCorasickTest, MultiplePatterns_MultipleMatchesEach)
{
    ACA ac;
    insertPatterns(ac, {"ab", "bc"});
    const auto triples = buildAndMatch(ac, "abcabc");

    ASSERT_EQ(triples.size(), 4U);
    EXPECT_TRUE(triples.contains({0, 2, 0}));  // "ab" at 0
    EXPECT_TRUE(triples.contains({1, 2, 1}));  // "bc" at 1
    EXPECT_TRUE(triples.contains({3, 2, 0}));  // "ab" at 3
    EXPECT_TRUE(triples.contains({4, 2, 1}));  // "bc" at 4
}

// ══════════════════════════════════════════════════════════════════════════
//  5. Overlapping & nested patterns  (4 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Overlapping patterns where matches share characters.
 * @details Patterns {"ana", "anana"} in text "banana":
 *          - "ana"   at positions 1 and 3
 *          - "anana" at position 1
 */
TEST_F(AhoCorasickTest, Overlapping_AnaAnana)
{
    ACA ac;
    insertPatterns(ac, {"ana", "anana"});
    const auto triples = buildAndMatch(ac, "banana");

    ASSERT_EQ(triples.size(), 3U);
    EXPECT_TRUE(triples.contains({1, 3, 0}));  // "ana"
    EXPECT_TRUE(triples.contains({3, 3, 0}));  // "ana"
    EXPECT_TRUE(triples.contains({1, 5, 1}));  // "anana"
}

/**
 * @brief Nested patterns of increasing length.
 * @details Patterns {"a", "aa", "aaa"} in text "aaaa":
 *          - "a"   appears at positions 0,1,2,3
 *          - "aa"  appears at positions 0,1,2
 *          - "aaa" appears at positions 0,1
 */
TEST_F(AhoCorasickTest, Overlapping_NestedPatterns)
{
    ACA ac;
    insertPatterns(ac, {"a", "aa", "aaa"});
    const auto triples = buildAndMatch(ac, "aaaa");

    // Total match count: 4 + 3 + 2 = 9
    ASSERT_EQ(triples.size(), 9U);
    // "a" x4
    EXPECT_TRUE(triples.contains({0, 1, 0}));
    EXPECT_TRUE(triples.contains({1, 1, 0}));
    EXPECT_TRUE(triples.contains({2, 1, 0}));
    EXPECT_TRUE(triples.contains({3, 1, 0}));
    // "aa" x3
    EXPECT_TRUE(triples.contains({0, 2, 1}));
    EXPECT_TRUE(triples.contains({1, 2, 1}));
    EXPECT_TRUE(triples.contains({2, 2, 1}));
    // "aaa" x2
    EXPECT_TRUE(triples.contains({0, 3, 2}));
    EXPECT_TRUE(triples.contains({1, 3, 2}));
}

/**
 * @brief Complex overlapping with "the", "there", "they" in "there they are".
 * @details "there they are" contains:
 *          - "the"   at 0 (inside "there")
 *          - "there" at 0
 *          - "the"   at 6 (inside "they")
 *          - "they"  at 6
 */
TEST_F(AhoCorasickTest, Overlapping_ComplexSet)
{
    ACA ac;
    insertPatterns(ac, {"the", "there", "their", "they"});
    const auto triples = buildAndMatch(ac, "there they are");

    ASSERT_EQ(triples.size(), 4U);
    EXPECT_TRUE(triples.contains({0, 3, 0}));   // "the"
    EXPECT_TRUE(triples.contains({0, 5, 1}));   // "there"
    EXPECT_TRUE(triples.contains({6, 3, 0}));   // "the" inside "they"
    EXPECT_TRUE(triples.contains({6, 4, 3}));   // "they"
}

/**
 * @brief Every character is a pattern; all substrings are matched.
 */
TEST_F(AhoCorasickTest, Overlapping_AllCharactersArePatterns)
{
    ACA ac;
    insertPatterns(ac, {"a", "b", "c", "ab", "bc"});
    const auto triples = buildAndMatch(ac, "abc");

    ASSERT_EQ(triples.size(), 5U);
    EXPECT_TRUE(triples.contains({0, 1, 0}));  // "a"
    EXPECT_TRUE(triples.contains({1, 1, 1}));  // "b"
    EXPECT_TRUE(triples.contains({2, 1, 2}));  // "c"
    EXPECT_TRUE(triples.contains({0, 2, 3}));  // "ab"
    EXPECT_TRUE(triples.contains({1, 2, 4}));  // "bc"
}

// ══════════════════════════════════════════════════════════════════════════
//  6. Boundary & exception cases  (8 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Inserting an empty pattern string throws std::invalid_argument.
 * @details The automaton explicitly rejects empty patterns at insertion
 *          time.  This test verifies that the exception is thrown, and
 *          that the automaton remains in a consistent state afterwards.
 */
TEST_F(AhoCorasickTest, Boundary_EmptyPattern)
{
    ACA ac;
    EXPECT_THROW(ac.insert(""), std::invalid_argument);
    EXPECT_EQ(ac.patternCount(), 0U);

    // Automaton is still usable after the failed insertion
    ac.build();
    const auto results = ac.match("abc");
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Inserting an empty pattern after a valid pattern does not corrupt
 *        the automaton state.
 * @details Verifies that:
 *          1. A valid pattern is inserted successfully.
 *          2. A subsequent empty-pattern insertion throws std::invalid_argument.
 *          3. The pattern count remains unchanged (the valid pattern is kept).
 *          4. The automaton can still be built and produces correct match results.
 */
TEST_F(AhoCorasickTest, Boundary_EmptyPattern_AfterValidInsert)
{
    ACA ac;

    // Insert a valid pattern first
    ac.insert("hello");
    EXPECT_EQ(ac.patternCount(), 1U);
    EXPECT_EQ(ac.getPattern(0), "hello");

    // Now attempt to insert an empty pattern �?must throw and leave state intact
    EXPECT_THROW(ac.insert(""), std::invalid_argument);
    EXPECT_EQ(ac.patternCount(), 1U);
    EXPECT_EQ(ac.getPattern(0), "hello");

    // Build and match �?the valid pattern must still work
    ac.build();
    const auto triples = buildAndMatch(ac, "hello world");
    ASSERT_EQ(triples.size(), 1U);
    EXPECT_TRUE(triples.contains({0, 5, 0}));
}

/**
 * @brief Repeatedly inserting empty patterns always throws and never
 *        corrupts the automaton state.
 * @details Multiple consecutive calls to insert("") must each throw
 *          std::invalid_argument, and patternCount() must remain 0
 *          throughout.
 */
TEST_F(AhoCorasickTest, Boundary_EmptyPattern_MultipleRejections)
{
    ACA ac;

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_THROW(ac.insert(""), std::invalid_argument);
        EXPECT_EQ(ac.patternCount(), 0U);
    }

    // The automaton must still be usable after the repeated rejections
    ac.build();
    const auto results = ac.match("any text");
    EXPECT_TRUE(results.empty());
}

/**
 * @brief Inserting an empty pattern after build() throws std::invalid_argument
 *        (not std::runtime_error).
 * @details The empty-pattern check occurs before the built_ check in insert(),
 *          so the exception type is std::invalid_argument regardless of whether
 *          build() has been called.  This test documents this precedence behaviour.
 */
TEST_F(AhoCorasickTest, Boundary_EmptyPattern_AfterBuild_ThrowsInvalidArgument)
{
    ACA ac;
    ac.insert("valid");
    ac.build();

    // The empty check fires first �?expecting invalid_argument, not runtime_error
    EXPECT_THROW(ac.insert(""), std::invalid_argument);

    // Match results must be unchanged
    const auto results = ac.match("valid");
    ASSERT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0].position, 0U);
    EXPECT_EQ(results[0].length, 5U);
    EXPECT_EQ(results[0].patternIndex, 0U);
}

/**
 * @brief Matching against an empty text string returns empty results.
 */
TEST_F(AhoCorasickTest, Boundary_EmptyText)
{
    ACA ac;
    insertPatterns(ac, {"pattern1", "pattern2"});
    const auto triples = buildAndMatch(ac, "");
    EXPECT_TRUE(triples.empty());
}

/**
 * @brief Pattern longer than the text never matches.
 */
TEST_F(AhoCorasickTest, Boundary_PatternLongerThanText)
{
    ACA ac;
    ac.insert("very long pattern that exceeds text");
    const auto triples = buildAndMatch(ac, "short");
    EXPECT_TRUE(triples.empty());
}

/**
 * @brief Non-ASCII bytes (e.g. UTF-8 encoded characters) are matched correctly.
 * @details The automaton operates on raw bytes (ALPHABET_SIZE = 256), so
 *          multi-byte UTF-8 sequences are treated as individual byte patterns.
 *          This test uses the UTF-8 encoding of "é" (U+00E9 �?0xC3 0xA9).
 */
TEST_F(AhoCorasickTest, Boundary_NonAsciiBytes)
{
    ACA ac;
    // é in UTF-8: 0xC3 0xA9
    ac.insert("\xC3\xA9");
    const auto triples = buildAndMatch(ac, "caf\xC3\xA9");  // "café"

    ASSERT_EQ(triples.size(), 1U);
    EXPECT_TRUE(triples.contains({3, 2, 0}));  // position 3, 2 bytes
}

/**
 * @brief build() is idempotent; multiple calls do not change match results.
 */
TEST_F(AhoCorasickTest, Boundary_BuildIdempotent)
{
    ACA ac;
    insertPatterns(ac, {"he", "she", "hers"});
    ac.build();

    const auto first = ac.match("ushers");

    // Call build() two more times
    ac.build();
    ac.build();

    const auto second = ac.match("ushers");

    ASSERT_EQ(first.size(), second.size());
    for (std::size_t i = 0; i < first.size(); ++i)
    {
        EXPECT_EQ(first[i].position, second[i].position);
        EXPECT_EQ(first[i].length, second[i].length);
        EXPECT_EQ(first[i].patternIndex, second[i].patternIndex);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  7. Polymorphic usage through IACAutomaton interface  (3 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Full lifecycle via std::unique_ptr<IACAutomaton>.
 * @details Verifies that insert, build, match, patternCount, and getPattern
 *          all work correctly when accessed through the abstract interface.
 */
TEST_F(AhoCorasickTest, Interface_FullLifecycle)
{
    auto ac = std::unique_ptr<IACAutomaton>(new ACA());

    EXPECT_EQ(ac->patternCount(), 0U);

    ac->insert("hello");
    ac->insert("world");
    EXPECT_EQ(ac->patternCount(), 2U);

    ac->build();

    const auto results = ac->match("hello world");
    ASSERT_EQ(results.size(), 2U);

    // Results may be in any order, build a set for comparison.
    std::set<MatchTriple> triples;
    for (const auto& r : results)
    {
        triples.emplace(r.position, r.length, r.patternIndex);
    }
    EXPECT_TRUE(triples.contains({0, 5, 0}));   // "hello"
    EXPECT_TRUE(triples.contains({6, 5, 1}));   // "world"

    EXPECT_EQ(ac->getPattern(0), "hello");
    EXPECT_EQ(ac->getPattern(1), "world");
}

/**
 * @brief Interface pointer correctly throws on error conditions.
 * @details Verifies that match() before build() and getPattern() OOB both
 *          throw through the polymorphic interface.
 */
TEST_F(AhoCorasickTest, Interface_ErrorHandling)
{
    auto ac = std::unique_ptr<IACAutomaton>(new ACA());

    // match() before build() �?runtime_error
    EXPECT_THROW(static_cast<void>(ac->match("text")), std::runtime_error);

    // getPattern() on empty automaton �?out_of_range
    EXPECT_THROW(static_cast<void>(ac->getPattern(0)), std::out_of_range);

    // insert after build �?runtime_error
    ac->insert("pattern");
    ac->build();
    EXPECT_THROW(ac->insert("another"), std::runtime_error);
}

/**
 * @brief Interface pointer works with the classic AC example.
 * @details Ensures that the polymorphic dispatch produces exactly the same
 *          results as direct usage.
 */
TEST_F(AhoCorasickTest, Interface_ClassicExample)
{
    auto ac = std::unique_ptr<IACAutomaton>(new ACA());

    ac->insert("he");
    ac->insert("she");
    ac->insert("his");
    ac->insert("hers");
    ac->build();

    const auto results = ac->match("ushers");

    ASSERT_EQ(results.size(), 3U);

    std::set<MatchTriple> triples;
    for (const auto& r : results)
    {
        triples.emplace(r.position, r.length, r.patternIndex);
    }
    EXPECT_TRUE(triples.contains({1, 3, 1}));  // "she"
    EXPECT_TRUE(triples.contains({2, 2, 0}));  // "he"
    EXPECT_TRUE(triples.contains({2, 4, 3}));  // "hers"
}

// ══════════════════════════════════════════════════════════════════════════
//  8. Additional regression �?getPattern returns original string  (1 test)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief getPattern() returns the exact string that was inserted.
 * @details Verifies that no transformation is applied to the stored pattern,
 *          including empty strings and multi-byte UTF-8 sequences.
 */
TEST_F(AhoCorasickTest, GetPattern_ReturnsOriginalString)
{
    ACA ac;
    ac.insert("a");                   // single character
    ac.insert("hello world");         // conventional text
    ac.insert("\xC3\xA9\xC3\xA0");   // UTF-8 multi-byte: éà

    EXPECT_EQ(ac.patternCount(), 3U);
    EXPECT_EQ(ac.getPattern(0), "a");
    EXPECT_EQ(ac.getPattern(1), "hello world");
    EXPECT_EQ(ac.getPattern(2), "\xC3\xA9\xC3\xA0");
}

// ══════════════════════════════════════════════════════════════════════════
//  9. matchEach �?zero-allocation callback API  (4 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Single pattern: callback is invoked once with correct match details.
 */
TEST_F(AhoCorasickTest, MatchEach_SinglePattern)
{
    ACA ac;
    ac.insert("hello");
    ac.build();

    std::vector<MatchResult> results;
    ac.matchEach("hello world", [&](const MatchResult& m) { results.push_back(m); });

    ASSERT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0].position, 0U);
    EXPECT_EQ(results[0].length, 5U);
    EXPECT_EQ(results[0].patternIndex, 0U);
}

/**
 * @brief Multiple patterns: all matches are reported via the callback.
 */
TEST_F(AhoCorasickTest, MatchEach_MultiplePatterns)
{
    ACA ac;
    insertPatterns(ac, {"abc", "xyz", "123"});
    ac.build();

    std::set<MatchTriple> cbSet;
    ac.matchEach("---abc---xyz---123---", [&](const MatchResult& m) {
        cbSet.emplace(m.position, m.length, m.patternIndex);
    });

    ASSERT_EQ(cbSet.size(), 3U);
    EXPECT_TRUE(cbSet.contains({3, 3, 0}));
    EXPECT_TRUE(cbSet.contains({9, 3, 1}));
    EXPECT_TRUE(cbSet.contains({15, 3, 2}));
}

/**
 * @brief No matching pattern: callback is never invoked.
 */
TEST_F(AhoCorasickTest, MatchEach_NoMatch_DoesNotCall)
{
    ACA ac;
    ac.insert("xyz");
    ac.build();

    int callCount = 0;
    ac.matchEach("there is no match here", [&](const MatchResult&) {
        ++callCount;
    });

    EXPECT_EQ(callCount, 0);
}

/**
 * @brief Calling matchEach() before build() throws std::runtime_error.
 * @details This mirrors the precondition of match().
 */
TEST_F(AhoCorasickTest, MatchEach_BeforeBuild_Throws)
{
    ACA ac;
    ac.insert("pattern");
    // build() is intentionally NOT called

    EXPECT_THROW(
        ac.matchEach("text", [](const MatchResult&) {}),
        std::runtime_error
    );
}

// ══════════════════════════════════════════════════════════════════════════
//  10. matchEach equivalence with match()  (3 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Classic AC example {he, she, his, hers} on "ushers".
 * @details Verifies that matchEach and match produce identical match sets
 *          for the textbook example from the original 1975 paper.
 */
TEST_F(AhoCorasickTest, MatchEach_EqualsMatch_ClassicExample)
{
    ACA ac;
    insertPatterns(ac, {"he", "she", "his", "hers"});
    ac.build();

    assertMatchEachEqualsMatch(ac, "ushers");
}

/**
 * @brief Overlapping patterns {ana, anana} on "banana".
 * @details Tests equivalence when matches overlap and share characters.
 */
TEST_F(AhoCorasickTest, MatchEach_EqualsMatch_Overlapping)
{
    ACA ac;
    insertPatterns(ac, {"ana", "anana"});
    ac.build();

    assertMatchEachEqualsMatch(ac, "banana");
}

/**
 * @brief Large pattern set (50+ patterns) scanned against a non-trivial text.
 * @details Stress-tests the equivalence invariant under a realistic load.
 *          Patterns are generated as short strings covering various prefixes
 *          to exercise dense trie branches and fail-link traversals.
 */
TEST_F(AhoCorasickTest, MatchEach_EqualsMatch_LargeSet)
{
    ACA ac;

    // Insert 52 patterns: "p0" through "p9", "q0" through "q9",
    // "ra" through "rz" (26 patterns), "s00" through "s15" (16 patterns).
    for (int i = 0; i < 10; ++i)
    {
        ac.insert("p" + std::to_string(i));
    }
    for (int i = 0; i < 10; ++i)
    {
        ac.insert("q" + std::to_string(i));
    }
    for (char ch = 'a'; ch <= 'z'; ++ch)
    {
        ac.insert(std::string("r") + ch);
    }
    for (int i = 0; i < 16; ++i)
    {
        ac.insert("s" + (i < 10 ? "0" + std::to_string(i) : std::to_string(i)));
    }

    ASSERT_EQ(ac.patternCount(), 62U);
    ac.build();

    // A text that triggers many overlapping matches
    const std::string text =
        "p0 q1 ra rb rc s01 s02 s15 "
        "p5 p9 rj rk s00 "
        "q3 q7 rd re rf rg s10 s11 "
        "extra noise with no match "
        "p1 p2 p3 q0 q2 rn ro rp s03 s04 s05";

    assertMatchEachEqualsMatch(ac, text);
}

// ══════════════════════════════════════════════════════════════════════════
//  11. matchEach callback accumulation  (2 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief matchEach results collected into a vector match() results exactly.
 * @details Uses collectMatchEach helper to gather callback results and
 *          compares them with the vector returned by match().
 */
TEST_F(AhoCorasickTest, MatchEach_CollectInVector_EqualsMatch)
{
    ACA ac;
    insertPatterns(ac, {"the", "there", "their", "they", "them"});
    ac.build();

    const auto cbResults = collectMatchEach(ac, "there they are with them");
    const auto directResults = ac.match("there they are with them");

    // Compare as sets (order-insensitive within the same position)
    std::set<MatchTriple> cbSet;
    for (const auto& r : cbResults)
    {
        cbSet.emplace(r.position, r.length, r.patternIndex);
    }

    std::set<MatchTriple> directSet;
    for (const auto& r : directResults)
    {
        directSet.emplace(r.position, r.length, r.patternIndex);
    }

    EXPECT_EQ(cbSet, directSet);
}

/**
 * @brief Count matches via callback and verify against match().size().
 * @details Demonstrates the zero-allocation counting use case �?the callback
 *          increments a counter without constructing a result vector.
 */
TEST_F(AhoCorasickTest, MatchEach_CountMatches)
{
    ACA ac;
    insertPatterns(ac, {"a", "aa", "aaa"});
    ac.build();

    std::size_t count = 0;
    ac.matchEach("aaaa", [&](const MatchResult&) { ++count; });

    const auto results = ac.match("aaaa");
    EXPECT_EQ(count, results.size());
    // Known: "aaaa" with patterns {a, aa, aaa} yields 4 + 3 + 2 = 9 matches
    EXPECT_EQ(count, 9U);
}

// ══════════════════════════════════════════════════════════════════════════
//  12. Zero-backtracking correctness regression  (2 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief fail_[s] > s scenario: patterns {"ab", "b"}.
 * @details In this case:
 *          - State chain: 0�?'a')�?�?'b')�?
 *          - New state:   0�?'b')�?
 *          - fail_[2] = 3, and 3 > 2 (the fail state has a higher index).
 *          This exercises the completed transition table path where the
 *          fail-link target lies *ahead* of the source state in the linear
 *          state array, a subtle edge case in the BFS completion logic.
 *
 *          Expected matches for text "ab":
 *            - "ab" at position 0 (patternIndex 0)
 *            - "b"  at position 1 (patternIndex 1)
 */
TEST_F(AhoCorasickTest, ZeroBacktracking_FailGtState)
{
    ACA ac;
    ac.insert("ab");  // state 0�?'a')�?�?'b')�?
    ac.insert("b");   // state 0�?'b')�?
    ac.build();

    // Verify matchEach and match both agree
    assertMatchEachEqualsMatch(ac, "ab");

    // Additional explicit check on the match results
    const auto results = ac.match("ab");
    ASSERT_EQ(results.size(), 2U);

    std::set<MatchTriple> triples;
    for (const auto& r : results)
    {
        triples.emplace(r.position, r.length, r.patternIndex);
    }
    EXPECT_TRUE(triples.contains({0, 2, 0}));  // "ab"
    EXPECT_TRUE(triples.contains({1, 1, 1}));  // "b"
}

/**
 * @brief Many states (500+) exercise the large completed transition table.
 * @details Builds a chain of 502 states (root + 501 pattern-extension states)
 *          by inserting patterns "a", "aa", "aaa", ..., up to a 501-character
 *          string of all 'a's.  Scans a moderately long text to verify that
 *          the large trans_ table does not cause crashes, memory corruption,
 *          or incorrect matches.
 *
 *          The completed transition table size = 502 × 256 �?128K entries
 *          (~512 KiB for int32_t), which is well within reasonable bounds
 *          but exercises allocator and cache behaviour.
 */
TEST_F(AhoCorasickTest, ZeroBacktracking_ManyStates)
{
    ACA ac;

    // Build a chain of states: "a", "aa", "aaa", ... up to 501 characters
    constexpr int MAX_LEN = 501;
    std::string pat;
    for (int len = 1; len <= MAX_LEN; ++len)
    {
        pat.push_back('a');
        ac.insert(pat);
    }

    ASSERT_EQ(ac.patternCount(), static_cast<std::size_t>(MAX_LEN));
    ASSERT_NO_THROW(ac.build());

    // Match against a text containing runs of 'a's of varying lengths
    const std::string text =
        "aaa b aaaaa x aa y aaaa z "
        "aaaaaaaa aaaaaaa aaaaaa aaaaa aaaa aaa aa a";

    // matchEach must not crash and must produce the same results as match()
    assertMatchEachEqualsMatch(ac, text);

    // Also verify that the total match count is consistent
    const auto results = ac.match(text);
    std::size_t cbCount = 0;
    ac.matchEach(text, [&](const MatchResult&) { ++cbCount; });
    EXPECT_EQ(cbCount, results.size());
}

// ══════════════════════════════════════════════════════════════════════════
//  13. Thread Safety  (3 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Multiple threads call match() concurrently with no data race.
 * @details Inserts 10 patterns, builds the automaton, then spawns 8 threads
 *          each calling match() 100 times on the same instance.  Verifies
 *          that all threads produce identical match results (same number of
 *          matches, same position/length/patternIndex for every match).
 *          This validates the ADR-004 guarantee that match() is read-only
 *          and thread-safe after build() has completed.
 *
 * @par Threads  8
 * @par Iterations  100 per thread
 * @par Patterns  {"alpha", "beta", "gamma", "delta", "epsilon",
 *                 "zeta", "eta", "theta", "iota", "kappa"}
 * @par Text  "alpha beta gamma delta epsilon zeta eta theta iota kappa"
 */
TEST_F(AhoCorasickTest, ThreadSafety_ConcurrentMatch_NoDataRace)
{
    ACA ac;
    insertPatterns(ac, {"alpha", "beta", "gamma", "delta", "epsilon",
                        "zeta", "eta", "theta", "iota", "kappa"});
    ac.build();

    constexpr int NUM_THREADS = 8;
    constexpr int ITERATIONS = 100;
    const std::string text =
        "alpha beta gamma delta epsilon zeta eta theta iota kappa";

    std::vector<std::vector<MatchResult>> threadResults(NUM_THREADS);
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < ITERATIONS; ++i)
            {
                auto results = ac.match(text);
                if (i == 0)  // save first iteration for cross-thread comparison
                {
                    threadResults[t] = std::move(results);
                }
            }
        });
    }

    for (auto& th : threads) th.join();

    // All threads must see identical match sets
    for (int t = 1; t < NUM_THREADS; ++t)
    {
        ASSERT_EQ(threadResults[0].size(), threadResults[t].size());
        for (std::size_t j = 0; j < threadResults[0].size(); ++j)
        {
            EXPECT_EQ(threadResults[0][j].position, threadResults[t][j].position);
            EXPECT_EQ(threadResults[0][j].length, threadResults[t][j].length);
            EXPECT_EQ(threadResults[0][j].patternIndex, threadResults[t][j].patternIndex);
        }
    }
}

/**
 * @brief Multiple threads call matchEach() concurrently with no data race.
 * @details Same structure as ThreadSafety_ConcurrentMatch_NoDataRace but
 *          uses the callback-based matchEach() API.  Each thread collects
 *          results via a thread-local callback into a local vector.
 *          Verifies that all threads see identical match sets, confirming
 *          that the callback-based hot path is also thread-safe.
 *
 * @par Threads  8
 * @par Iterations  100 per thread
 * @par Patterns  Same 10 words as the match() concurrency test
 */
TEST_F(AhoCorasickTest, ThreadSafety_ConcurrentMatchEach_NoDataRace)
{
    ACA ac;
    insertPatterns(ac, {"alpha", "beta", "gamma", "delta", "epsilon",
                        "zeta", "eta", "theta", "iota", "kappa"});
    ac.build();

    constexpr int NUM_THREADS = 8;
    constexpr int ITERATIONS = 100;
    const std::string text =
        "alpha beta gamma delta epsilon zeta eta theta iota kappa";

    std::vector<std::vector<MatchResult>> threadResults(NUM_THREADS);
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < ITERATIONS; ++i)
            {
                std::vector<MatchResult> local;
                ac.matchEach(text, [&](const MatchResult& m) { local.push_back(m); });
                if (i == 0)
                {
                    threadResults[t] = std::move(local);
                }
            }
        });
    }

    for (auto& th : threads) th.join();

    // All threads must see identical match sets
    for (int t = 1; t < NUM_THREADS; ++t)
    {
        ASSERT_EQ(threadResults[0].size(), threadResults[t].size());
        for (std::size_t j = 0; j < threadResults[0].size(); ++j)
        {
            EXPECT_EQ(threadResults[0][j].position, threadResults[t][j].position);
            EXPECT_EQ(threadResults[0][j].length, threadResults[t][j].length);
            EXPECT_EQ(threadResults[0][j].patternIndex, threadResults[t][j].patternIndex);
        }
    }
}

/**
 * @brief Mixed concurrent usage of match() and matchEach().
 * @details Even-numbered threads call match() while odd-numbered threads
 *          call matchEach().  All threads verify that their match count
 *          is identical, confirming that the two read-only paths do not
 *          interfere with each other under concurrent load.
 *
 * @par Threads  8 (4 match + 4 matchEach)
 * @par Iterations  100 per thread
 * @par Patterns  Same 10 words as the other concurrency tests
 */
TEST_F(AhoCorasickTest, ThreadSafety_MixedMatchAndMatchEach)
{
    ACA ac;
    insertPatterns(ac, {"alpha", "beta", "gamma", "delta", "epsilon",
                        "zeta", "eta", "theta", "iota", "kappa"});
    ac.build();

    constexpr int NUM_THREADS = 8;
    constexpr int ITERATIONS = 100;
    const std::string text =
        "alpha beta gamma delta epsilon zeta eta theta iota kappa";

    std::vector<std::size_t> matchCounts(NUM_THREADS);
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t)
    {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < ITERATIONS; ++i)
            {
                if (t % 2 == 0)
                {
                    // Even-indexed threads use match()
                    const auto results = ac.match(text);
                    if (i == 0)
                    {
                        matchCounts[t] = results.size();
                    }
                }
                else
                {
                    // Odd-indexed threads use matchEach()
                    std::size_t count = 0;
                    ac.matchEach(text, [&](const MatchResult&) { ++count; });
                    if (i == 0)
                    {
                        matchCounts[t] = count;
                    }
                }
            }
        });
    }

    for (auto& th : threads) th.join();

    // Every thread must report the same total match count, regardless
    // of whether it used match() or matchEach().
    for (int t = 1; t < NUM_THREADS; ++t)
    {
        EXPECT_EQ(matchCounts[0], matchCounts[t]);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  14. Edge Cases  (2 tests)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Insert after build() throws and leaves match state unchanged.
 * @details After building, the automaton is in matching mode.  This test
 *          verifies that:
 *          1. Calling insert() after build() throws std::runtime_error.
 *          2. The automaton's match results are unaffected by the failed
 *             insertion �?i.e. the automaton state is rolled back or
 *             unchanged.
 *
 *          This differs from the existing Insert_AfterBuild_Throws test
 *          (which only checks the exception) by additionally verifying
 *          that the reading side (match) produces identical results
 *          before and after the failed insertion.
 */
TEST_F(AhoCorasickTest, EdgeCase_InsertAfterBuild_ThrowStateUnchanged)
{
    ACA ac;
    insertPatterns(ac, {"alpha", "beta", "gamma"});
    ac.build();

    const std::string text = "alpha beta gamma delta";

    // Capture baseline match results
    const auto baseline = ac.match(text);

    // Attempt insert after build �?must throw
    EXPECT_THROW(ac.insert("delta"), std::runtime_error);

    // Verify that the automaton state is unchanged �?match results
    // must be identical to the baseline.
    const auto after = ac.match(text);

    ASSERT_EQ(baseline.size(), after.size());
    for (std::size_t i = 0; i < baseline.size(); ++i)
    {
        EXPECT_EQ(baseline[i].position, after[i].position);
        EXPECT_EQ(baseline[i].length, after[i].length);
        EXPECT_EQ(baseline[i].patternIndex, after[i].patternIndex);
    }
}

/**
 * @brief Calling build() twice leaves the transition table stable.
 * @details build() has an early-return guard (`if (built_) return;`),
 *          so a second call is a no-op.  This test explicitly verifies
 *          that:
 *          1. Build �?match produces a known result.
 *          2. A second build() call does not corrupt the transition
 *             table �?match after the second build() produces the
 *             same result.
 *
 *          This complements the existing Boundary_BuildIdempotent test
 *          by using a different pattern set and exercising a larger
 *          automaton (10 patterns instead of 3), which stresses the
 *          SoA output arrays across the second build() call.
 */
TEST_F(AhoCorasickTest, EdgeCase_BuildTwice_TransTableStable)
{
    ACA ac;
    insertPatterns(ac, {"alpha", "beta", "gamma", "delta", "epsilon",
                        "zeta", "eta", "theta", "iota", "kappa"});
    ac.build();

    // First match �?establish a baseline
    const std::string text =
        "alpha beta gamma delta epsilon zeta eta theta iota kappa";
    const auto firstBaseline = ac.match(text);

    // Call build() a second time (should be a no-op)
    ac.build();

    // Second match �?must be identical to the baseline
    const auto secondBaseline = ac.match(text);

    ASSERT_EQ(firstBaseline.size(), secondBaseline.size());
    for (std::size_t i = 0; i < firstBaseline.size(); ++i)
    {
        EXPECT_EQ(firstBaseline[i].position, secondBaseline[i].position);
        EXPECT_EQ(firstBaseline[i].length, secondBaseline[i].length);
        EXPECT_EQ(firstBaseline[i].patternIndex, secondBaseline[i].patternIndex);
    }
}
