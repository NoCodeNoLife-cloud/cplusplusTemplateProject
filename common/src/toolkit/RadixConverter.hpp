// ReSharper disable CppDFANotInitializedField
#pragma once
#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <concepts>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

/**
 * @namespace common
 * @brief Common utilities for numeric computation and data conversion
 * 
 * @details
 * The common namespace hosts cross-platform utilities that are frequently used
 * across multiple modules. All radix conversion functionality resides in this
 * namespace to prevent symbol collisions.
 */
namespace common {
    /**
     * @concept Numeric
     * @ingroup concepts
     * @brief Constrains types to numeric categories (integral, floating-point, or big integer)
     * @tparam T Type to constrain
     *
     * @details
     * Accepts any standard integral type (signed/unsigned), floating-point type,
     * or Boost.Multiprecision cpp_int arbitrary-precision integer.
     *
     * @note This is the primary concept used by RadixConverter primary template.
     *
     * @see StandardIntegral, FloatingPoint
     */
    template<typename T>
    concept Numeric = std::integral<T> || std::floating_point<T> ||
                      std::same_as<T, boost::multiprecision::cpp_int>;

    /**
     * @concept StandardIntegral
     * @ingroup concepts
     * @brief Constrains types to standard integral types (excluding big integers)
     * @tparam T Type to constrain
     *
     * @details
     * Matches only built-in integral types (int, long, uint64_t, etc.).
     * Excludes boost::multiprecision::cpp_int which requires special handling.
     *
     * @warning Do not use for arbitrary-precision integers. Use explicit
     *          RadixConverter<BigInt> for big integers.
     */
    template<typename T>
    concept StandardIntegral = std::integral<T>;

    /**
     * @concept FloatingPoint
     * @ingroup concepts
     * @brief Constrains types to IEEE 754 floating-point types
     * @tparam T Type to constrain
     *
     * @details
     * Matches float, double, and long double. Specialized handling required
     * for base conversion due to fractional part representation.
     *
     * @attention Floating-point conversions may lose precision for very large
     *            mantissas or when converting to bases that don't divide cleanly
     *            into powers of 2.
     */
    template<typename T>
    concept FloatingPoint = std::floating_point<T>;

    /**
     * @class ConvertError
     * @ingroup errors
     * @brief Specialized exception for radix conversion operation failures
     * @extends std::runtime_error
     *
     * @details
     * Provides granular error classification through the Type enum, allowing
     * callers to programmatically distinguish between invalid input, overflow
     * conditions, format errors, and unsupported operations.
     *
     * @par Thread Safety:
     * As with all standard exceptions, this class is not thread-safe for
     * modification but safe for concurrent reading.
     *
     * @example
     * @code{.cpp}
     * try {
     *     auto val = converter.decode("xyz");
     * } catch (const ConvertError& e) {
     *     if (e.type() == ConvertError::Type::InvalidDigit) {
     *         // Handle invalid character...
     *     }
     * }
     * @endcode
     */
    class ConvertError : public std::runtime_error {
    public:
        /**
         * @enum Type
         * @brief Error classification types for programmatic error handling
         */
        enum class Type {
            InvalidDigit,
            ///< Character not found in valid alphabet
            Overflow,
            ///< Value exceeds maximum limit of target type
            Underflow,
            ///< Value below minimum limit of target type
            InvalidFormat,
            ///< Malformed input string (e.g., multiple dots)
            EmptyInput,
            ///< Empty or whitespace-only input provided
            UnsupportedBase ///< Base outside valid range (2-256) or alphabet insufficient
        };

        /**
         * @brief Constructs a conversion error with type classification
         * @param[in] t Error type classification for programmatic handling
         * @param[in] msg Human-readable error description ( UTF-8 encoded)
         * @throws None Operation is noexcept
         */
        ConvertError(Type t, const std::string &msg) : std::runtime_error(msg), type_(t) {
        }

        /**
         * @brief Retrieves the error type category
         * @return Type enum value indicating error classification
         * @retval Type::InvalidDigit When character not in alphabet
         * @retval Type::Overflow When result exceeds std::numeric_limits<T>::max()
         * @retval Type::InvalidFormat When string format is invalid
         * @retval Type::EmptyInput When input string is empty
         * @retval Type::UnsupportedBase When base < 2 or base > alphabet size
         *
         * @note This accessor enables switch-based error handling without string parsing.
         */
        [[nodiscard]] Type type() const noexcept { return type_; }

    private:
        Type type_; ///< Internal storage for error classification
    };

    /**
     * @struct ConvertOptions
     * @ingroup config
     * @brief Configuration parameters for numeric encoding and decoding operations
     *
     * @details
     * Aggregates all formatting preferences for radix conversion operations.
     * Default values provide standard behavior (base-10, lowercase, no grouping).
     *
     * @par Alphabet Requirements:
     * The alphabet string must contain at least 'base' unique characters.
     * Characters beyond position 'base-1' are ignored during encoding but
     * accepted during decoding if they appear in valid positions.
     *
     * @par Grouping Behavior:
     * When group_size > 0 and group_separator != '\0', digits are grouped
     * from right to left (least significant to most significant).
     *
     * @warning Setting case_sensitive=false with overlapping upper/lowercase
     *          characters in alphabet may lead to ambiguous decoding.
     */
    struct ConvertOptions {
        /**
         * @brief Custom digit alphabet mapping indices to characters
         * @details Default: "0123456789abcdefghijklmnopqrstuvwxyz" (36 chars)
         * Position 0 represents digit value 0, position 1 represents 1, etc.
         */
        std::string alphabet = "0123456789abcdefghijklmnopqrstuvwxyz";

        /**
         * @brief Error handling strategy
         * @details
         * - true: Throw ConvertError on any validation failure
         * - false: Return std::nullopt (for tryDecode methods)
         */
        bool throw_on_error = true;

        /**
         * @brief Case sensitivity for alphabetic digits
         * @details
         * - false (default): 'A' and 'a' both map to decimal 10
         * - true: Only exact case matches alphabet position
         */
        bool case_sensitive = false;

        /**
         * @brief Separator character for digit grouping
         * @details Common values: ',' (thousands), ':' (MAC addresses), ' ' (bytes)
         * Set to '\0' (null char) to disable grouping.
         */
        char group_separator = '\0';

        /**
         * @brief Number of digits per group
         * @details Typical values: 3 (decimal thousands), 2 (hex bytes), 4 (binary nibbles)
         * Only applies when group_separator is non-null.
         */
        int group_size = 0;

