/**
 * @file AhoCorasick.cc
 * @brief AhoCorasick implementation — completed transition table (zero-backtracking)
 * @details Implements the IACAutomaton interface using a Trie with fail links.
 *          The build phase uses BFS to compute fail pointers, merge output lists,
 *          and then completes the transition table (@c trans_) so that every
 *          (state, byte) pair has a valid successor.  The match phase uses O(1)
 *          single-step transitions with no fail-link backtracking.
 *
 * @par Complexity
 * - Insert: O(p) per pattern, where p is the pattern length.
 * - Build:  O(N × Σ) where N is the total number of states and Σ = 256.
 * - Match:  O(n + m) where n is the text length and m is the total match
 *           count (no backtracking overhead).
 *
 * @par Thread Safety
 * - @c insert() and @c build() are **not** thread-safe; callers must
 *   serialise construction.
 * - @c match() and @c matchEach() are **read-only** after @c build() has
 *   completed and are safe for concurrent invocation from multiple threads.
 */

#include "data_structure/string/AhoCorasick.hpp"

#include <algorithm>
#include <cassert>
#include <queue>
#include <stdexcept>

namespace common::data_structure::string
{

// ══════════════════════════════════════════════════════════════════════════
//  Construction
// ══════════════════════════════════════════════════════════════════════════

AhoCorasick::AhoCorasick()
{
    // Allocate the root state (index 0)
    goto_.resize(ALPHABET_SIZE, NO_STATE);
    fail_.push_back(0);          // root's fail link points to itself
    outputPtr_.push_back(0);
    outputCnt_.push_back(0);
    output_.emplace_back();      // empty output list for root
}

// ══════════════════════════════════════════════════════════════════════════
//  IACAutomaton interface
// ══════════════════════════════════════════════════════════════════════════

void AhoCorasick::insert(std::string_view pattern)
{
    if (pattern.empty())
    {
        throw std::invalid_argument(
            "AhoCorasick::insert: empty pattern is not supported");
    }

    if (built_)
    {
        throw std::runtime_error(
            "AhoCorasick::insert: cannot insert after build() has been called");
    }

    std::size_t state = 0;

    for (const unsigned char c : pattern)
    {
        const auto idx = state * ALPHABET_SIZE + static_cast<std::size_t>(c);

        if (goto_[idx] == NO_STATE)
        {
            const auto newState = addState();
            goto_[idx] = static_cast<int32_t>(newState);
        }

        state = static_cast<std::size_t>(goto_[idx]);
    }

    // Record pattern at the terminal state.
    // Exception-safe write order: output_ first, then patterns_.  If
    // patterns_.emplace_back() throws, the output_ entry is rolled back.
    const auto patIdx = patterns_.size();
    try
    {
        output_[state].push_back(static_cast<int32_t>(patIdx));
        patterns_.emplace_back(pattern);
    }
    catch (...)
    {
        output_[state].pop_back();
        throw;
    }
}

void AhoCorasick::build()
{
    if (built_)
    {
        return;
    }

    const auto numStates = goto_.size() / ALPHABET_SIZE;

    // ── Step 1: Root self-loops ─────────────────────────────────────────
    // Fill every missing transition from the root with a self-loop (0).
    // This guarantees that the root never returns NO_STATE.
    for (std::size_t c = 0; c < ALPHABET_SIZE; ++c)
    {
        if (goto_[c] == NO_STATE)
        {
            goto_[c] = 0;
        }
    }

    // ── Step 2: BFS initialisation ──────────────────────────────────────
    // Enqueue root's children and set their fail links to root.
    std::queue<std::size_t> q;
    std::vector<std::size_t> bfsOrder;
    bfsOrder.reserve(numStates);

    for (std::size_t c = 0; c < ALPHABET_SIZE; ++c)
    {
        const auto child = static_cast<std::size_t>(goto_[c]);
        if (child != 0)
        {
            fail_[child] = 0;
            q.push(child);
        }
    }

    // ── Step 3: BFS loop (compute fail links and merge output) ──────────

    while (!q.empty())
    {
        const auto u = q.front();
        q.pop();
        bfsOrder.push_back(u);

        for (std::size_t c = 0; c < ALPHABET_SIZE; ++c)
        {
            const auto idx = u * ALPHABET_SIZE + c;
            const auto v = goto_[idx];

            if (v != NO_STATE)
            {
                // Walk up the fail chain to find the longest proper suffix
                // that has a transition on character c.
                auto f = static_cast<std::size_t>(fail_[u]);

                while (f != 0 && goto_[f * ALPHABET_SIZE + c] == NO_STATE)
                {
                    f = static_cast<std::size_t>(fail_[f]);
                }

                // fail_[v] = goto_[f][c]
                // Invariant: after Step 1 (root self-loops) and the while-loop,
                // goto_[f][c] is guaranteed to be a valid state (never NO_STATE).
                fail_[v] = goto_[f * ALPHABET_SIZE + c];

                // Merge output from the fail-link target
                mergeOutput(static_cast<std::size_t>(v),
                            static_cast<std::size_t>(fail_[v]));

                q.push(static_cast<std::size_t>(v));
            }
        }
    }

    // ── Step 4: Flatten temporary output into SoA arrays ─────────────────
    flattenOutput();

    // ── Step 5: Complete the transition table (zero-backtracking) ──────
    // For every state s and every byte c, compute the effective transition:
    //   if goto_[s*256 + c] != -1 → trans_ = goto_[s*256 + c]
    //   else → trans_ = trans_[fail_[s]*256 + c]  (recursive completion)
    //
    // States are processed in BFS order so that fail_[s] (which has strictly
    // smaller depth than s) is guaranteed to be already completed.
    //
    // Note: trans_ uses state-number-based indexing (state * 256 + byte).
    // resize + operator[] is preferred over reserve + push_back here because
    // it expresses the random-access write pattern directly (trans_[failBase + c]
    // reads a previously written row) without requiring the reader to verify
    // that the BFS write order coincides with the state-number order.
    // The double-write (zero-init then fill) is an acceptable cost for the
    // cache-friendly flat layout.

    trans_.resize(numStates * ALPHABET_SIZE);

    // Root's completed transitions: use goto_ if valid, else self-loop (0)
    for (std::size_t c = 0; c < ALPHABET_SIZE; ++c)
    {
        const auto g = goto_[c];
        trans_[c] = (g != NO_STATE) ? g : 0;
    }

    // Process remaining states in BFS order
    for (const auto s : bfsOrder)
    {
        const auto base = s * ALPHABET_SIZE;
        const auto failBase = static_cast<std::size_t>(fail_[s]) * ALPHABET_SIZE;

        for (std::size_t c = 0; c < ALPHABET_SIZE; ++c)
        {
            const auto g = goto_[base + c];
            trans_[base + c] = (g != NO_STATE) ? g : trans_[failBase + c];
        }
    }

    built_ = true;

    // Release goto_ — trans_ is now the sole transition table.
    { std::vector<int32_t> empty; goto_.swap(empty); }
}

auto AhoCorasick::patternCount() const noexcept -> std::size_t
{
    return patterns_.size();
}

auto AhoCorasick::match(std::string_view text) const -> std::vector<MatchResult>
{
    if (!built_)
    {
        throw std::runtime_error(
            "AhoCorasick::match: build() must be called before match()");
    }

    std::vector<MatchResult> results;
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
                results.push_back(MatchResult{
                    i - pat.size() + 1,
                    pat.size(),
                    patIdx
                });
            }
        }
    }

    return results;
}

