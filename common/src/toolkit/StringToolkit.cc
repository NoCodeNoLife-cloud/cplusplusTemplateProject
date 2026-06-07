/**
 * @file StringToolkit.cc
 * @brief StringToolkit class implementation
 * @details This file contains the implementation of the StringToolkit class methods for General utility toolkits for strings, arrays, and other operations.
 */

#include "toolkit/StringToolkit.hpp"

#include <cctype>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <glog/logging.h>

namespace common::toolkit
{
    std::string StringToolkit::toUpperCase(const std::string& str)
    {
        if (str.empty())
        {
            return str;
        }

        std::string result = str;
        for (auto& ch : result)
        {
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        }
        return result;
    }

    std::string StringToolkit::toLowerCase(const std::string& str)
    {
        if (str.empty())
        {
            return str;
        }

        std::string result = str;
        for (auto& ch : result)
        {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        return result;
    }

    std::vector<std::string> StringToolkit::split(const std::string& target, const char split_char)
    {
        std::vector<std::string> result;
        size_t start = 0;
        size_t pos = target.find(split_char);
        while (pos != std::string::npos)
        {
            result.push_back(target.substr(start, pos - start));
            start = pos + 1;
            pos = target.find(split_char, start);
        }
        result.push_back(target.substr(start));
        return result;
    }

    std::vector<std::string> StringToolkit::splitWithEmpty(const std::string& target, const char split_char)
    {
        std::vector<std::string> result;
        size_t start = 0;
        size_t pos = 0;

        while (pos <= target.length())
        {
            if (pos == target.length() || target[pos] == split_char)
            {
                result.push_back(target.substr(start, pos - start));
                start = pos + 1;
            }
            pos++;
        }

        return result;
    }

    std::vector<std::string> StringToolkit::split(const std::string& target, const std::string& delimiter)
    {
        if (delimiter.empty())
        {
            std::vector<std::string> result;
            result.push_back(target);
            return result;
        }

        std::vector<std::string> result;
        size_t start = 0;
        size_t pos = target.find(delimiter);
        while (pos != std::string::npos)
        {
            result.push_back(target.substr(start, pos - start));
            start = pos + delimiter.length();
            pos = target.find(delimiter, start);
        }
        result.push_back(target.substr(start));
        return result;
    }

    std::string StringToolkit::concatenate(const std::vector<std::string>& source, const char split_char)
    {
        if (source.empty())
        {
            return "";
        }
        std::string result = source[0];
        for (size_t i = 1; i < source.size(); ++i)
        {
            result += split_char;
            result += source[i];
        }
        return result;
    }

    std::string StringToolkit::concatenate(const std::vector<std::string>& source, const std::string& delimiter)
    {
        if (source.empty())
        {
            return "";
        }
        if (source.size() == 1)
        {
            return source[0];
        }

        std::string result = source[0];
        for (size_t i = 1; i < source.size(); ++i)
        {
            result += delimiter;
            result += source[i];
        }
        return result;
    }

    bool StringToolkit::startsWith(const std::string& str, const std::string& prefix)
    {
        if (prefix.length() > str.length())
        {
            return false;
        }
        return str.compare(0, prefix.length(), prefix) == 0;
    }

    bool StringToolkit::endsWith(const std::string& str, const std::string& suffix)
    {
        if (suffix.length() > str.length())
        {
            return false;
        }
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }

    std::string StringToolkit::trim(const std::string& str)
    {
        if (str.empty())
        {
            return str;
        }

        size_t start = 0;
        while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start])))
        {
            ++start;
        }

        if (start == str.length())
        {
            return "";
        }

        size_t end = str.length() - 1;
        while (end > start && std::isspace(static_cast<unsigned char>(str[end])))
        {
            --end;
        }

        return str.substr(start, end - start + 1);
    }

    std::string StringToolkit::trimLeft(const std::string& str)
    {
        if (str.empty())
        {
            return str;
        }

        size_t start = 0;
        while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start])))
        {
            ++start;
        }

        if (start == str.length())
        {
            return "";
        }

        return str.substr(start);
    }

    std::string StringToolkit::trimRight(const std::string& str)
    {
        if (str.empty())
        {
            return str;
        }

        auto it = str.end();
        while (it != str.begin() && std::isspace(static_cast<unsigned char>(*(it - 1))))
        {
            --it;
        }

        return std::string(str.begin(), it);
    }

    std::string StringToolkit::replaceAll(const std::string& str, const std::string& from, const std::string& to)
    {
        if (from.empty())
        {
            return str;
        }

        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos)
        {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }
        return result;
    }

    bool StringToolkit::contains(const std::string& str, const std::string& substr)
    {
        if (substr.empty())
        {
            return true;
        }
        if (str.empty())
        {
            return false;
        }
        return str.find(substr) != std::string::npos;
    }

    std::optional<size_t> StringToolkit::indexOf(const std::string& str, const std::string& substr, const size_t start)
    {
        if (start > str.length())
        {
            DLOG(WARNING) << fmt::format("StringToolkit indexOf: Start position {} exceeds string length {}", start, str.length());
            throw std::invalid_argument("Start position exceeds string length");
        }
        const auto pos = str.find(substr, start);
        return pos != std::string::npos ? std::optional(pos) : std::nullopt;
    }

    std::optional<size_t> StringToolkit::lastIndexOf(const std::string& str, const std::string& substr)
    {
        const auto pos = str.rfind(substr);
        return pos != std::string::npos ? std::optional(pos) : std::nullopt;
    }

    std::string StringToolkit::substring(const std::string& str, const size_t start, const size_t length)
    {
        if (start > str.length())
        {
            DLOG(WARNING) << fmt::format("StringToolkit substring: Start position {} exceeds string length {}", start, str.length());
            throw std::invalid_argument("StringToolkit::substring: Start position exceeds string length");
        }

        // Check for potential overflow in start + length calculation
        if (length > str.length() - start)
        {
            // Length exceeds available characters, use available length instead of throwing
            return str.substr(start);
        }

        return str.substr(start, length);
    }

    std::string StringToolkit::left(const std::string& str, const size_t count)
    {
        if (count == 0 || str.empty())
        {
            return "";
        }
        if (count >= str.length())
        {
            return str;
        }
        return str.substr(0, count);
    }

    std::string StringToolkit::right(const std::string& str, const size_t count)
    {
        if (count == 0 || str.empty())
        {
            return "";
        }
        if (count >= str.length())
        {
            return str;
        }
        return str.substr(str.length() - count);
    }

    std::string StringToolkit::padLeft(const std::string& str, const size_t width, const char padChar)
    {
        if (str.length() >= width)
        {
            return str;
        }

        const size_t paddingLength = width - str.length();
        return std::string(paddingLength, padChar) + str;
    }

    std::string StringToolkit::padRight(const std::string& str, const size_t width, const char padChar)
    {
        if (str.length() >= width)
        {
            return str;
        }

        const size_t paddingLength = width - str.length();
        return str + std::string(paddingLength, padChar);
    }

    std::string StringToolkit::repeat(const std::string& str, const size_t count)
    {
        if (count == 0 || str.empty())
        {
            return "";
        }

        try
        {
            std::string result;
            result.reserve(str.length() * count);
            for (size_t i = 0; i < count; ++i)
            {
                result += str;
            }
            return result;
        }
        catch (const std::bad_alloc&)
        {
            throw std::runtime_error(
                "StringToolkit::repeat: Failed to allocate memory for repeated string (count=" +
                std::to_string(count) + ", str.length=" + std::to_string(str.length()) + ")"
            );
        }
    }

    std::string StringToolkit::reverse(const std::string& str)
    {
        if (str.empty())
        {
            return str;
        }

        std::string result(str.rbegin(), str.rend());
        return result;
    }

    bool StringToolkit::isEmpty(const std::string& str)
    {
        return str.empty();
    }

    bool StringToolkit::isBlank(const std::string& str)
    {
        if (str.empty())
        {
            return true;
        }

        for (const auto& ch : str)
        {
            if (!std::isspace(static_cast<unsigned char>(ch)))
            {
                return false;
            }
        }
        return true;
    }

    bool StringToolkit::isNumeric(const std::string& str)
    {
        if (str.empty())
        {
            return false;
        }

        for (const auto& ch : str)
        {
            if (!std::isdigit(static_cast<unsigned char>(ch)))
            {
                return false;
            }
        }
        return true;
    }

    bool StringToolkit::isAlpha(const std::string& str)
    {
        if (str.empty())
        {
            return false;
        }

        for (const auto& ch : str)
        {
            if (!std::isalpha(static_cast<unsigned char>(ch)))
            {
                return false;
            }
        }
        return true;
    }

    bool StringToolkit::isAlphanumeric(const std::string& str)
    {
        if (str.empty())
        {
            return false;
        }

        for (const auto& ch : str)
        {
            if (!std::isalnum(static_cast<unsigned char>(ch)))
            {
                return false;
            }
        }
        return true;
    }

    std::pair<std::string, std::string> StringToolkit::splitOnce(const std::string& target, const std::string& delimiter)
    {
        if (delimiter.empty())
        {
            DLOG(WARNING) << "StringToolkit splitOnce: Delimiter is empty";
            throw std::invalid_argument("Delimiter cannot be empty");
        }

        const size_t pos = target.find(delimiter);
        if (pos == std::string::npos)
        {
            return {target, ""};
        }

        return {
            target.substr(0, pos),
            target.substr(pos + delimiter.length())
        };
    }

    std::string StringToolkit::remove(const std::string& str, const std::string& toRemove)
    {
        if (toRemove.empty())
        {
            DLOG(WARNING) << "StringToolkit remove: String to remove is empty";
            throw std::invalid_argument("String to remove cannot be empty");
        }

        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find(toRemove, pos)) != std::string::npos)
        {
            result.erase(pos, toRemove.length());
        }
        return result;
    }

    std::string StringToolkit::removeChars(const std::string& str, const std::string& charsToRemove)
    {
        if (charsToRemove.empty())
        {
            DLOG(WARNING) << "StringToolkit removeChars: Characters to remove is empty";
            throw std::invalid_argument("Characters to remove cannot be empty");
        }

        std::string result;
        result.reserve(str.length());

        for (const auto& ch : str)
        {
            if (charsToRemove.find(ch) == std::string::npos)
            {
                result += ch;
            }
        }
        return result;
    }

    std::string StringToolkit::truncate(const std::string& str, const size_t maxLength, const std::string& suffix)
    {
        if (str.length() <= maxLength)
        {
            return str;
        }

        if (maxLength == 0)
        {
            return "";
        }

        if (suffix.length() >= maxLength)
        {
            DLOG(WARNING) << fmt::format("StringToolkit truncate: Suffix length ({}) >= max length ({})", suffix.length(), maxLength);
            throw std::invalid_argument("Suffix length must be less than max length");
        }

        const size_t contentLength = maxLength - suffix.length();
        return str.substr(0, contentLength) + suffix;
    }

    std::string StringToolkit::join(const std::vector<std::string>& parts, const std::string& delimiter)
    {
        return concatenate(parts, delimiter);
    }
}