        /**
         * @brief Add base prefix to encoded output
         * @details
         * - Base 2: prepends "0b" or "0B"
         * - Base 8: prepends "0"
         * - Base 16: prepends "0x" or "0X"
         * - Other bases: no prefix
         */
        bool prefix_base = false;

        /**
         * @brief Use uppercase for alphabetic digits (A-F vs a-f)
         * @details Affects both encoding output and prefix format (0X vs 0x).
         * Only meaningful when alphabet contains letters.
         */
        bool upper_case = false;
    };

    /**
     * @struct HexOptions
     * @ingroup config
     * @brief Specialized configuration for hexadecimal byte array encoding
     *
     * @details
     * Dedicated configuration structure for ByteArrayConverter::toHex(),
     * optimized for common byte encoding scenarios (MAC addresses, memory dumps,
     * UUIDs, etc.).
     *
     * @note This is separate from ConvertOptions because byte array conversion
     *       has different concerns than numeric conversion (byte order, separators
     *       between bytes rather than digits).
     */
    struct HexOptions {
        /** @brief Use uppercase hexadecimal digits (A-F vs a-f) */
        bool use_uppercase = false;

        /** @brief Prepend "0x" or "0X" prefix to output string */
        bool add_prefix = false;

        /**
         * @brief Byte separator character
         * @details Common patterns:
         * - ':' for MAC addresses (AA:BB:CC:DD:EE:FF)
         * - ' ' for hex dumps (48 65 6C 6C 6F)
         * - '\0' for compact representation (AABBCCDDEEFF)
         */
        char separator = '\0';

        /**
         * @brief Reverse byte order (little-endian display)
         * @details
         * - false: Display bytes in array order (index 0 first)
         * - true: Display bytes in reverse order (last index first)
         * Useful for representing little-endian multi-byte values.
         */
        bool reverse_order = false;
    };

    /**
     * @typedef BigInt
     * @ingroup types
     * @brief Convenience alias for Boost.Multiprecision cpp_int
     *
     * @details
     * Arbitrary-precision signed integer with no fixed size limit.
     * Memory expands dynamically as needed (heap allocation).
     * Operations are slower than native integers but have no overflow risk.
     *
     * @attention Copying BigInt involves deep copy of dynamic memory.
     *            Use const references for function parameters when possible.
     *
     * @see boost::multiprecision::cpp_int
     */
    using BigInt = boost::multiprecision::cpp_int;

    namespace detail {
        /**
         * @class AlphabetLookup
         * @ingroup internal
         * @brief O(1) character-to-value lookup table with case folding support
         *
         * @details
         * Replaces std::string::find() (O(N)) with array indexing (O(1)).
         * Constructs a 256-element lookup table mapping ASCII values to digit values.
         * Supports case-insensitive matching by duplicating mappings for A-Z to a-z.
         *
         * @par Complexity:
         * - Construction: O(AlphabetSize) - typically <= 256
         * - Lookup: O(1) - single array index
         *
         * @par Memory:
         * Fixed 256 bytes per instance (256 ints × 1 byte each with packing,
         * or 1024 bytes for 4-byte ints).
         */
        class AlphabetLookup {
        public:
            /**
             * @brief Constructs lookup table from alphabet string
             * @param[in] alphabet String defining digit-to-value mapping
             * @param[in] case_sensitive Whether to treat 'A' and 'a' as distinct
             * @throws None Operation is noexcept
             * @pre alphabet contains unique characters (caller responsibility)
             */
            explicit AlphabetLookup(std::string_view alphabet, bool case_sensitive) {
                table_.fill(-1);

                for (size_t i = 0; i < alphabet.size(); ++i) {
                    unsigned char c = static_cast<unsigned char>(alphabet[i]);
                    table_[c] = static_cast<int>(i);

                    // Populate case-insensitive mappings for letters
                    if (!case_sensitive && c >= 'a' && c <= 'z') {
                        table_[c - 'a' + 'A'] = static_cast<int>(i); // Map uppercase to same value
                    } else if (!case_sensitive && c >= 'A' && c <= 'Z') {
                        table_[c - 'A' + 'a'] = static_cast<int>(i); // Map lowercase to same value
                    }
                }
            }

            /**
             * @brief Lookup digit value for character
             * @param[in] c Character to convert
             * @return Digit value (0-255) or -1 if invalid
             * @retval >=0 Valid digit value representing position in alphabet
             * @retval -1 Character not found in alphabet (or case mismatch if sensitive)
             * @throws None noexcept operation
             */
            [[nodiscard]] int lookup(char c) const noexcept {
                return table_[static_cast<unsigned char>(c)];
            }

        private:
            std::array<int, 256> table_; ///< Fixed-size lookup table covering all ASCII values
        };

        /**
         * @brief Format output string with grouping, prefixes, and case transformation
         * @ingroup internal
         * @param[in] s Raw digit string (already reversed and sign-applied)
         * @param[in] opts Formatting options
         * @param[in] base Numeric base (affects prefix selection)
         * @return Formatted string with separators, prefixes, and case applied
         * @throws std::bad_alloc If string insertion causes reallocation failure
         *
         * @details
         * Applies three formatting stages in order:
         * 1. Digit grouping (right-to-left insertion of separators)
         * 2. Base prefix insertion (0x, 0b, 0 for hex/bin/oct)
         * 3. Case transformation (toupper for upper_case option)
         */
        [[nodiscard]] inline std::string formatOutput(std::string s,
                                                      const ConvertOptions &opts,
                                                      unsigned int base) {
            // Apply grouping
            if (opts.group_size > 0 && opts.group_separator != '\0' && !s.empty()) {
                int start = static_cast<int>(s.size()) - opts.group_size;
                bool has_sign = (s[0] == '-');
                if (has_sign) start--;

                // Pre-calculate final size to avoid reallocations
                int num_separators = std::max(0, (start - (has_sign ? 1 : 0) + opts.group_size - 1) / opts.group_size);
                s.reserve(s.size() + num_separators);

                for (int i = start; i > (has_sign ? 1 : 0); i -= opts.group_size) {
                    if (i > (has_sign ? 1 : 0)) {
                        s.insert(static_cast<size_t>(i), 1, opts.group_separator);
                    }
                }
            }

            // Apply prefix
            if (opts.prefix_base) {
                std::string prefix;
                if (base == 16) prefix = opts.upper_case ? "0X" : "0x";
                else if (base == 2) prefix = opts.upper_case ? "0B" : "0b";
                else if (base == 8) prefix = "0";

                if (!prefix.empty()) {
                    s.insert(0, prefix);
                }
            }

            // Apply case transformation
            if (opts.upper_case) {
                std::ranges::transform(s, s.begin(),
                                       [](unsigned char c) { return std::toupper(c); });
            }

            return s;
        }

