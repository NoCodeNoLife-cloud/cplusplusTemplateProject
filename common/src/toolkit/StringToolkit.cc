#include "src/toolkit/StringToolkit.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <cctype>
#include <string>
#include <vector>

namespace common::toolkit {
    auto StringToolkit::split(const std::string &target, const char split_char) -> std::vector<std::string> {
        DLOG(INFO) << fmt::format("StringToolkit split - splitting by char '{}', length: {}", split_char, target.length());
        std::vector<std::string> result;
        size_t start = 0;
        size_t pos = target.find(split_char);
        while (pos != std::string::npos) {
            result.push_back(target.substr(start, pos - start));
            start = pos + 1;
            pos = target.find(split_char, start);
        }
        result.push_back(target.substr(start));
        DLOG(INFO) << fmt::format("StringToolkit split succeeded - parts: {}", result.size());
        return result;
    }

    auto StringToolkit::splitWithEmpty(const std::string &target, const char split_char) -> std::vector<std::string> {
        DLOG(INFO) << fmt::format("StringToolkit splitWithEmpty - splitting by char '{}', length: {}", split_char, target.length());
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

        DLOG(INFO) << fmt::format("StringToolkit splitWithEmpty succeeded - parts: {}", result.size());
        return result;
    }

    auto StringToolkit::split(const std::string &target, const std::string &delimiter) -> std::vector<std::string> {
        if (delimiter.empty()) {
            DLOG(WARNING) << "StringToolkit split - empty delimiter, returning original string";
            std::vector<std::string> result;
            result.push_back(target);
            return result;
        }

        DLOG(INFO) << fmt::format("StringToolkit split - splitting by delimiter '{}', length: {}", delimiter, target.length());
        std::vector<std::string> result;
        size_t start = 0;
        size_t pos = target.find(delimiter);
        while (pos != std::string::npos) {
            result.push_back(target.substr(start, pos - start));
            start = pos + delimiter.length();
            pos = target.find(delimiter, start);
        }
        result.push_back(target.substr(start));
        DLOG(INFO) << fmt::format("StringToolkit split succeeded - parts: {}", result.size());
        return result;
    }

    auto StringToolkit::concatenate(const std::vector<std::string> &source, const char split_char) -> std::string {
        if (source.empty()) {
            DLOG(INFO) << "StringToolkit concatenate - empty source vector";
            return "";
        }
        DLOG(INFO) << fmt::format("StringToolkit concatenate - joining {} parts with char '{}'", source.size(), split_char);
        std::string result = source[0];
        for (size_t i = 1; i < source.size(); ++i) {
            result += split_char;
            result += source[i];
        }
        DLOG(INFO) << fmt::format("StringToolkit concatenate succeeded - result length: {}", result.length());
        return result;
    }

    auto StringToolkit::concatenate(const std::vector<std::string> &source, const std::string &delimiter) -> std::string {
        if (source.empty()) {
            DLOG(INFO) << "StringToolkit concatenate - empty source vector";
            return "";
        }
        if (source.size() == 1) {
            DLOG(INFO) << "StringToolkit concatenate - single element, returning as-is";
            return source[0];
        }

        DLOG(INFO) << fmt::format("StringToolkit concatenate - joining {} parts with delimiter '{}'", source.size(), delimiter);
        std::string result = source[0];
        for (size_t i = 1; i < source.size(); ++i) {
            result += delimiter;
            result += source[i];
        }
        DLOG(INFO) << fmt::format("StringToolkit concatenate succeeded - result length: {}", result.length());
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
            DLOG(INFO) << "StringToolkit trim - empty string";
            return str;
        }

        DLOG(INFO) << fmt::format("StringToolkit trim - trimming string of length {}", str.length());
        size_t start = 0;
        while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start]))) {
            ++start;
        }

        if (start == str.length()) {
            DLOG(INFO) << "StringToolkit trim - all whitespace, returning empty string";
            return "";
        }

        size_t end = str.length() - 1;
        while (end > start && std::isspace(static_cast<unsigned char>(str[end]))) {
            --end;
        }

        const auto result = str.substr(start, end - start + 1);
        DLOG(INFO) << fmt::format("StringToolkit trim succeeded - trimmed length: {}", result.length());
        return result;
    }

    auto StringToolkit::replaceAll(const std::string &str, const std::string &from, const std::string &to) -> std::string {
        if (from.empty()) {
            DLOG(WARNING) << "StringToolkit replaceAll - empty 'from' string, returning original";
            return str;
        }

        DLOG(INFO) << fmt::format("StringToolkit replaceAll - replacing '{}' with '{}'", from, to);
        std::string result = str;
        size_t pos = 0;
        size_t count = 0;
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
            ++count;
        }
        DLOG(INFO) << fmt::format("StringToolkit replaceAll succeeded - replacements made: {}", count);
        return result;
    }

    auto StringToolkit::join(const std::vector<std::string> &parts, const std::string &delimiter) -> std::string {
        return concatenate(parts, delimiter);
    }
}
