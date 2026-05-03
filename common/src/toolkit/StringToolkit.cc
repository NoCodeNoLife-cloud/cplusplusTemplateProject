#include "src/toolkit/StringToolkit.hpp"

#include <fmt/format.h>
#include <cctype>
#include <string>
#include <vector>

namespace common::toolkit {
    auto StringToolkit::toUpperCase(const std::string &str) -> std::string {
        if (str.empty()) {
            return str;
        }

        std::string result = str;
        for (auto &ch : result) {
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        }
        return result;
    }

    auto StringToolkit::toLowerCase(const std::string &str) -> std::string {
        if (str.empty()) {
            return str;
        }

        std::string result = str;
        for (auto &ch : result) {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        return result;
    }

    auto StringToolkit::trimLeft(const std::string &str) -> std::string {
        if (str.empty()) {
            return str;
        }

        size_t start = 0;
        while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start]))) {
            ++start;
        }

        if (start == str.length()) {
            return "";
        }

        return str.substr(start);
    }

    auto StringToolkit::trimRight(const std::string &str) -> std::string {
        if (str.empty()) {
            return str;
        }

        size_t end = str.length() - 1;
        while (end > 0 && std::isspace(static_cast<unsigned char>(str[end]))) {
            --end;
        }

        // Check if the first character is also whitespace
        if (std::isspace(static_cast<unsigned char>(str[end]))) {
            return "";
        }

        return str.substr(0, end + 1);
    }

    auto StringToolkit::contains(const std::string &str, const std::string &substr) -> bool {
        if (substr.empty()) {
            return true;
        }
        if (str.empty()) {
            return false;
        }
        return str.find(substr) != std::string::npos;
    }

    auto StringToolkit::indexOf(const std::string &str, const std::string &substr, const size_t start) -> size_t {
        if (start > str.length()) {
            throw std::invalid_argument("Start position exceeds string length");
        }
        return str.find(substr, start);
    }

    auto StringToolkit::lastIndexOf(const std::string &str, const std::string &substr) -> size_t {
        return str.rfind(substr);
    }

    auto StringToolkit::substring(const std::string &str, const size_t start, const size_t length) -> std::string {
        if (start > str.length()) {
            throw std::invalid_argument("Start position exceeds string length");
        }

        const size_t availableLength = str.length() - start;
        const size_t actualLength = std::min(length, availableLength);
        return str.substr(start, actualLength);
    }

    auto StringToolkit::left(const std::string &str, const size_t count) -> std::string {
        if (count == 0 || str.empty()) {
            return "";
        }
        if (count >= str.length()) {
            return str;
        }
        return str.substr(0, count);
    }

    auto StringToolkit::right(const std::string &str, const size_t count) -> std::string {
        if (count == 0 || str.empty()) {
            return "";
        }
        if (count >= str.length()) {
            return str;
        }
        return str.substr(str.length() - count);
    }

    auto StringToolkit::padLeft(const std::string &str, const size_t width, const char padChar) -> std::string {
        if (str.length() >= width) {
            return str;
        }

        const size_t paddingLength = width - str.length();
        return std::string(paddingLength, padChar) + str;
    }

    auto StringToolkit::padRight(const std::string &str, const size_t width, const char padChar) -> std::string {
        if (str.length() >= width) {
            return str;
        }

        const size_t paddingLength = width - str.length();
        return str + std::string(paddingLength, padChar);
    }

    auto StringToolkit::repeat(const std::string &str, const size_t count) -> std::string {
        if (count == 0 || str.empty()) {
            return "";
        }

        std::string result;
        result.reserve(str.length() * count);
        for (size_t i = 0; i < count; ++i) {
            result += str;
        }
        return result;
    }

    auto StringToolkit::reverse(const std::string &str) -> std::string {
        if (str.empty()) {
            return str;
        }

        std::string result(str.rbegin(), str.rend());
        return result;
    }

    auto StringToolkit::isEmpty(const std::string &str) -> bool {
        return str.empty();
    }

    auto StringToolkit::isBlank(const std::string &str) -> bool {
        if (str.empty()) {
            return true;
        }

        for (const auto &ch : str) {
            if (!std::isspace(static_cast<unsigned char>(ch))) {
                return false;
            }
        }
        return true;
    }

    auto StringToolkit::isNumeric(const std::string &str) -> bool {
        if (str.empty()) {
            return false;
        }

        for (const auto &ch : str) {
            if (!std::isdigit(static_cast<unsigned char>(ch))) {
                return false;
            }
        }
        return true;
    }

    auto StringToolkit::isAlpha(const std::string &str) -> bool {
        if (str.empty()) {
            return false;
        }

        for (const auto &ch : str) {
            if (!std::isalpha(static_cast<unsigned char>(ch))) {
                return false;
            }
        }
        return true;
    }

    auto StringToolkit::isAlphanumeric(const std::string &str) -> bool {
        if (str.empty()) {
            return false;
        }

        for (const auto &ch : str) {
            if (!std::isalnum(static_cast<unsigned char>(ch))) {
                return false;
            }
        }
        return true;
    }

    auto StringToolkit::splitOnce(const std::string &target, const std::string &delimiter) -> std::pair<std::string, std::string> {
        if (delimiter.empty()) {
            throw std::invalid_argument("Delimiter cannot be empty");
        }

        const size_t pos = target.find(delimiter);
        if (pos == std::string::npos) {
            return {target, ""};
        }

        return {
            target.substr(0, pos),
            target.substr(pos + delimiter.length())
        };
    }

    auto StringToolkit::remove(const std::string &str, const std::string &toRemove) -> std::string {
        if (toRemove.empty()) {
            throw std::invalid_argument("String to remove cannot be empty");
        }

        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find(toRemove, pos)) != std::string::npos) {
            result.erase(pos, toRemove.length());
        }
        return result;
    }

    auto StringToolkit::removeChars(const std::string &str, const std::string &charsToRemove) -> std::string {
        if (charsToRemove.empty()) {
            throw std::invalid_argument("Characters to remove cannot be empty");
        }

        std::string result;
        result.reserve(str.length());

        for (const auto &ch : str) {
            if (charsToRemove.find(ch) == std::string::npos) {
                result += ch;
            }
        }
        return result;
    }

    auto StringToolkit::truncate(const std::string &str, const size_t maxLength, const std::string &suffix) -> std::string {
        if (str.length() <= maxLength) {
            return str;
        }

        if (maxLength == 0) {
            return "";
        }

        if (suffix.length() >= maxLength) {
            throw std::invalid_argument("Suffix length must be less than max length");
        }

        const size_t contentLength = maxLength - suffix.length();
        return str.substr(0, contentLength) + suffix;
    }

    auto StringToolkit::split(const std::string &target, const char split_char) -> std::vector<std::string> {
        std::vector<std::string> result;
        size_t start = 0;
        size_t pos = target.find(split_char);
        while (pos != std::string::npos) {
            result.push_back(target.substr(start, pos - start));
            start = pos + 1;
            pos = target.find(split_char, start);
        }
        result.push_back(target.substr(start));
        return result;
    }

    auto StringToolkit::splitWithEmpty(const std::string &target, const char split_char) -> std::vector<std::string> {
        std::vector<std::string> result;
        size_t start = 0;
        size_t pos = 0;

        while (pos <= target.length()) {
            if (pos == target.length() || target[pos] == split_char) {
                result.push_back(target.substr(start, pos - start));
                start = pos + 1;
            }
            pos++;
        }

        return result;
    }

    auto StringToolkit::split(const std::string &target, const std::string &delimiter) -> std::vector<std::string> {
        if (delimiter.empty()) {
            std::vector<std::string> result;
            result.push_back(target);
            return result;
        }

        std::vector<std::string> result;
        size_t start = 0;
        size_t pos = target.find(delimiter);
        while (pos != std::string::npos) {
            result.push_back(target.substr(start, pos - start));
            start = pos + delimiter.length();
            pos = target.find(delimiter, start);
        }
        result.push_back(target.substr(start));
        return result;
    }

    auto StringToolkit::concatenate(const std::vector<std::string> &source, const char split_char) -> std::string {
        if (source.empty()) {
            return "";
        }
        std::string result = source[0];
        for (size_t i = 1; i < source.size(); ++i) {
            result += split_char;
            result += source[i];
        }
        return result;
    }

    auto StringToolkit::concatenate(const std::vector<std::string> &source, const std::string &delimiter) -> std::string {
        if (source.empty()) {
            return "";
        }
        if (source.size() == 1) {
            return source[0];
        }

        std::string result = source[0];
        for (size_t i = 1; i < source.size(); ++i) {
            result += delimiter;
            result += source[i];
        }
        return result;
    }

    auto StringToolkit::startsWith(const std::string &str, const std::string &prefix) -> bool {
        if (prefix.length() > str.length()) {
            return false;
        }
        return str.compare(0, prefix.length(), prefix) == 0;
    }

    auto StringToolkit::endsWith(const std::string &str, const std::string &suffix) -> bool {
        if (suffix.length() > str.length()) {
            return false;
        }
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }

    auto StringToolkit::trim(const std::string &str) -> std::string {
        if (str.empty()) {
            return str;
        }

        size_t start = 0;
        while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start]))) {
            ++start;
        }

        if (start == str.length()) {
            return "";
        }

        size_t end = str.length() - 1;
        while (end > start && std::isspace(static_cast<unsigned char>(str[end]))) {
            --end;
        }

        return str.substr(start, end - start + 1);
    }

    auto StringToolkit::replaceAll(const std::string &str, const std::string &from, const std::string &to) -> std::string {
        if (from.empty()) {
            return str;
        }

        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }
        return result;
    }

    auto StringToolkit::join(const std::vector<std::string> &parts, const std::string &delimiter) -> std::string {
        return concatenate(parts, delimiter);
    }
}