        /**
         * @brief Strip standard base prefixes from input string
         * @ingroup internal
         * @param[in] input Raw input string view
         * @param[in] base Expected base (determines which prefixes to strip)
         * @param[in] prefix_base Whether prefix stripping is enabled
         * @return String view with prefix removed (points to same underlying data)
         * @throws None noexcept operation
         * @note Returns modified string_view with prefix stripped if applicable
         */
        [[nodiscard]] inline std::string_view stripPrefix(const std::string_view input,
                                                          const unsigned int base,
                                                          const bool prefix_base) noexcept {
            if (!prefix_base || input.size() < 2) {
                return input.substr();
            }

            // Check for 0x/0X (hex)
            if ((input.substr(0, 2) == "0x" || input.substr(0, 2) == "0X") && base == 16) {
                return input.substr(2);
            }
            // Check for 0b/0B (binary)
            if ((input.substr(0, 2) == "0b" || input.substr(0, 2) == "0B") && base == 2) {
                return input.substr(2);
            }
            // Check for 0 (octal) - single prefix, ensure we don't strip "0" alone
            if (input[0] == '0' && base == 8 && input.size() > 1) {
                return input.substr(1);
            }

            return input.substr();
        }

        /**
         * @brief Validate base against alphabet size constraints
         * @ingroup internal
         * @param[in] base Requested numeric base
         * @param[in] alphabet Available character alphabet
         * @return Validated base (unchanged)
         * @throws ConvertError with Type::UnsupportedBase if validation fails
         *
         * @details
         * Constraints checked:
         * - Base must be >= 2 (MIN_BASE)
         * - Base must be <= 256 (MAX_BASE)
         * - Base must be <= alphabet size (unless alphabet empty, then default 36 allowed)
         */
        [[nodiscard]] inline unsigned int validateBase(unsigned int base,
                                                       std::string_view alphabet) {
            constexpr unsigned int MAX_BASE = 256;
            constexpr unsigned int MIN_BASE = 2;

            size_t max_allowed = std::min(alphabet.empty() ? static_cast<size_t>(36) : alphabet.size(),
                                          static_cast<size_t>(MAX_BASE));
            if (base < MIN_BASE || base > max_allowed) {
                throw ConvertError(ConvertError::Type::UnsupportedBase,
                                   std::format(R"(Base must be between {} and {})", MIN_BASE, max_allowed));
            }
            return base;
        }

        /**
         * @brief Check if multiplication would cause unsigned overflow
         * @ingroup internal
         * @tparam T Unsigned integral type
         * @param[in] value Current accumulated value
         * @param[in] multiplier Base being multiplied by
         * @param[in] max_val Maximum value for type T
         * @return true if value * multiplier would exceed max_val
         * @note All parameters must be unsigned to avoid undefined behavior
         */
        template<typename T>
        [[nodiscard]] constexpr bool willMultiplyOverflow(T value, T multiplier, T max_val) noexcept {
            return value > max_val / multiplier;
        }

        /**
         * @brief Check if addition would cause unsigned overflow
         * @ingroup internal
         * @tparam T Unsigned integral type
         * @param[in] value Current accumulated value
         * @param[in] addend Digit being added (0 <= addend < base)
         * @param[in] max_val Maximum value for type T
         * @return true if value + addend would exceed max_val
         */
        template<typename T>
        [[nodiscard]] constexpr bool willAddOverflow(T value, T addend, T max_val) noexcept {
            return value > max_val - addend;
        }
    } // namespace detail

    /**
     * @class RadixConverter
     * @ingroup converters
     * @brief Primary template declaration for numeric base conversion
     * @tparam T Numeric type (constrained by Numeric concept)
     *
     * @warning This primary template is not defined. Only the following
     *          specializations are available:
     *          - RadixConverter<StandardIntegral T>
     *          - RadixConverter<FloatingPoint T>
     *          - RadixConverter<BigInt> (explicit specialization)
     *
     * @see RadixConverter<int>, RadixConverter<double>, RadixConverter<BigInt>
     */
    template<Numeric T>
    class RadixConverter;

    /**
     * @class RadixConverter
     * @ingroup converters
     * @brief Specialized high-performance converter for standard integral types
     * @tparam T Standard integral type (int, long, uint64_t, etc.), constrained by StandardIntegral concept
     *
     * @details
     * Optimized implementation for native integer types featuring:
     * - Overflow detection using unsigned arithmetic
     * - O(1) alphabet lookup
     * - Branch prediction hints for common paths (value == 0)
     *
     * @par Supported Bases:
     * 2 to 256 (constrained by alphabet size)
     *
     * @par Thread Safety:
     * Const methods are thread-safe. Non-const methods (none in this version)
     * would require external synchronization.
     *
     * @par Example Usage:
     * @code{.cpp}
     * RadixConverter<uint32_t> conv(16);  // Hex converter
     * auto str = conv.encode(0xDEADBEEF); // "deadbeef"
     * auto val = conv.decode("BEEF");     // 48879
     * @endcode
     */
    template<StandardIntegral T>
    class RadixConverter<T> {
    public:
        /** @brief Expose template parameter as public type */
        using ValueType = T;

        static constexpr unsigned int MAX_BASE = 256; ///< Maximum supported base
        static constexpr unsigned int MIN_BASE = 2; ///< Minimum supported base

        /**
         * @brief Construct converter with specified base and options
         * @param[in] base Target numeric base (2-256)
         * @param[in] opts Conversion configuration options
         * @throws ConvertError::Type::UnsupportedBase If base out of range
         * @throws ConvertError::Type::UnsupportedBase If alphabet size < base
         * @post Converter is immutable and ready for encode/decode operations
         *
         * @note The alphabet is copied into the converter instance.
         */
        explicit RadixConverter(unsigned int base = 10, ConvertOptions opts = {}) : opts_(std::move(opts)),
                                                                                    base_(detail::validateBase(base, opts_.alphabet)),
                                                                                    lookup_(opts_.alphabet, opts_.case_sensitive) {
            if (opts_.alphabet.size() < base_) {
                throw ConvertError(ConvertError::Type::UnsupportedBase,
                                   std::format("Alphabet size ({}) smaller than base ({})",
                                               opts_.alphabet.size(), base_));
            }
        }

