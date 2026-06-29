/**
 * @file Trie.hpp
 * @brief Prefix tree (Trie) for efficient string-key storage and prefix matching
 * @details A trie (prefix tree) that stores strings by their constituent
 *          characters along a tree path.  Provides O(k) insert, search, and
 *          prefix-match operations where k is the key length â€?independent of
 *          the total number of stored keys.  Each node holds a pointer to
 *          an associated value type V (null if the path does not form a
 *          complete key).
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - insert / search / erase: O(k) where k = key length
 * - startsWith / keysWithPrefix: O(k + m) where m = number of matching keys
 *
 * @par Memory
 * Each character in each unique key path allocates one node.
 */

#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cppforge::data_structure::tree::multiway
{
    /**
     * @class   Trie
     * @tparam  T The type of values associated with keys.
     *            T must be default-constructible.
     * @brief   A prefix tree (trie) data structure for string keys
     *          with associated values.
     *
     * Provides O(k) average-case complexity for insertion, search,
     * prefix queries, and deletion, where k is the length of the key.
     *
     * @par Thread Safety
     * This class is **not** thread-safe. External synchronization is required
     * for concurrent access.
     *
     * @par Complexity Guarantees
     * - Insert: Average @f$ O(k) @f$
     * - Find:   Average @f$ O(k) @f$
     * - Erase:  Average @f$ O(k) @f$
     * - Prefix: @f$ O(m + n) @f$ where @f$ m @f$ = prefix length,
     *           @f$ n @f$ = number of matching keys
     * - Memory: @f$ O(N \times K) @f$ where @f$ N @f$ = number of keys,
     *           @f$ K @f$ = average key length
     */
    template <typename T>
    class Trie
    {
    public:
        /**
         * @brief Constructs an empty Trie.
         *
         * @post empty() == true
         */
        Trie();

        /** @brief Default destructor. */
        ~Trie() = default;

        /** @name Disable Copy Semantics */
        /**@{*/
        Trie(const Trie&) = delete;

        Trie& operator=(const Trie&) = delete;
        /**@}*/

        /** @name Move Semantics */
        /**@{*/
        Trie(Trie&& other) noexcept
            : root_(std::move(other.root_)), size_(other.size_)
        {
            other.size_ = 0;
        }

        Trie& operator=(Trie&& other) noexcept
        {
            if (this != &other)
            {
                root_ = std::move(other.root_);
                size_ = other.size_;
                other.size_ = 0;
            }
            return *this;
        }
        /**@}*/

        /**
         * @brief Inserts a key-value pair into the trie.
         *
         * If the key already exists, its value is updated and the size is
         * unchanged. Otherwise a new entry is created.
         *
         * @param[in] key   The string key to insert.
         * @param[in] value The value to associate with the key.
         *
         * @post If the key did not exist, @c size() is incremented by 1.
         * @complexity O(k) where k = key.length()
         */
        void insert(const std::string& key, const T& value);

        /**
         * @brief Inserts a key-value pair (move semantics).
         *
         * @param[in] key   The string key to insert.
         * @param[in] value The value to move-associate with the key.
         *
         * @post If the key did not exist, @c size() is incremented by 1.
         * @complexity O(k) where k = key.length()
         */
        void insert(const std::string& key, T&& value);

        /**
         * @brief Finds the value associated with a key (mutable).
         *
         * @param[in] key The string key to search for.
         * @return Pointer to the associated value if the key exists,
         *         @c nullptr otherwise.
         *
         * @complexity O(k) where k = key.length()
         */
        T* find(const std::string& key);

        /**
         * @brief Finds the value associated with a key (const).
         *
         * @param[in] key The string key to search for.
         * @return Pointer to the associated value if the key exists,
         *         @c nullptr otherwise.
         *
         * @complexity O(k) where k = key.length()
         */
        [[nodiscard]] const T* find(const std::string& key) const;

        /**
         * @brief Checks whether a key exists in the trie.
         *
         * @param[in] key The string key to check.
         * @return @c true if the key exists, @c false otherwise.
         *
         * @complexity O(k) where k = key.length()
         */
        [[nodiscard]] bool contains(const std::string& key) const;

        /**
         * @brief Removes a key and its associated value from the trie.
         *
         * Unreachable nodes are automatically pruned to conserve memory.
         *
         * @param[in] key The string key to erase.
         * @return @c true if the key was found and removed, @c false if absent.
         *
         * @post If the key existed, @c size() is decremented by 1.
         * @complexity O(k) where k = key.length()
         */
        bool erase(const std::string& key);

        /**
         * @brief Finds all stored keys that start with a given prefix.
         *
         * @param[in] prefix The prefix to match.
         * @return A vector containing every stored key beginning with
         *         @p prefix, in lexicographic order.
         *
         * @complexity O(m + n) where m = prefix.length(),
         *             n = number of matching keys
         */
        [[nodiscard]] std::vector<std::string> startsWith(const std::string& prefix) const;

        /**
         * @brief Returns all stored keys.
         *
         * @return A vector containing every key, in lexicographic order.
         *
         * @complexity O(N) where N = number of keys
         */
        [[nodiscard]] std::vector<std::string> keys() const;

        /**
         * @brief Returns all stored values.
         *
         * @return A vector containing every value, ordered by key
         *         (lexicographic traversal).
         *
         * @complexity O(N x K) where N = number of keys, K = avg key length
         */
        [[nodiscard]] std::vector<T> values() const;

        /**
         * @brief Checks whether the trie is empty.
         *
         * @return @c true if no keys are stored, @c false otherwise.
         */
        [[nodiscard]] bool empty() const;

        /**
         * @brief Returns the number of keys stored in the trie.
         *
         * @return The element count.
         */
        [[nodiscard]] std::size_t size() const;

        /**
         * @brief Removes all keys from the trie.
         *
         * @post empty() == true
         * @complexity O(1) (replaces the root node)
         */
        void clear();

    private:
        /**
         * @brief Internal node structure representing a single character level.
         */
        struct Node
        {
            std::unordered_map<char, std::unique_ptr<Node>> children;
            T value{};
            bool isEnd{false};
        };

        std::unique_ptr<Node> root_;
        std::size_t size_;

        /**
         * @brief Recursive erase helper.
         *
         * @param[in,out] node  The current node being examined.
         * @param[in]     key   The full key being erased.
         * @param[in]     depth The current depth (character index) in the key.
         * @return @c true if the caller should prune this child (no longer
         *         needed), @c false otherwise.
         */
        bool eraseHelper(Node* node, const std::string& key, std::size_t depth);

        /**
         * @brief Recursively collects all complete keys under a node.
         *
         * @param[in]     node   The node to start collecting from.
         * @param[in,out] prefix The accumulated prefix string.
         * @param[out]    result The output vector of collected keys.
         */
        void collectKeys(const Node* node,
                         std::string& prefix,
                         std::vector<std::string>& result) const;
    };

    // =========================================================================
    //  Template Implementation
    // =========================================================================

    template <typename T>
    Trie<T>::Trie() : root_(std::make_unique<Node>()), size_(0)
    {
    }

    template <typename T>
    void Trie<T>::insert(const std::string& key, const T& value)
    {
        Node* node = root_.get();

        for (const char c : key)
        {
            auto& child = node->children[c];
            if (!child)
            {
                child = std::make_unique<Node>();
            }
            node = child.get();
        }

        if (!node->isEnd)
        {
            node->isEnd = true;
            ++size_;
        }
        node->value = value;
    }

    template <typename T>
    void Trie<T>::insert(const std::string& key, T&& value)
    {
        Node* node = root_.get();

        for (const char c : key)
        {
            auto& child = node->children[c];
            if (!child)
            {
                child = std::make_unique<Node>();
            }
            node = child.get();
        }

        if (!node->isEnd)
        {
            node->isEnd = true;
            ++size_;
        }
        node->value = std::move(value);
    }

    template <typename T>
    T* Trie<T>::find(const std::string& key)
    {
        Node* node = root_.get();

        for (const char c : key)
        {
            const auto it = node->children.find(c);
            if (it == node->children.end())
            {
                return nullptr;
            }
            node = it->second.get();
        }

        return node->isEnd ? &node->value : nullptr;
    }

    template <typename T>
    const T* Trie<T>::find(const std::string& key) const
    {
        const Node* node = root_.get();

        for (const char c : key)
        {
            const auto it = node->children.find(c);
            if (it == node->children.end())
            {
                return nullptr;
            }
            node = it->second.get();
        }

        return node->isEnd ? &node->value : nullptr;
    }

    template <typename T>
    bool Trie<T>::contains(const std::string& key) const
    {
        return find(key) != nullptr;
    }

    template <typename T>
    bool Trie<T>::erase(const std::string& key)
    {
        if (!contains(key))
        {
            return false;
        }
        eraseHelper(root_.get(), key, 0);
        return true;
    }

    template <typename T>
    bool Trie<T>::eraseHelper(Node* node,
                              const std::string& key,
                              const std::size_t depth)
    {
        if (depth == key.size())
        {
            if (!node->isEnd)
            {
                return false;
            }

            node->isEnd = false;
            --size_;
            return node->children.empty();
        }

        const char c = key[depth];
        const auto it = node->children.find(c);

        if (it == node->children.end())
        {
            return false;
        }

        const bool shouldPruneChild = eraseHelper(it->second.get(), key, depth + 1);

        if (shouldPruneChild)
        {
            node->children.erase(it);
            return !node->isEnd && node->children.empty();
        }

        return false;
    }

    template <typename T>
    std::vector<std::string> Trie<T>::startsWith(const std::string& prefix) const
    {
        const Node* node = root_.get();

        for (const char c : prefix)
        {
            const auto it = node->children.find(c);
            if (it == node->children.end())
            {
                return {};
            }
            node = it->second.get();
        }

        std::vector<std::string> result;
        std::string current = prefix;
        collectKeys(node, current, result);
        return result;
    }

    template <typename T>
    std::vector<std::string> Trie<T>::keys() const
    {
        std::vector<std::string> result;
        std::string prefix;
        collectKeys(root_.get(), prefix, result);
        return result;
    }

    template <typename T>
    std::vector<T> Trie<T>::values() const
    {
        const auto keyList = keys();
        std::vector<T> result;
        result.reserve(keyList.size());

        for (const auto& key : keyList)
        {
            const T* val = find(key);
            if (val)
            {
                result.push_back(*val);
            }
        }

        return result;
    }

    template <typename T>
    void Trie<T>::collectKeys(const Node* node,
                              std::string& prefix,
                              std::vector<std::string>& result) const
    {
        if (node->isEnd)
        {
            result.push_back(prefix);
        }

        for (const auto& [c, child] : node->children)
        {
            prefix.push_back(c);
            collectKeys(child.get(), prefix, result);
            prefix.pop_back();
        }
    }

    template <typename T>
    bool Trie<T>::empty() const
    {
        return size_ == 0;
    }

    template <typename T>
    std::size_t Trie<T>::size() const
    {
        return size_;
    }

    template <typename T>
    void Trie<T>::clear()
    {
        root_ = std::make_unique<Node>();
        size_ = 0;
    }
}
