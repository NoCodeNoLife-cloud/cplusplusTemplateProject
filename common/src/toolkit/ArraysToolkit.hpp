#pragma once
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <numeric>
#include <functional>
#include <set>
#include <random>

namespace common::toolkit {
    /// @brief Utility class for array operations.
    /// @details This class provides static methods to perform various operations on arrays,
    ///          such as converting to a vector, binary search, copying, comparison, filling,
    ///          sorting, and generating string representations.
    class ArraysToolkit {
    public:
        /// @brief Converts an array to a vector.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @return A vector containing the elements of the array.
        template<typename T>
        [[nodiscard]] static auto asList(const T *array, size_t size) -> std::vector<T>;

        /// @brief Searches for a key in a sorted array using binary search.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the sorted array.
        /// @param size Size of the array.
        /// @param key The value to search for.
        /// @return Index of the key if found, otherwise -1.
        template<typename T>
        [[nodiscard]] static auto binarySearch(const T *array, size_t size, const T &key) -> int32_t;

        /// @brief Searches for a key in a sorted subarray using binary search.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the sorted array.
        /// @param fromIndex Starting index of the subarray (inclusive).
        /// @param toIndex Ending index of the subarray (exclusive).
        /// @param key The value to search for.
        /// @return Index of the key if found, otherwise -1.
        /// @throws std::out_of_range If fromIndex >= toIndex
        template<typename T>
        [[nodiscard]] static auto binarySearch(const T *array, size_t fromIndex, size_t toIndex, const T &key) -> int32_t;

        /// @brief Copies an array to a new vector of specified length.
        /// @tparam T The type of elements in the array.
        /// @param original Pointer to the original array.
        /// @param originalSize Size of the original array.
        /// @param newLength Length of the new vector.
        /// @return A new vector with the copied elements.
        template<typename T>
        [[nodiscard]] static auto copyOf(const T *original, size_t originalSize, size_t newLength) -> std::vector<T>;

        /// @brief Copies a range of an array to a new vector.
        /// @tparam T The type of elements in the array.
        /// @param original Pointer to the original array.
        /// @param from Starting index (inclusive).
        /// @param to Ending index (exclusive).
        /// @return A new vector containing the specified range.
        /// @throws std::out_of_range If from > to
        template<typename T>
        [[nodiscard]] static auto copyOfRange(const T *original, size_t from, size_t to) -> std::vector<T>;

        /// @brief Checks if two arrays are equal.
        /// @tparam T The type of elements in the arrays.
        /// @param a Pointer to the first array.
        /// @param sizeA Size of the first array.
        /// @param b Pointer to the second array.
        /// @param sizeB Size of the second array.
        /// @return True if the arrays are equal, false otherwise.
        template<typename T>
        [[nodiscard]] static auto equals(const T *a, size_t sizeA, const T *b, size_t sizeB) -> bool;

        /// @brief Fills an array with a specific value.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param value The value to fill the array with.
        template<typename T>
        static auto fill(T *array, size_t size, const T &value) -> void;

        /// @brief Sorts an array in ascending order.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        template<typename T>
        static auto sort(T *array, size_t size) -> void;

        /// @brief Sorts a subarray in ascending order.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param fromIndex Starting index of the subarray (inclusive).
        /// @param toIndex Ending index of the subarray (exclusive).
        /// @throws std::out_of_range If fromIndex >= toIndex
        template<typename T>
        static auto sort(T *array, size_t fromIndex, size_t toIndex) -> void;

        /// @brief Converts an array to a string representation.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @return A string representation of the array.
        template<typename T>
        [[nodiscard]] static auto toString(const T *array, size_t size) -> std::string;

        /// @brief Checks if array is empty or null.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @return True if the array is empty or null, false otherwise.
        template<typename T>
        [[nodiscard]] static auto isEmpty(const T *array, size_t size) -> bool;

        /// @brief Checks if array contains a specific value.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param value The value to search for.
        /// @return True if the value is found, false otherwise.
        template<typename T>
        [[nodiscard]] static auto contains(const T *array, size_t size, const T &value) -> bool;