        /**
         * @brief Encode integer value to string representation
         * @param[in] value Integer to encode
         * @return String representation in configured base and format
         * @throws None noexcept for valid inputs, may throw std::bad_alloc
         * @retval "0" If value is zero
         * @retval "-[digits]" If value negative (signed types only)
         * @retval "[prefix][grouped_digits]" Depending on ConvertOptions
         *
         * @details
         * Algorithm: Repeated division-remainder (standard base conversion).
         * Complexity: O(log_base(value)) divisions.
         *
         * @note Handles std::numeric_limits<T>::min() specially to avoid
         *       overflow during negation.
         */
        [[nodiscard]] std::string encode(T value) const {
            if (value == 0) {
                return detail::formatOutput(std::string(1, opts_.alphabet[0]), opts_, base_);
            }

            // Handle signed minimum value specially to avoid overflow on negation
            if constexpr (std::is_signed_v<T>) {
                if (value == std::numeric_limits<T>::min()) {
                    return encodeSignedMin();
                }
            }

            bool negative = false;
            if constexpr (std::is_signed_v<T>) {
                if (value < 0) {
                    negative = true;
                    value = -value; // Safe because we handled min() above
                }
            }

            using UnsignedT = std::make_unsigned_t<T>;
            UnsignedT uValue = static_cast<UnsignedT>(value);

            // Conservative buffer estimation
            constexpr int max_bits = sizeof(T) * 8 + (std::is_signed_v<T> ? 1 : 0);
            const size_t estimated_digits = (max_bits * 2 + base_ - 2) / base_ + 1;

            std::string result;
            result.reserve(estimated_digits);

            while (uValue > 0) {
                result.push_back(opts_.alphabet[uValue % base_]);
                uValue /= static_cast<UnsignedT>(base_);
            }

            if (negative) result.push_back('-');
            std::ranges::reverse(result);

            return detail::formatOutput(result, opts_, base_);
        }

        /**
         * @brief Decode string to integer value
         * @param[in] input String to parse (may contain separators/prefixes)
         * @return Decoded integer value of type T
         * @throws ConvertError::Type::EmptyInput If input is empty
         * @throws ConvertError::Type::InvalidFormat If sign on unsigned type
         * @throws ConvertError::Type::InvalidDigit If character not in alphabet
         * @throws ConvertError::Type::Overflow If value exceeds type limits
         *
         * @details
         * Algorithm: Horner's method with overflow checking.
         * Complexity: O(N) where N is input length.
         *
         * @note Strips prefixes (0x, 0b, 0) automatically if prefix_base enabled.
         *       Skips group_separator characters transparently.
         */
        [[nodiscard]] T decode(std::string_view input) const {
            if (input.empty()) {
                throw ConvertError(ConvertError::Type::EmptyInput, "Empty input string");
            }

            input = detail::stripPrefix(input, base_, opts_.prefix_base);

            size_t pos = 0;
            bool negative = false;

            if (input[0] == '-') {
                if constexpr (!std::is_signed_v<T>) {
                    throw ConvertError(ConvertError::Type::InvalidFormat,
                                       "Negative value for unsigned type");
                } else {
                    negative = true;
                }
                pos = 1;
            } else if (input[0] == '+') {
                pos = 1;
            }

            using UnsignedT = std::make_unsigned_t<T>;
            UnsignedT result = 0;

            const UnsignedT ubase = static_cast<UnsignedT>(base_);
            const UnsignedT max_val = static_cast<UnsignedT>(std::numeric_limits<T>::max());
            const UnsignedT max_div = max_val / ubase;
            const UnsignedT max_rem = max_val % ubase;

            for (; pos < input.size(); ++pos) {
                char c = input[pos];
                if (opts_.group_separator != '\0' && c == opts_.group_separator) {
                    continue;
                }

                int digit = lookup_.lookup(c);
                if (digit < 0 || static_cast<unsigned int>(digit) >= base_) {
                    throw ConvertError(ConvertError::Type::InvalidDigit,
                                       std::format("Invalid character '{}' at position {}", c, pos));
                }

                // Overflow check using unsigned arithmetic (safe from UB)
                if (result > max_div || (result == max_div &&
                                         static_cast<UnsignedT>(digit) > max_rem)) {
                    throw ConvertError(ConvertError::Type::Overflow,
                                       "Value exceeds type limits during decoding");
                }

                result = result * ubase + static_cast<UnsignedT>(digit);
            }

            if constexpr (std::is_signed_v<T>) {
                if (negative) {
                    if (result > static_cast<UnsignedT>(std::numeric_limits<T>::max()) + 1) {
                        throw ConvertError(ConvertError::Type::Overflow, "Value underflow");
                    }
                    return -static_cast<T>(result);
                }
            }

            return static_cast<T>(result);
        }

        /**
         * @brief Non-throwing variant of decode
         * @param[in] input String to parse
         * @return Optional containing value, or nullopt on any error
         * @throws None noexcept guarantee - catches all exceptions internally
         * @note Useful when error handling through exceptions is undesirable
         */
        [[nodiscard]] std::optional<T> tryDecode(std::string_view input) const noexcept {
            try {
                return decode(input);
            } catch (...) {
                return std::nullopt;
            }
        }

        /**
         * @brief Cross-base conversion using intermediate type
         * @tparam U Intermediate integral type (typically larger than T)
         * @param[in] input String in source base
         * @param[in] src_base Source numeric base
         * @param[in] src_opts Source conversion options
         * @return String encoded in this converter's target base
         * @throws ConvertError Propagated from source or target conversion
         *
         * @details
         * Convenience method for converting between bases without manually
         * managing intermediate values.
         *
         * @code{.cpp}
         * // Convert hex string "FF" to binary string "11111111"
         * RadixConverter<int> bin_conv(2);
         * auto result = bin_conv.convert<int>("FF", 16, {.prefix_base=true});
         * @endcode
         */
        template<StandardIntegral U>
        [[nodiscard]] std::string convert(std::string_view input,
                                          unsigned int src_base,
                                          const ConvertOptions &src_opts = {}) const {
            RadixConverter<U> src_conv(src_base, src_opts);
            U intermediate = src_conv.decode(input);
            return encode(static_cast<T>(intermediate));
        }

        /** @brief Get configured base */
        [[nodiscard]] unsigned int base() const noexcept { return base_; }