auto AhoCorasick::getPattern(std::size_t index) const -> std::string_view
{
    return patterns_.at(index);
}

// ══════════════════════════════════════════════════════════════════════════
//  Internal helpers
// ══════════════════════════════════════════════════════════════════════════

auto AhoCorasick::addState() -> std::size_t
{
    const auto newState = goto_.size() / ALPHABET_SIZE;

    // Extend the flat goto table by one full row
    goto_.resize(goto_.size() + ALPHABET_SIZE, NO_STATE);

    // Append entries for the other SoA vectors
    fail_.push_back(NO_STATE);
    outputPtr_.push_back(0);
    outputCnt_.push_back(0);

    // Append an empty per-state output list
    output_.emplace_back();

    return newState;
}

void AhoCorasick::mergeOutput(std::size_t state, std::size_t failState)
{
    if (output_[failState].empty())
    {
        return;
    }

    auto& dst = output_[state];
    const auto& src = output_[failState];

    for (const auto patIdx : src)
    {
        // Linear dedup scan — output lists are typically small, so this is
        // acceptable for Task 1.  A more efficient approach (e.g. a bitset
        // or hash set) can be adopted in Task 2.
        bool found = false;
        for (const auto existing : dst)
        {
            if (existing == patIdx)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            dst.push_back(patIdx);
        }
    }
}

void AhoCorasick::flattenOutput()
{
    const auto numStates = output_.size();
    assert(outputPtr_.size() == numStates);
    assert(outputCnt_.size() == numStates);

    std::vector<int32_t> flat;

    for (std::size_t s = 0; s < numStates; ++s)
    {
        outputPtr_[s] = static_cast<int32_t>(flat.size());
        outputCnt_[s] = static_cast<int32_t>(output_[s].size());

        flat.insert(flat.end(),
                    output_[s].begin(),
                    output_[s].end());
    }

    outputData_ = std::move(flat);

    // Free temporary per-state storage
    output_.clear();
    output_.shrink_to_fit();
}

} // namespace common::data_structure::string