        /// @brief Performs linear search for a key in an array.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param key The value to search for.
        /// @return Index of the key if found, otherwise -1.
        template<typename T>
        [[nodiscard]] static auto linearSearch(const T *array, size_t size, const T &key) -> int32_t;

        /// @brief Finds the index of the maximum element in the array.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @return Index of the maximum element, or -1 if array is empty.
        template<typename T>
        [[nodiscard]] static auto maxElement(const T *array, size_t size) -> int32_t;

        /// @brief Finds the index of the minimum element in the array.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @return Index of the minimum element, or -1 if array is empty.
        template<typename T>
        [[nodiscard]] static auto minElement(const T *array, size_t size) -> int32_t;

        /// @brief Counts occurrences of a value in the array.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param value The value to count.
        /// @return Number of occurrences of the value.
        template<typename T>
        [[nodiscard]] static auto count(const T *array, size_t size, const T &value) -> size_t;

        /// @brief Reverses the array in place.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        template<typename T>
        static auto reverse(T *array, size_t size) -> void;

        /// @brief Reverses a range of the array in place.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param fromIndex Starting index (inclusive).
        /// @param toIndex Ending index (exclusive).
        /// @throws std::out_of_range If fromIndex >= toIndex
        template<typename T>
        static auto reverse(T *array, size_t fromIndex, size_t toIndex) -> void;

        /// @brief Removes duplicates and returns a new sorted vector with unique elements.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @return A vector with unique elements in sorted order.
        template<typename T>
        [[nodiscard]] static auto distinct(const T *array, size_t size) -> std::vector<T>;

        /// @brief Transforms array elements using a function.
        /// @tparam T The input element type.
        /// @tparam U The output element type.
        /// @tparam Func The transformation function type.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param func Transformation function.
        /// @return A vector with transformed elements.
        template<typename T, typename U, typename Func>
        [[nodiscard]] static auto map(const T *array, size_t size, Func func) -> std::vector<U>;

        /// @brief Filters array elements based on a predicate.
        /// @tparam T The type of elements in the array.
        /// @tparam Predicate The predicate function type.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param predicate Filter predicate.
        /// @return A vector with elements that satisfy the predicate.
        template<typename T, typename Predicate>
        [[nodiscard]] static auto filter(const T *array, size_t size, Predicate predicate) -> std::vector<T>;

        /// @brief Calculates the sum of all elements in the array.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @return Sum of all elements.
        template<typename T>
        [[nodiscard]] static auto sum(const T *array, size_t size) -> T;

        /// @brief Calculates the average of all elements in the array.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @return Average of all elements.
        /// @throws std::invalid_argument If array is empty
        template<typename T>
        [[nodiscard]] static auto average(const T *array, size_t size) -> double;

        /// @brief Checks if all elements satisfy a predicate.
        /// @tparam T The type of elements in the array.
        /// @tparam Predicate The predicate function type.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param predicate Condition to check.
        /// @return True if all elements satisfy the predicate, false otherwise.
        template<typename T, typename Predicate>
        [[nodiscard]] static auto allMatch(const T *array, size_t size, Predicate predicate) -> bool;

        /// @brief Checks if any element satisfies a predicate.
        /// @tparam T The type of elements in the array.
        /// @tparam Predicate The predicate function type.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param predicate Condition to check.
        /// @return True if any element satisfies the predicate, false otherwise.
        template<typename T, typename Predicate>
        [[nodiscard]] static auto anyMatch(const T *array, size_t size, Predicate predicate) -> bool;

        /// @brief Checks if no element satisfies a predicate.
        /// @tparam T The type of elements in the array.
        /// @tparam Predicate The predicate function type.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param predicate Condition to check.
        /// @return True if no element satisfies the predicate, false otherwise.
        template<typename T, typename Predicate>
        [[nodiscard]] static auto noneMatch(const T *array, size_t size, Predicate predicate) -> bool;