        /**
         * @brief Check if character is valid digit in current alphabet
         * @param[in] c Character to validate
         * @return true if character maps to a valid digit value
         */
        [[nodiscard]] bool isValidDigit(char c) const { return lookup_.lookup(c) >= 0; }

        /**
         * @brief Stream insertion operator for debugging
         * @param[in,out] os Output stream
         * @param[in] conv Converter instance to describe
         * @return Reference to os for chaining
         */
        friend std::ostream &operator<<(std::ostream &os, const RadixConverter &conv) {
            return os << std::format("RadixConverter<{}>(base={})", typeid(T).name(), conv.base_);
        }

    private:
        ConvertOptions opts_; ///< Stored formatting options
        unsigned int base_; ///< Numeric base (2-256)
        detail::AlphabetLookup lookup_; ///< O(1) character lookup table

        /**
         * @brief Handle encoding of signed minimum value (abs overflows)
         * @return String representation of std::numeric_limits<T>::min()
         * @throws None
         * @details
         * Special case: -2^N cannot be negated in two's complement (overflows to -2^N).
         * Strategy: Convert using unsigned arithmetic with precomputed constant.
         */
        [[nodiscard]] std::string encodeSignedMin() const {
            using UnsignedT = std::make_unsigned_t<T>;

            // Precomputed absolute values for standard sizes (avoids overflow)
            UnsignedT uValue;
            if constexpr (sizeof(T) == 1) {
                uValue = static_cast<UnsignedT>(128); // int8_t: -128
            } else if constexpr (sizeof(T) == 2) {
                uValue = static_cast<UnsignedT>(32768); // int16_t: -32768
            } else if constexpr (sizeof(T) == 4) {
                uValue = static_cast<UnsignedT>(2147483648ULL); // int32_t: -2147483648
            } else {
                uValue = static_cast<UnsignedT>(9223372036854775808ULL); // int64_t
            }

            std::string result;
            result.reserve(24);

            while (uValue > 0) {
                result.push_back(opts_.alphabet[uValue % base_]);
                uValue /= static_cast<UnsignedT>(base_);
            }

            result.push_back('-');
            std::ranges::reverse(result);

            return detail::formatOutput(result, opts_, base_);
        }
    };

    /**
     * @class RadixConverter
     * @ingroup converters
     * @brief IEEE 754 floating-point base converter with scientific notation support
     * @tparam T Floating-point type (float, double, long double), constrained by FloatingPoint concept
     *
     * @details
     * Handles floating-point values by separating integer and fractional parts.
     * Supports scientific notation for very large/small values.
     *
     * @attention Precision is limited by the @c precision parameter (default 6).
     *            Values requiring more digits will be rounded/truncated.
     *
     * @par Special Values:
     * - NaN: encoded as "NaN", decoded case-insensitively
     * - Infinity: encoded as "Inf"/"-Inf", supports various input formats
     */
    template<FloatingPoint T>
    class RadixConverter<T> {
    public:
        /** @brief Expose template parameter */
        using ValueType = T;

        /**
         * @brief Construct floating-point converter
         * @param[in] base Target base (2-36, limited by single-char digits)
         * @param[in] opts Formatting options
         * @param[in] precision Maximum fractional digits (default 6)
         * @param[in] max_exp_digits Max exponent digits in scientific notation
         * @throws ConvertError::Type::UnsupportedBase If base outside 2-36
         *
         * @note Floating-point bases are limited to 36 because standard alphabet
         *       uses single ASCII characters for digits 0-35.
         */
        explicit RadixConverter(unsigned int base = 10,
                                ConvertOptions opts = {},
                                int precision = 6,
                                int max_exp_digits = 4) : opts_(std::move(opts)),
                                                          base_(base),
                                                          precision_(precision),
                                                          max_exp_digits_(max_exp_digits),
                                                          lookup_(opts_.alphabet.empty() ? default_alphabet : opts_.alphabet,
                                                                  opts_.case_sensitive) {
            if (base_ < 2 || base_ > 36) {
                throw ConvertError(ConvertError::Type::UnsupportedBase,
                                   "Float base must be 2-36");
            }

            if (opts_.alphabet.empty()) {
                opts_.alphabet = std::string(default_alphabet);
            }
        }

        /**
         * @brief Encode floating-point value to string
         * @param[in] value Number to encode
         * @return String representation with optional fractional and exponent parts
         * @retval "NaN" If value is not-a-number
         * @retval "Inf" or "-Inf" If value is infinite
         * @retval "1.5e+20" For very large values (scientific notation)
         *
         * @details
         * Algorithm:
         * 1. Decompose into integer and fractional parts (modf)
         * 2. Encode integer part using integer converter (or BigInt if overflow)
         * 3. Multiply fractional part by base repeatedly to extract digits
         * 4. Use scientific notation if value magnitude exceeds thresholds
         */
        [[nodiscard]] std::string encode(T value) const {
            if (std::isnan(value)) return "NaN";
            if (std::isinf(value)) return value < 0 ? "-Inf" : "Inf";

            bool negative = value < 0;
            if (negative) value = -value;

            // Use scientific notation for extreme magnitudes
            if (value > static_cast<T>(1e12) || (value < static_cast<T>(1e-12) && value > 0)) {
                return encodeScientific(value, negative);
            }

            T int_part;
            T frac_part = std::modf(value, &int_part);

            // Encode integer part (use BigInt if too large for long long)
            std::string result;
            if (int_part > static_cast<T>(std::numeric_limits<long long>::max()) ||
                int_part < static_cast<T>(std::numeric_limits<long long>::min())) {
                BigInt big_int_part = static_cast<BigInt>(int_part);
                result = encodeBigIntPart(big_int_part); // Forward declaration
            } else {
                RadixConverter<long long> int_conv(base_, opts_);
                result = int_conv.encode(static_cast<long long>(int_part));
            }

            // Encode fractional part if present
            if (frac_part > T{0} && precision_ > 0) {
                result.push_back('.');
                encodeFractional(frac_part, result);
            }

            if (negative) result.insert(0, 1, '-');

            if (opts_.upper_case) {
                std::ranges::transform(result, result.begin(),
                                       [](unsigned char c) { return std::toupper(c); });
            }

            return result;
        }

