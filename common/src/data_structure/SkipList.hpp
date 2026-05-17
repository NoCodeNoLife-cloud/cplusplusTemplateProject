/**
 * @file SkipList.hpp
 * @brief SkipList class declaration
 * @details This header defines the SkipList class that provides functionality for Advanced data structures including trees and skip lists.
 */

#pragma once
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

namespace common::common::data_structure::tree {
/**
 * @class   SkipList
 * @tparam  T The type of elements stored in the skip list.
 *            T must be LessThanComparable (operator< defined).
 * @brief   A probabilistic data structure providing expected O(log n)
 *          complexity for search, insertion, and deletion.
 *
 * @par Thread Safety
 * This class is **not** thread-safe. External synchronization is required
 * for concurrent access.
 *
 * @par Complexity Guarantees
 * - Search:  Average @f$ O(\log n) @f$, Worst @f$ O(n) @f$
 * - Insert:  Average @f$ O(\log n) @f$, Worst @f$ O(n) @f$
 * - Erase:   Average @f$ O(\log n) @f$, Worst @f$ O(n) @f$
 * - Memory:  Expected @f$ O(n) @f$ (each node has expected 2 pointers)
 */
template <typename T>
class SkipList {
public:
    /**
     * @brief Constructs an empty SkipList.
     *
     * @param[in] maxLevel Maximum allowed tower height (default 16).
     *                     This limits the maximum number of pointer levels.
     * @param[in] p        Probability of level promotion (default 0.5).
     *
     * @pre @p maxLevel >= 1
     * @pre 0.0 < @p p < 1.0
     */
    explicit SkipList(int maxLevel = 16, float p = 0.5f) : maxLevel_(maxLevel),
                                                           probability_(p),
                                                           header_(new Node(T{}, maxLevel)),
                                                           currentLevel_(1),
                                                           size_(0),
                                                           gen_(rd_()),
                                                           dis_(0.0f, 1.0f) {
    }

    /**
     * @brief Destructor.
     *
     * Traverses the bottom-level linked list and deletes all nodes.
     *
     * @complexity O(n)
     */
    ~SkipList() {
        Node* current = header_;
        while (current != nullptr) {
            Node* next = current->forward[0];
            delete current;
            current = next;
        }
    }

    /** @name Disable Copy Semantics */
    /**@{*/
    SkipList(const SkipList&) = delete;

    SkipList& operator=(const SkipList&) = delete;

    /**@}*/

    /** @name Move Semantics (Defaulted) */
    /**@{*/
    SkipList(SkipList&&) = default;

    SkipList& operator=(SkipList&&) = default;

    /**@}*/

    /**
     * @brief Searches for a key in the skip list.
     *
     * @param[in] key The value to search for.
     * @return @c true if the key exists, @c false otherwise.
     *
     * @complexity Average @f$ O(\log n) @f$, Worst @f$ O(n) @f$
     */
    bool search(const T& key) const {
        Node* current = header_;

        // Traverse from top level down to level 0
        for (int i = currentLevel_ - 1; i >= 0; --i) {
            while (current->forward[i] != nullptr && current->forward[i]->key < key) {
                current = current->forward[i];
            }
        }

        current = current->forward[0];
        return current != nullptr && current->key == key;
    }

    /**
     * @brief Inserts a key into the skip list.
     *
     * If the key already exists, this operation has no effect (idempotent insert).
     *
     * @param[in] key The value to insert.
     *
     * @post If the key did not exist, @c size() is incremented by 1.
     * @complexity Average @f$ O(\log n) @f$, Worst @f$ O(n) @f$
     */
    void insert(const T& key) {
        std::vector<Node*> update(maxLevel_, nullptr);
        Node* current = header_;

        // Find predecessor at each level
        for (int i = currentLevel_ - 1; i >= 0; --i) {
            while (current->forward[i] != nullptr && current->forward[i]->key < key) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        current = current->forward[0];

        // Duplicate key: do nothing (or could update associated value)
        if (current != nullptr && current->key == key) {
            return;
        }

        // Generate random level and expand list if necessary
        int newLevel = randomLevel();
        if (newLevel > currentLevel_) {
            for (int i = currentLevel_; i < newLevel; ++i) {
                update[i] = header_;
            }
            currentLevel_ = newLevel;
        }

        // Create node and splice into all relevant levels
        Node* newNode = new Node(key, newLevel);
        for (int i = 0; i < newLevel; ++i) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
        ++size_;
    }

    /**
     * @brief Removes a key from the skip list.
     *
     * @param[in] key The value to erase.
     * @return @c true if the key was found and removed, @c false if absent.
     *
     * @post If the key existed, @c size() is decremented by 1.
     * @complexity Average @f$ O(\log n) @f$, Worst @f$ O(n) @f$
     */
    bool erase(const T& key) {
        std::vector<Node*> update(maxLevel_, nullptr);
        Node* current = header_;

        for (int i = currentLevel_ - 1; i >= 0; --i) {
            while (current->forward[i] != nullptr && current->forward[i]->key < key) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        current = current->forward[0];

        // Key not found
        if (current == nullptr || current->key != key) {
            return false;
        }

        // Unlink from all levels that contain the target node
        for (int i = 0; i < currentLevel_; ++i) {
            if (update[i]->forward[i] != current) {
                break;
            }
            update[i]->forward[i] = current->forward[i];
        }
        delete current;
        --size_;

        // Shrink currentLevel_ if top levels became empty
        while (currentLevel_ > 1 && header_->forward[currentLevel_ - 1] == nullptr) {
            --currentLevel_;
        }
        return true;
    }

    /**
     * @brief Checks whether the container is empty.
     * @return @c true if no elements are stored.
     */
    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    /**
     * @brief Returns the number of elements stored.
     * @return The element count.
     */
    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    /**
     * @brief Returns all elements in sorted order as a vector.
     *
     * @return Vector containing all elements in ascending order.
     */
    [[nodiscard]] std::vector<T> toVector() const {
        std::vector<T> result;
        Node* current = header_->forward[0];
        while (current != nullptr) {
            result.push_back(current->key);
            current = current->forward[0];
        }
        return result;
    }

private:
    /**
     * @brief Internal node structure representing an element at a given tower height.
     */
    struct Node {
        T key;                      ///< The stored key value.
        std::vector<Node*> forward; ///< Forward pointers indexed by level.

        /**
         * @brief Constructs a node.
         * @param[in] k The key to store.
         * @param[in] level The height of the node tower (number of levels).
         */
        Node(const T& k, int level) : key(k), forward(level, nullptr) {
        }
    };

    const int maxLevel_;      ///< Absolute maximum height allowed.
    const float probability_; ///< Promotion probability per level (typically 0.5).
    Node* const header_;      ///< Sentinel head node spanning all levels.
    int currentLevel_;        ///< Current highest level in use (1-based).
    std::size_t size_;        ///< Number of elements stored.

    // Random number generation state
    std::random_device rd_;                     ///< Non-deterministic seed source.
    std::mt19937 gen_;                          ///< Mersenne Twister engine.
    std::uniform_real_distribution<float> dis_; ///< Uniform [0.0, 1.0) distribution.

    /**
     * @brief Generates a random level for a new node.
     *
     * Each successive level is generated with probability @c probability_.
     * The expected height is @f$ \frac{1}{1-p} @f$.
     *
     * @return A level in the range [1, maxLevel_].
     */
    int randomLevel() {
        int level = 1;
        while (dis_(gen_) < probability_ && level < maxLevel_) {
            ++level;
        }
        return level;
    }
};
};