        /// @brief Concatenates two arrays into a new vector.
        /// @tparam T The type of elements in the arrays.
        /// @param a Pointer to the first array.
        /// @param sizeA Size of the first array.
        /// @param b Pointer to the second array.
        /// @param sizeB Size of the second array.
        /// @return A vector containing elements from both arrays.
        template<typename T>
        [[nodiscard]] static auto concat(const T *a, size_t sizeA, const T *b, size_t sizeB) -> std::vector<T>;

        /// @brief Computes the intersection of two arrays.
        /// @tparam T The type of elements in the arrays.
        /// @param a Pointer to the first array.
        /// @param sizeA Size of the first array.
        /// @param b Pointer to the second array.
        /// @param sizeB Size of the second array.
        /// @return A vector containing common elements (sorted, unique).
        template<typename T>
        [[nodiscard]] static auto intersection(const T *a, size_t sizeA, const T *b, size_t sizeB) -> std::vector<T>;

        /// @brief Computes the union of two arrays.
        /// @tparam T The type of elements in the arrays.
        /// @param a Pointer to the first array.
        /// @param sizeA Size of the first array.
        /// @param b Pointer to the second array.
        /// @param sizeB Size of the second array.
        /// @return A vector containing all unique elements (sorted).
        template<typename T>
        [[nodiscard]] static auto unionSet(const T *a, size_t sizeA, const T *b, size_t sizeB) -> std::vector<T>;

        /// @brief Computes the difference of two arrays (elements in a but not in b).
        /// @tparam T The type of elements in the arrays.
        /// @param a Pointer to the first array.
        /// @param sizeA Size of the first array.
        /// @param b Pointer to the second array.
        /// @param sizeB Size of the second array.
        /// @return A vector containing elements in a but not in b (sorted, unique).
        template<typename T>
        [[nodiscard]] static auto difference(const T *a, size_t sizeA, const T *b, size_t sizeB) -> std::vector<T>;

        /// @brief Checks if array is sorted in ascending order.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @return True if the array is sorted, false otherwise.
        template<typename T>
        [[nodiscard]] static auto isSorted(const T *array, size_t size) -> bool;

        /// @brief Shuffles the array randomly in place.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        template<typename T>
        static auto shuffle(T *array, size_t size) -> void;

        /// @brief Rotates array elements by specified positions.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param positions Number of positions to rotate (positive = right, negative = left).
        template<typename T>
        static auto rotate(T *array, size_t size, int32_t positions) -> void;

        /// @brief Gets the top K largest elements from the array.
        /// @tparam T The type of elements in the array.
        /// @param array Pointer to the array.
        /// @param size Size of the array.
        /// @param k Number of top elements to retrieve.
        /// @return A vector containing the top K elements in descending order.
        /// @throws std::invalid_argument If k is 0 or greater than size
        template<typename T>
        [[nodiscard]] static auto topK(const T *array, size_t size, size_t k) -> std::vector<T>;
    };