        /**
         * @brief Decode string to floating-point value
         * @param[in] input String representation (may use scientific notation)
         * @return Decoded floating-point value
         * @throws ConvertError::Type::InvalidDigit For invalid characters
         * @throws ConvertError::Type::InvalidFormat For malformed numbers
         * @throws ConvertError::Type::Overflow If exponent too large
         *
         * @details
         * Supports formats:
         * - Standard: "123.456", "-0.001"
         * - Scientific: "1.5e10", "-2.5E-3"
         * - Special: "NaN", "Inf", "-Inf"
         * - Grouped: "1,000.5" (if configured)
         */
        [[nodiscard]] T decode(std::string_view input) const {
            // Handle special values
            if (input == "NaN" || input == "nan") return std::numeric_limits<T>::quiet_NaN();
            if (input == "Inf" || input == "inf" || input == "+Inf" || input == "+inf") return std::numeric_limits<T>::infinity();
            if (input == "-Inf" || input == "-inf") return -std::numeric_limits<T>::infinity();

            // Parse scientific notation exponent
            size_t exp_pos = input.find_first_of("eE");
            int exponent = 0;

            if (exp_pos != std::string_view::npos) {
                auto exp_part = input.substr(exp_pos + 1);
                std::from_chars(exp_part.data(), exp_part.data() + exp_part.size(), exponent);
                input = input.substr(0, exp_pos);
            }

            size_t dot_pos = input.find('.');
            std::string_view int_str = input.substr(0, dot_pos);
            std::string_view frac_str = (dot_pos != std::string_view::npos) ? input.substr(dot_pos + 1) : std::string_view{};

            // Decode integer part
            T result = T{0};
            if (!int_str.empty() && int_str != "-") {
                if (int_str.size() > 18) {
                    // Risk of long long overflow
                    BigInt big_val = decodeBigIntPart(int_str); // Forward declaration
                    result = static_cast<T>(big_val);
                } else {
                    RadixConverter<long long> int_conv(base_, opts_);
                    result = static_cast<T>(int_conv.decode(int_str));
                }
            }

            // Decode fractional part
            if (!frac_str.empty()) {
                T divisor = static_cast<T>(1);
                T frac_val = T{0};
                T base_t = static_cast<T>(base_);

                for (char c: frac_str) {
                    int digit = lookup_.lookup(c);
                    if (digit < 0) {
                        throw ConvertError(ConvertError::Type::InvalidDigit,
                                           std::format("Invalid digit in fractional part: {}", c));
                    }
                    frac_val = frac_val * base_t + static_cast<T>(digit);
                    divisor *= base_t;
                }
                result += frac_val / divisor;
            }

            // Apply exponent if present
            if (exponent != 0) {
                T factor = std::pow(static_cast<T>(base_), static_cast<T>(exponent));
                result *= factor;
            }

            return result;
        }

    private:
        ConvertOptions opts_; ///< Formatting options
        unsigned int base_; ///< Numeric base (2-36)
        int precision_; ///< Max fractional digits
        int max_exp_digits_; ///< Max width of exponent field
        detail::AlphabetLookup lookup_; ///< Character lookup table
        static constexpr std::string_view default_alphabet =
                "0123456789abcdefghijklmnopqrstuvwxyz";

        // Forward declarations for BigInt helper functions
        // (implemented after RadixConverter<BigInt> specialization)
        [[nodiscard]] std::string encodeBigIntPart(const BigInt &value) const;

        [[nodiscard]] BigInt decodeBigIntPart(std::string_view input) const;

        /**
         * @brief Append fractional part digits to string
         * @param[in] frac Fractional part value (0.0 <= frac < 1.0)
         * @param[in,out] out Output string to append to
         * @post out contains digits representing fractional part in target base
         */
        void encodeFractional(T frac, std::string &out) const {
            T base_t = static_cast<T>(base_);

            for (int i = 0; i < precision_ && frac > T{1e-15}; ++i) {
                frac *= base_t;
                int digit = static_cast<int>(frac);

                if (digit >= 0 && digit < static_cast<int>(opts_.alphabet.size())) {
                    out.push_back(opts_.alphabet[static_cast<size_t>(digit)]);
                } else {
                    out.push_back('?'); // Should never happen with valid base
                }

                frac -= static_cast<T>(digit);
            }
        }

        /**
         * @brief Encode value using scientific notation (mantissa + exponent)
         * @param[in] value Absolute value to encode (must be > 0)
         * @param[in] negative Whether original value was negative
         * @return String in format "[-]M.MMMMe[+/-]EEE"
         */
        [[nodiscard]] std::string encodeScientific(T value, bool negative) const {
            int exponent = 0;
            T base_t = static_cast<T>(base_);

            // Normalize to [1, base)
            while (value >= base_t) {
                value /= base_t;
                exponent++;
            }
            while (value < T{1} && value > T{0}) {
                value *= base_t;
                exponent--;
            }

            std::string mantissa = encode(value);

            // Remove sign from mantissa (we handle it separately)
            if (mantissa[0] == '-') mantissa.erase(0, 1);

            std::string exp_str;
            if (exponent >= 0) {
                exp_str = "e+" + std::to_string(exponent);
            } else {
                exp_str = "e" + std::to_string(exponent);
            }

            std::string result = mantissa + exp_str;
            if (negative) result.insert(0, 1, '-');

            return result;
        }
    };

    /**
     * @class RadixConverter
     * @ingroup converters
     * @brief Arbitrary-precision integer converter (explicit specialization for BigInt)
     *
     * @details
     * Specialized implementation for boost::multiprecision::cpp_int supporting
     * unlimited integer size (memory constrained only). No overflow checks needed
     * because the type expands dynamically.
     *
     * @par Complexity:
     * - Encoding: O(N) where N is number of output digits
     * - Decoding: O(M) where M is input length
     * - Space: O(N) dynamic allocation
     *
     * @note This specialization does not support group separators during decoding
     *       (they are silently skipped like other specializations).
     */
    template<>
    class RadixConverter<BigInt> {
    public:
        using ValueType = BigInt; ///< Type alias for template parameter

        /**
         * @brief Construct big integer converter
         * @param[in] base Target base (2-256)
         * @param[in] opts Conversion options
         * @throws ConvertError::Type::UnsupportedBase If alphabet < base
         */
        explicit RadixConverter(unsigned int base = 10, ConvertOptions opts = {}) : opts_(std::move(opts)), base_(base) {
            if (opts_.alphabet.empty()) {
                opts_.alphabet = "0123456789abcdefghijklmnopqrstuvwxyz";
            }
            if (opts_.alphabet.size() < base_) {
                throw ConvertError(ConvertError::Type::UnsupportedBase,
                                   std::format("Alphabet too small for base {}", base_));
            }
        }

        /**
         * @brief Encode arbitrary-precision integer to string
         * @param[in] value BigInt to encode (unlimited magnitude)
         * @return String representation in configured base
         * @throws std::bad_alloc If result string allocation fails
         * @complexity O(N) where N is number of digits in target base
         *
         * @note Pre-allocates string capacity based on bit count estimation
         *       to minimize reallocations during digit extraction.
         */
        [[nodiscard]] std::string encode(const BigInt &value) const {
            if (value == 0) {
                return detail::formatOutput(std::string(1, opts_.alphabet[0]), opts_, base_);
            }

            bool negative = value < 0;
            BigInt abs_val = negative ? -value : value;

            // Estimate digits: log2(abs_val) / log2(base) + 1
            size_t bit_count = static_cast<size_t>(abs_val != 0 ? msb(abs_val) + 1 : 1);
            double digits_estimate = static_cast<double>(bit_count) /
                                     std::log2(static_cast<double>(base_)) + 1;

            std::string result;
            result.reserve(static_cast<size_t>(digits_estimate) + 2);

            while (abs_val > 0) {
                uint32_t digit = static_cast<uint32_t>(abs_val % base_);
                result.push_back(opts_.alphabet[static_cast<size_t>(digit)]);
                abs_val /= static_cast<BigInt>(base_);
            }

            if (negative) result.push_back('-');
            std::ranges::reverse(result);

            return detail::formatOutput(result, opts_, base_);
        }

        /**
         * @brief Decode string to arbitrary-precision integer
         * @param[in] input String representation
         * @return BigInt with unlimited magnitude
         * @throws ConvertError::Type::EmptyInput If input empty after prefix strip
         * @throws ConvertError::Type::InvalidDigit If character not in alphabet
         *
         * @details
         * No overflow checking required - cpp_int dynamically expands to accommodate
         * any value representable by the input string in the given base.
         */
        [[nodiscard]] BigInt decode(std::string_view input) const {
            input = detail::stripPrefix(input, base_, opts_.prefix_base);

            if (input.empty()) {
                throw ConvertError(ConvertError::Type::EmptyInput, "Empty input");
            }

            size_t pos = 0;
            bool negative = false;

            if (input[0] == '-') {
                negative = true;
                pos = 1;
            } else if (input[0] == '+') {
                pos = 1;
            }

            detail::AlphabetLookup lookup(opts_.alphabet, opts_.case_sensitive);
            BigInt result = 0;
            BigInt base_big = base_;

            for (; pos < input.size(); ++pos) {
                char c = input[pos];
                if (opts_.group_separator != '\0' && c == opts_.group_separator) {
                    continue;
                }

                int digit = lookup.lookup(c);
                if (digit < 0 || static_cast<unsigned int>(digit) >= base_) {
                    throw ConvertError(ConvertError::Type::InvalidDigit,
                                       std::format("Invalid character: {}", c));
                }

                result *= base_big;
                result += digit;
            }

            return negative ? -result : result;
        }

        /**
         * @brief Non-throwing decode variant
         * @param[in] input String to parse
         * @return std::optional containing BigInt or nullopt on error
         */
        [[nodiscard]] std::optional<BigInt> tryDecode(std::string_view input) const noexcept {
            try {
                return decode(input);
            } catch (...) {
                return std::nullopt;
            }
        }

        /**
         * @brief Cross-base conversion using BigInt as intermediate
         * @param[in] input String in source base
         * @param[in] src_base Source numeric base
         * @param[in] src_opts Source conversion options
         * @return String encoded in this converter's base
         *
         * @details
         * Useful for converting between bases when intermediate might overflow
         * standard integer types. Automatically handles arbitrarily large values.
         */
        [[nodiscard]] std::string convertBig(std::string_view input,
                                             unsigned int src_base,
                                             const ConvertOptions &src_opts = {}) const {
            RadixConverter<BigInt> src_conv(src_base, src_opts);
            BigInt intermediate = src_conv.decode(input);
            return encode(intermediate);
        }

    private:
        ConvertOptions opts_; ///< Stored options
        unsigned int base_; ///< Numeric base

        /**
         * @brief Get most significant bit position (helper for capacity estimation)
         * @param[in] x BigInt value (must be > 0)
         * @return Index of highest set bit (0-based)
         */
        [[nodiscard]] static unsigned msb(const BigInt &x) {
            if (x <= 0) return 0;
            return static_cast<unsigned>(boost::multiprecision::msb(x));
        }
    };

    template<FloatingPoint T>
    [[nodiscard]] std::string RadixConverter<T>::encodeBigIntPart(const BigInt &value) const {
        RadixConverter<BigInt> big_conv(base_, ConvertOptions{.alphabet = opts_.alphabet});
        return big_conv.encode(value);
    }

    template<FloatingPoint T>
    [[nodiscard]] BigInt RadixConverter<T>::decodeBigIntPart(std::string_view input) const {
        RadixConverter<BigInt> big_conv(base_, ConvertOptions{.alphabet = opts_.alphabet});
        return big_conv.decode(input);
    }

    /**
     * @class ByteArrayConverter
     * @ingroup utils
     * @brief Static utility class for byte array to hex/binary conversion
     *
     * @details
     * Stateless class providing static methods for converting between byte arrays
     * (std::span<const uint8_t>) and string representations (hex, binary).
     *
     * @par Thread Safety:
     * All methods are thread-safe (stateless).
     *
     * @par Use Cases:
     * - MAC address formatting ("AA:BB:CC:DD:EE:FF")
     * - Memory dump generation
     * - Cryptographic hash representation
     * - Binary protocol debugging
     */
    class ByteArrayConverter {
    public:
        /**
         * @brief Encode byte span to hexadecimal string
         * @param[in] data Byte array to encode
         * @param[in] opts Hexadecimal formatting options
         * @return Formatted hex string
         * @retval "" If data is empty
         * @retval "0xAA:BB:CC" Example with prefix and separator
         *
         * @complexity O(N) where N is data.size()
         */
        [[nodiscard]] static std::string toHex(std::span<const uint8_t> data,
                                               const HexOptions &opts = {}) {
            if (data.empty()) return "";

            const size_t sep_count = (opts.separator != '\0') ? data.size() - 1 : 0;
            std::string result;
            result.reserve(data.size() * 2 + (opts.add_prefix ? 2 : 0) + sep_count);

            if (opts.add_prefix) {
                result.append(opts.use_uppercase ? "0X" : "0x");
            }

            const char *hex_digits = opts.use_uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

            for (size_t i = 0; i < data.size(); ++i) {
                if (i > 0 && opts.separator != '\0') {
                    result += opts.separator;
                }

                size_t idx = opts.reverse_order ? (data.size() - 1 - i) : i;
                uint8_t byte = data[idx];
                result += hex_digits[byte >> 4];
                result += hex_digits[byte & 0x0F];
            }

            return result;
        }