    template<typename T>
    auto ArraysToolkit::asList(const T *array, size_t size) -> std::vector<T> {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::asList: Array cannot be null when size > 0");
        }
        return std::vector<T>(array, array + size);
    }

    template<typename T>
    auto ArraysToolkit::binarySearch(const T *array, size_t size, const T &key) -> int32_t {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::binarySearch: Array cannot be null when size > 0");
        }
        auto it = std::lower_bound(array, array + size, key);
        if (it != array + size && *it == key) {
            return static_cast<int32_t>(it - array);
        }
        return -1;
    }

    template<typename T>
    auto ArraysToolkit::binarySearch(const T *array, size_t fromIndex, size_t toIndex, const T &key) -> int32_t {
        if (!array) {
            throw std::invalid_argument("ArraysToolkit::binarySearch: Array cannot be null");
        }
        if (fromIndex >= toIndex) {
            throw std::out_of_range("ArraysToolkit::binarySearch: fromIndex must be less than toIndex");
        }
        auto start = array + fromIndex;
        auto end = array + toIndex;
        auto it = std::lower_bound(start, end, key);
        if (it != end && *it == key) {
            return static_cast<int32_t>(it - array);
        }
        return -1;
    }

    template<typename T>
    auto ArraysToolkit::copyOf(const T *original, const size_t originalSize, size_t newLength) -> std::vector<T> {
        if (!original && originalSize > 0) {
            throw std::invalid_argument("ArraysToolkit::copyOf: Original array cannot be null when originalSize > 0");
        }
        std::vector<T> result(newLength);
        const size_t copyLength = std::min(originalSize, newLength);
        if (copyLength > 0 && original) {
            std::copy(original, original + copyLength, result.begin());
        }
        return result;
    }

    template<typename T>
    auto ArraysToolkit::copyOfRange(const T *original, size_t from, size_t to) -> std::vector<T> {
        if (!original) {
            throw std::invalid_argument("ArraysToolkit::copyOfRange: Original array cannot be null");
        }
        if (from > to) {
            throw std::out_of_range("ArraysToolkit::copyOfRange: from must be less than or equal to to");
        }
        return std::vector<T>(original + from, original + to);
    }

    template<typename T>
    auto ArraysToolkit::equals(const T *a, size_t sizeA, const T *b, const size_t sizeB) -> bool {
        if (!a && sizeA > 0) {
            throw std::invalid_argument("ArraysToolkit::equals: First array cannot be null when sizeA > 0");
        }
        if (!b && sizeB > 0) {
            throw std::invalid_argument("ArraysToolkit::equals: Second array cannot be null when sizeB > 0");
        }
        if (sizeA != sizeB) return false;
        if (sizeA == 0) return true; // Both arrays are empty, so they're equal
        return std::equal(a, a + sizeA, b);
    }

    template<typename T>
    auto ArraysToolkit::fill(T *array, size_t size, const T &value) -> void {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::fill: Array cannot be null when size > 0");
        }
        if (size > 0 && array) {
            std::fill(array, array + size, value);
        }
    }

    template<typename T>
    auto ArraysToolkit::sort(T *array, size_t size) -> void {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::sort: Array cannot be null when size > 0");
        }
        if (size > 0 && array) {
            std::sort(array, array + size);
        }
    }

    template<typename T>
    auto ArraysToolkit::sort(T *array, size_t fromIndex, size_t toIndex) -> void {
        if (!array) {
            throw std::invalid_argument("ArraysToolkit::sort: Array cannot be null");
        }
        if (fromIndex >= toIndex) {
            throw std::out_of_range("ArraysToolkit::sort: fromIndex must be less than toIndex");
        }
        std::sort(array + fromIndex, array + toIndex);
    }

    template<typename T>
    auto ArraysToolkit::toString(const T *array, const size_t size) -> std::string {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::toString: Array cannot be null when size > 0");
        }
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < size; ++i) {
            if (i > 0) oss << ", ";
            if (array) oss << array[i];
        }
        oss << "]";
        return oss.str();
    }

    template<typename T>
    auto ArraysToolkit::isEmpty(const T *array, size_t size) -> bool {
        return (array == nullptr) || (size == 0);
    }

    template<typename T>
    auto ArraysToolkit::contains(const T *array, size_t size, const T &value) -> bool {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::contains: Array cannot be null when size > 0");
        }
        return std::find(array, array + size, value) != array + size;
    }

    template<typename T>
    auto ArraysToolkit::linearSearch(const T *array, size_t size, const T &key) -> int32_t {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::linearSearch: Array cannot be null when size > 0");
        }
        for (size_t i = 0; i < size; ++i) {
            if (array[i] == key) {
                return static_cast<int32_t>(i);
            }
        }
        return -1;
    }

    template<typename T>
    auto ArraysToolkit::maxElement(const T *array, size_t size) -> int32_t {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::maxElement: Array cannot be null when size > 0");
        }
        if (size == 0) {
            return -1;
        }
        auto it = std::max_element(array, array + size);
        return static_cast<int32_t>(it - array);
    }

    template<typename T>
    auto ArraysToolkit::minElement(const T *array, size_t size) -> int32_t {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::minElement: Array cannot be null when size > 0");
        }
        if (size == 0) {
            return -1;
        }
        auto it = std::min_element(array, array + size);
        return static_cast<int32_t>(it - array);
    }

    template<typename T>
    auto ArraysToolkit::count(const T *array, size_t size, const T &value) -> size_t {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::count: Array cannot be null when size > 0");
        }
        return std::count(array, array + size, value);
    }

    template<typename T>
    auto ArraysToolkit::reverse(T *array, size_t size) -> void {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::reverse: Array cannot be null when size > 0");
        }
        if (size > 0 && array) {
            std::reverse(array, array + size);
        }
    }

    template<typename T>
    auto ArraysToolkit::reverse(T *array, size_t fromIndex, size_t toIndex) -> void {
        if (!array) {
            throw std::invalid_argument("ArraysToolkit::reverse: Array cannot be null");
        }
        if (fromIndex >= toIndex) {
            throw std::out_of_range("ArraysToolkit::reverse: fromIndex must be less than toIndex");
        }
        std::reverse(array + fromIndex, array + toIndex);
    }

    template<typename T>
    auto ArraysToolkit::distinct(const T *array, size_t size) -> std::vector<T> {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::distinct: Array cannot be null when size > 0");
        }
        std::set<T> uniqueElements(array, array + size);
        return std::vector<T>(uniqueElements.begin(), uniqueElements.end());
    }

    template<typename T, typename U, typename Func>
    auto ArraysToolkit::map(const T *array, size_t size, Func func) -> std::vector<U> {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::map: Array cannot be null when size > 0");
        }
        std::vector<U> result;
        result.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            result.push_back(func(array[i]));
        }
        return result;
    }

    template<typename T, typename Predicate>
    auto ArraysToolkit::filter(const T *array, size_t size, Predicate predicate) -> std::vector<T> {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::filter: Array cannot be null when size > 0");
        }
        std::vector<T> result;
        for (size_t i = 0; i < size; ++i) {
            if (predicate(array[i])) {
                result.push_back(array[i]);
            }
        }
        return result;
    }

    template<typename T>
    auto ArraysToolkit::sum(const T *array, size_t size) -> T {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::sum: Array cannot be null when size > 0");
        }
        if (size == 0) {
            return T{};
        }
        return std::accumulate(array, array + size, T{});
    }

    template<typename T>
    auto ArraysToolkit::average(const T *array, size_t size) -> double {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::average: Array cannot be null when size > 0");
        }
        if (size == 0) {
            throw std::invalid_argument("ArraysToolkit::average: Cannot calculate average of empty array");
        }
        return static_cast<double>(std::accumulate(array, array + size, T{})) / static_cast<double>(size);
    }

    template<typename T, typename Predicate>
    auto ArraysToolkit::allMatch(const T *array, size_t size, Predicate predicate) -> bool {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::allMatch: Array cannot be null when size > 0");
        }
        return std::all_of(array, array + size, predicate);
    }

    template<typename T, typename Predicate>
    auto ArraysToolkit::anyMatch(const T *array, size_t size, Predicate predicate) -> bool {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::anyMatch: Array cannot be null when size > 0");
        }
        return std::any_of(array, array + size, predicate);
    }

    template<typename T, typename Predicate>
    auto ArraysToolkit::noneMatch(const T *array, size_t size, Predicate predicate) -> bool {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::noneMatch: Array cannot be null when size > 0");
        }
        return std::none_of(array, array + size, predicate);
    }

    template<typename T>
    auto ArraysToolkit::concat(const T *a, size_t sizeA, const T *b, size_t sizeB) -> std::vector<T> {
        if (!a && sizeA > 0) {
            throw std::invalid_argument("ArraysToolkit::concat: First array cannot be null when sizeA > 0");
        }
        if (!b && sizeB > 0) {
            throw std::invalid_argument("ArraysToolkit::concat: Second array cannot be null when sizeB > 0");
        }
        std::vector<T> result;
        result.reserve(sizeA + sizeB);
        if (sizeA > 0 && a) {
            result.insert(result.end(), a, a + sizeA);
        }
        if (sizeB > 0 && b) {
            result.insert(result.end(), b, b + sizeB);
        }
        return result;
    }

    template<typename T>
    auto ArraysToolkit::intersection(const T *a, size_t sizeA, const T *b, size_t sizeB) -> std::vector<T> {
        if (!a && sizeA > 0) {
            throw std::invalid_argument("ArraysToolkit::intersection: First array cannot be null when sizeA > 0");
        }
        if (!b && sizeB > 0) {
            throw std::invalid_argument("ArraysToolkit::intersection: Second array cannot be null when sizeB > 0");
        }
        std::set<T> setA(a, a + sizeA);
        std::set<T> setB(b, b + sizeB);
        std::vector<T> result;
        std::set_intersection(setA.begin(), setA.end(), setB.begin(), setB.end(), std::back_inserter(result));
        return result;
    }

    template<typename T>
    auto ArraysToolkit::unionSet(const T *a, size_t sizeA, const T *b, size_t sizeB) -> std::vector<T> {
        if (!a && sizeA > 0) {
            throw std::invalid_argument("ArraysToolkit::unionSet: First array cannot be null when sizeA > 0");
        }
        if (!b && sizeB > 0) {
            throw std::invalid_argument("ArraysToolkit::unionSet: Second array cannot be null when sizeB > 0");
        }
        std::set<T> setA(a, a + sizeA);
        std::set<T> setB(b, b + sizeB);
        std::vector<T> result;
        std::set_union(setA.begin(), setA.end(), setB.begin(), setB.end(), std::back_inserter(result));
        return result;
    }

    template<typename T>
    auto ArraysToolkit::difference(const T *a, size_t sizeA, const T *b, size_t sizeB) -> std::vector<T> {
        if (!a && sizeA > 0) {
            throw std::invalid_argument("ArraysToolkit::difference: First array cannot be null when sizeA > 0");
        }
        if (!b && sizeB > 0) {
            throw std::invalid_argument("ArraysToolkit::difference: Second array cannot be null when sizeB > 0");
        }
        std::set<T> setA(a, a + sizeA);
        std::set<T> setB(b, b + sizeB);
        std::vector<T> result;
        std::set_difference(setA.begin(), setA.end(), setB.begin(), setB.end(), std::back_inserter(result));
        return result;
    }

    template<typename T>
    auto ArraysToolkit::isSorted(const T *array, size_t size) -> bool {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::isSorted: Array cannot be null when size > 0");
        }
        if (size <= 1) {
            return true;
        }
        return std::is_sorted(array, array + size);
    }

    template<typename T>
    auto ArraysToolkit::shuffle(T *array, size_t size) -> void {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::shuffle: Array cannot be null when size > 0");
        }
        if (size > 0 && array) {
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(array, array + size, g);
        }
    }

    template<typename T>
    auto ArraysToolkit::rotate(T *array, size_t size, int32_t positions) -> void {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::rotate: Array cannot be null when size > 0");
        }
        if (size == 0 || positions == 0) {
            return;
        }
        // Normalize positions to be within [0, size)
        int32_t normalizedPos = positions % static_cast<int32_t>(size);
        if (normalizedPos < 0) {
            normalizedPos += static_cast<int32_t>(size);
        }
        std::rotate(array, array + (size - normalizedPos), array + size);
    }

    template<typename T>
    auto ArraysToolkit::topK(const T *array, size_t size, size_t k) -> std::vector<T> {
        if (!array && size > 0) {
            throw std::invalid_argument("ArraysToolkit::topK: Array cannot be null when size > 0");
        }
        if (k == 0 || k > size) {
            throw std::invalid_argument("ArraysToolkit::topK: k must be between 1 and size");
        }
        std::vector<T> copy(array, array + size);
        std::partial_sort(copy.begin(), copy.begin() + k, copy.end(), std::greater<T>());
        return std::vector<T>(copy.begin(), copy.begin() + k);
    }
}