        /**
         * @brief Decode hexadecimal string to byte vector
         * @param[in] hex Hexadecimal string (tolerates prefixes, separators, whitespace)
         * @return Vector of decoded bytes
         * @throws ConvertError::Type::EmptyInput If no valid hex digits found
         * @throws ConvertError::Type::InvalidDigit For non-hex characters
         * @throws ConvertError::Type::InvalidFormat If odd number of nibbles
         *
         * @par Input Flexibility:
         * Accepts multiple formats automatically:
         * - "DEADBEEF" (compact)
         * - "0xDEADBEEF" (with prefix)
         * - "DE:AD:BE:EF" (with separators)
         * - "DE AD BE EF" (space separated)
         */
        [[nodiscard]] static std::vector<uint8_t> fromHex(std::string_view hex) {
            if (hex.size() >= 2 && (hex.substr(0, 2) == "0x" || hex.substr(0, 2) == "0X")) {
                hex.remove_prefix(2);
            }

            if (hex.empty()) {
                throw ConvertError(ConvertError::Type::EmptyInput, "Empty hex string");
            }

            std::vector<uint8_t> result;
            result.reserve(hex.size() / 2);

            uint8_t byte = 0;
            bool high_nibble = true;

            for (char c: hex) {
                if (std::isspace(static_cast<unsigned char>(c)) || c == ':' || c == '-') continue;

                int val = hexCharToValue(c);
                if (val < 0) {
                    throw ConvertError(ConvertError::Type::InvalidDigit,
                                       std::format("Invalid hex character: {}", c));
                }

                if (high_nibble) {
                    byte = static_cast<uint8_t>(val << 4);
                    high_nibble = false;
                } else {
                    byte |= static_cast<uint8_t>(val);
                    result.push_back(byte);
                    high_nibble = true;
                }
            }

            if (!high_nibble) {
                throw ConvertError(ConvertError::Type::InvalidFormat,
                                   "Incomplete hex string (odd number of nibbles)");
            }

            return result;
        }

        /**
         * @brief Convert byte array to binary string representation
         * @param[in] data Byte span
         * @param[in] add_prefix Add "0b" prefix
         * @param[in] group_size Bits per group (0 for no grouping)
         * @param[in] separator Group separator character
         * @return Binary string (e.g., "0b1010:1111")
         * @retval "0b00000000" If data is single zero byte with prefix
         *
         * @note Bits are ordered MSB to LSB within each byte (standard network order).
         */
        [[nodiscard]] static std::string toBinary(std::span<const uint8_t> data,
                                                  bool add_prefix = false,
                                                  int group_size = 0,
                                                  char separator = '\0') {
            std::string result;
            const size_t total_bits = data.size() * 8;
            const size_t sep_count = (group_size > 0 && separator != '\0') ? (total_bits - 1) / group_size : 0;

            result.reserve(total_bits + (add_prefix ? 2 : 0) + sep_count);

            if (add_prefix) result.append("0b");

            size_t bit_count = 0;
            for (uint8_t byte: data) {
                for (int bit = 7; bit >= 0; --bit) {
                    if (group_size > 0 && separator != '\0' && bit_count > 0 &&
                        bit_count % group_size == 0) {
                        result += separator;
                    }
                    result += (byte & (1 << bit)) ? '1' : '0';
                    bit_count++;
                }
            }
            return result;
        }

    private:
        /**
         * @brief Convert hex character to numeric value
         * @param[in] c Hex character ('0'-'9', 'a'-'f', 'A'-'F')
         * @retval 0-15 Valid hex digit value
         * @retval -1 Invalid character
         * @note Internal helper, does not throw
         */
        [[nodiscard]] static int hexCharToValue(char c) noexcept {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        }
    };

    /**
     * @class RadixPipe
     * @ingroup fluent
     * @brief Fluent interface wrapper for chained conversions
     * @tparam T Source numeric type (must satisfy Numeric concept)
     *
     * @details
     * Provides a method-chaining API for conversions, allowing expressions like:
     * @code
     * convert(255).to(16, {.upper_case=true})
     * @endcode
     *
     * @note This is a lightweight value wrapper (sizeof(T) + alignment).
     *       Move semantics are automatically applied where beneficial.
     */
    template<Numeric T>
    class RadixPipe {
    public:
        /**
         * @brief Construct pipeline with source value
         * @param[in] value Value to be converted (copied or moved)
         */
        explicit RadixPipe(T value) : value_(value) {
        }

        /**
         * @brief Convert to target base with formatting options
         * @param[in] base Target base (2-256, validated at runtime)
         * @param[in] opts Formatting options
         * @return Encoded string representation
         * @throws ConvertError If base invalid or conversion fails
         */
        [[nodiscard]] std::string to(unsigned int base, const ConvertOptions &opts = {}) const {
            RadixConverter<T> conv(base, opts);
            return conv.encode(value_);
        }

        /**
         * @brief Cast to different numeric type for subsequent conversion
         * @tparam U Target numeric type (must satisfy Numeric)
         * @return New RadixPipe with casted value
         * @warning Narrowing conversions follow standard C++ rules (possible data loss)
         *
         * @code{.cpp}
         * // Convert large int to double for floating-point encoding
         * convert(123456).as<double>().to(16);
         * @endcode
         */
        template<Numeric U>
        [[nodiscard]] RadixPipe<U> as() const {
            return RadixPipe<U>(static_cast<U>(value_));
        }

    private:
        T value_; ///< Stored source value
    };

    /**
     * @brief Factory function for starting conversion pipeline
     * @ingroup fluent
     * @tparam T Numeric type (auto-deduced)
     * @param[in] value Value to convert
     * @return RadixPipe instance for method chaining
     *
     * @par Example:
     * @code{.cpp}
     * // Decimal to hex
     * auto hex = convert(255).to(16);
     *
     * // With options
     * auto pretty = convert(1000000).to(10, {
     *     .group_separator = ',',
     *     .group_size = 3
     * }); // "1,000,000"
     * @endcode
     */
    template<Numeric T>
    [[nodiscard]] RadixPipe<T> convert(T value) {
        return RadixPipe<T>(value);
    }
} // namespace common
